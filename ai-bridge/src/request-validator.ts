/**
 * 请求校验器
 * 校验 MCP 工具调用参数的合法性
 * 内置基础 schema，可从 Godot 动态更新
 */

import type { ToolDefinition } from './types.js';

/** JSON Schema 简化类型 */
interface JsonSchema {
  type?: string;
  properties?: Record<string, JsonSchema>;
  required?: string[];
  items?: JsonSchema;
  enum?: unknown[];
  minimum?: number;
  maximum?: number;
  minLength?: number;
  maxLength?: number;
  pattern?: string;
  [key: string]: unknown;
}

/** 校验结果 */
export interface ValidationResult {
  valid: boolean;
  errors?: string[];
}

export class RequestValidator {
  /** 工具参数 schema 映射 */
  private schemas: Map<string, JsonSchema> = new Map();

  constructor() {
    // 注册内置的基础 schema
    this._registerBuiltinSchemas();
  }

  /** 校验工具调用参数 */
  validate(toolName: string, args: Record<string, unknown>): ValidationResult {
    const schema = this.schemas.get(toolName);

    // 没有 schema 时的宽松校验
    if (schema === undefined) {
      return this._validateBasic(args);
    }

    const errors: string[] = [];

    // 检查必需参数
    if (schema.required) {
      for (const req of schema.required) {
        if (args[req] === undefined || args[req] === null) {
          errors.push(`缺少必需参数: ${req}`);
        }
      }
    }

    // 逐个校验参数
    if (schema.properties) {
      for (const [key, value] of Object.entries(args)) {
        const propSchema = schema.properties[key];
        if (propSchema === undefined) {
          // 额外参数不报错，仅警告（宽松模式）
          continue;
        }
        const propErrors = this._validateProperty(key, value, propSchema);
        errors.push(...propErrors);
      }
    }

    return errors.length > 0
      ? { valid: false, errors }
      : { valid: true };
  }

  /** 从 Godot 获取的工具定义更新 schema */
  updateSchemas(toolDefinitions: ToolDefinition[]): void {
    for (const tool of toolDefinitions) {
      if (tool.inputSchema) {
        this.schemas.set(tool.name, tool.inputSchema as JsonSchema);
      }
    }
  }

  /** 注册新的工具 schema */
  registerSchema(toolName: string, schema: JsonSchema): void {
    this.schemas.set(toolName, schema);
  }

  /** 获取已注册的 schema */
  getSchema(toolName: string): JsonSchema | undefined {
    return this.schemas.get(toolName);
  }

  /** 获取已注册的工具名列表 */
  getRegisteredTools(): string[] {
    return Array.from(this.schemas.keys());
  }

