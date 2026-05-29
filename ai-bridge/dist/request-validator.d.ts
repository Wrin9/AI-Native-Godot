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
export declare class RequestValidator {
    /** 工具参数 schema 映射 */
    private schemas;
    constructor();
    /** 校验工具调用参数 */
    validate(toolName: string, args: Record<string, unknown>): ValidationResult;
    /** 从 Godot 获取的工具定义更新 schema */
    updateSchemas(toolDefinitions: ToolDefinition[]): void;
    /** 注册新的工具 schema */
    registerSchema(toolName: string, schema: JsonSchema): void;
    /** 获取已注册的 schema */
    getSchema(toolName: string): JsonSchema | undefined;
    /** 获取已注册的工具名列表 */
    getRegisteredTools(): string[];
    /** 校验单个属性 */
    private _validateProperty;
    /** 检查值类型是否匹配 */
    private _checkType;
    /** 基础宽松校验 */
    private _validateBasic;
    /** 注册内置的基础 schema */
    private _registerBuiltinSchemas;
}
export {};
//# sourceMappingURL=request-validator.d.ts.map