  /** 校验单个属性 */
  private _validateProperty(key: string, value: unknown, schema: JsonSchema): string[] {
    const errors: string[] = [];

    // 类型校验
    if (schema.type) {
      const typeMatch = this._checkType(value, schema.type);
      if (!typeMatch) {
        errors.push(`参数 "${key}" 类型错误: 期望 ${schema.type}, 实际 ${typeof value}`);
        return errors; // 类型不对，不再继续校验
      }
    }

    // 枚举校验
    if (schema.enum && value !== undefined) {
      if (!schema.enum.includes(value)) {
        errors.push(
          `参数 "${key}" 值无效: 期望 ${JSON.stringify(schema.enum)} 之一, 实际 ${JSON.stringify(value)}`,
        );
      }
    }

    // 数值范围校验
    if (typeof value === 'number') {
      if (schema.minimum !== undefined && value < schema.minimum) {
        errors.push(`参数 "${key}" 值过小: 最小 ${schema.minimum}, 实际 ${value}`);
      }
      if (schema.maximum !== undefined && value > schema.maximum) {
        errors.push(`参数 "${key}" 值过大: 最大 ${schema.maximum}, 实际 ${value}`);
      }
    }

    // 字符串长度校验
    if (typeof value === 'string') {
      if (schema.minLength !== undefined && value.length < schema.minLength) {
        errors.push(`参数 "${key}" 长度不足: 最短 ${schema.minLength}, 实际 ${value.length}`);
      }
      if (schema.maxLength !== undefined && value.length > schema.maxLength) {
        errors.push(`参数 "${key}" 长度超限: 最长 ${schema.maxLength}, 实际 ${value.length}`);
      }
      if (schema.pattern !== undefined) {
        const regex = new RegExp(schema.pattern);
        if (!regex.test(value)) {
          errors.push(`参数 "${key}" 不匹配模式: ${schema.pattern}`);
        }
      }
    }

    // 数组元素校验
    if (Array.isArray(value) && schema.items) {
      for (let i = 0; i < value.length; i++) {
        const itemErrors = this._validateProperty(`${key}[${i}]`, value[i], schema.items);
        errors.push(...itemErrors);
      }
    }

    // 嵌套对象校验
    if (schema.properties && typeof value === 'object' && value !== null && !Array.isArray(value)) {
      const nested = value as Record<string, unknown>;
      if (schema.required) {
        for (const req of schema.required) {
          if (nested[req] === undefined || nested[req] === null) {
            errors.push(`参数 "${key}.${req}" 缺少必需值`);
          }
        }
      }
      for (const [nestedKey, nestedValue] of Object.entries(nested)) {
        const propSchema = schema.properties[nestedKey];
        if (propSchema) {
          const nestedErrors = this._validateProperty(`${key}.${nestedKey}`, nestedValue, propSchema);
          errors.push(...nestedErrors);
        }
      }
    }

    return errors;
  }

  /** 检查值类型是否匹配 */
  private _checkType(value: unknown, expectedType: string): boolean {
    if (value === null) return expectedType === 'null';
    if (value === undefined) return false;

    switch (expectedType) {
      case 'string': return typeof value === 'string';
      case 'number': return typeof value === 'number';
      case 'integer': return Number.isInteger(value);
      case 'boolean': return typeof value === 'boolean';
      case 'array': return Array.isArray(value);
      case 'object': return typeof value === 'object' && !Array.isArray(value);
      default: return true;
    }
  }

  /** 基础宽松校验 */
  private _validateBasic(args: Record<string, unknown>): ValidationResult {
    // 确保参数是对象
    if (typeof args !== 'object' || args === null || Array.isArray(args)) {
      return {
        valid: false,
        errors: ['参数必须是对象'],
      };
    }
    return { valid: true };
  }

  /** 注册内置的基础 schema */
  private _registerBuiltinSchemas(): void {
    // 通用节点操作 schema
    this.schemas.set('create_node', {
      type: 'object',
      properties: {
        type: { type: 'string', minLength: 1 },
        name: { type: 'string' },
        parent: { type: 'string' },
        properties: { type: 'object' },
      },
      required: ['type'],
    });

    this.schemas.set('delete_node', {
      type: 'object',
      properties: {
        node: { type: 'string', minLength: 1 },
      },
      required: ['node'],
    });

    this.schemas.set('set_node_property', {
      type: 'object',
      properties: {
        node: { type: 'string', minLength: 1 },
        prop: { type: 'string', minLength: 1 },
        value: {},
      },
      required: ['node', 'prop'],
    });

    this.schemas.set('get_node_property', {
      type: 'object',
      properties: {
        node: { type: 'string', minLength: 1 },
        prop: { type: 'string', minLength: 1 },
      },
      required: ['node', 'prop'],
    });

    this.schemas.set('get_scene_tree', {
      type: 'object',
      properties: {},
    });

    this.schemas.set('run_scene', {
      type: 'object',
      properties: {
        scene: { type: 'string' },
      },
    });

    this.schemas.set('stop_scene', {
      type: 'object',
      properties: {},
    });

    this.schemas.set('save_scene', {
      type: 'object',
      properties: {
        path: { type: 'string' },
      },
    });

    this.schemas.set('execute_script', {
      type: 'object',
      properties: {
        script: { type: 'string', minLength: 1 },
        language: { type: 'string', enum: ['gdscript', 'csharp'] },
      },
      required: ['script'],
    });

    this.schemas.set('get_project_info', {
      type: 'object',
      properties: {},
    });
  }
}
