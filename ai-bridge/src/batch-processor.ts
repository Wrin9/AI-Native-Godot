/**
 * 批处理器
 * 在时间窗口内合并同类命令，减少对 Godot 的请求数量
 */

import type { BatchProcessorOptions, PendingCommand, MergedCommand } from './types.js';

/** 可合并命令的分组键提取规则 */
interface MergeRule {
  /** 从方法名提取分组键 */
  groupKey: (method: string, params: Record<string, unknown>) => string | null;
  /** 合并同组命令 */
  merge: (commands: PendingCommand[]) => MergedCommand;
}

export class BatchProcessor {
  private readonly windowMs: number;
  private batchBuffer: Map<string, PendingCommand[]> = new Map();
  private flushTimer: NodeJS.Timeout | null = null;
  private enabled = true;

  /** 内置的合并规则 */
  private mergeRules: MergeRule[] = [
    // 规则：set_node_property 同一节点的属性设置合并
    {
      groupKey: (method, params) => {
        if (method === 'set_node_property' && params.node) {
          return `set_node_property:${params.node}`;
        }
        return null;
      },
      merge: (commands) => {
        const properties: Record<string, unknown> = {};
        const firstParams = commands[0]!.params;
        for (const cmd of commands) {
          if (cmd.params.prop !== undefined && cmd.params.value !== undefined) {
            properties[String(cmd.params.prop)] = cmd.params.value;
          }
        }
        return {
          method: 'set_node_properties',
          params: {
            node: firstParams.node,
            properties,
          },
        };
      },
    },
    // 规则：create_node 与后续的 set_node_property 合并为 create_node_full
    {
      groupKey: (method, params) => {
        if (method === 'create_node' && params.type) {
          return `create_node:${params.parent ?? 'root'}:${params.type}`;
        }
        return null;
      },
      merge: (commands) => {
        const firstParams = commands[0]!.params;
        const properties: Record<string, unknown> = {};
        for (const cmd of commands) {
          if (cmd.params.properties && typeof cmd.params.properties === 'object') {
            Object.assign(properties, cmd.params.properties as Record<string, unknown>);
          }
        }
        return {
          method: 'create_node',
          params: {
            ...firstParams,
            ...(Object.keys(properties).length > 0 ? { properties } : {}),
          },
        };
      },
    },
  ];

  constructor(options: BatchProcessorOptions = {}) {
    this.windowMs = options.windowMs ?? 100; // 默认 100ms 批处理窗口
  }

  /** 添加命令到批处理窗口 */
  add(command: PendingCommand): Promise<unknown> {
    if (!this.enabled) {
      // 批处理禁用时直接返回，由调用方处理
      return Promise.reject(new Error('Batch processor is disabled'));
    }

    // 查找匹配的合并规则
    let groupKey: string | null = null;
    for (const rule of this.mergeRules) {
      const key = rule.groupKey(command.method, command.params);
      if (key !== null) {
        groupKey = key;
        break;
      }
    }

    // 没有匹配的合并规则，使用方法名作为分组键
    if (groupKey === null) {
      groupKey = `${command.method}:${Date.now()}:${Math.random().toString(36).slice(2, 7)}`;
    }

    // 添加到缓冲区
    if (!this.batchBuffer.has(groupKey)) {
      this.batchBuffer.set(groupKey, []);
    }
    this.batchBuffer.get(groupKey)!.push(command);

    // 启动刷新定时器
    this.scheduleFlush();

    return new Promise((resolve, reject) => {
      // 替换原始的 resolve/reject，在批处理完成时调用
      const originalResolve = command.resolve;
      const originalReject = command.reject;

      // 这里返回的 Promise 在 flush 时由合并命令的结果来 resolve
      // 暂时用占位方式，实际逻辑在 flush 中处理
      command.resolve = (value: unknown) => {
        originalResolve(value);
        resolve(value);
      };
      command.reject = (reason: unknown) => {
        originalReject(reason);
        reject(reason);
      };
    });
  }

  /** 调度刷新 */
  private scheduleFlush(): void {
    if (this.flushTimer !== null) {
      return; // 已有定时器在等待
    }
    this.flushTimer = setTimeout(() => {
      this.flushTimer = null;
      this.flush().catch((err) => {
        console.error('[batch] Flush error:', err);
      });
    }, this.windowMs);
  }

  /** 刷新当前批次，返回合并后的命令列表 */
  async flush(): Promise<MergedCommand[]> {
    // 取消定时器
    if (this.flushTimer !== null) {
      clearTimeout(this.flushTimer);
      this.flushTimer = null;
    }

    // 取出当前缓冲区内容
    const currentBatch = this.batchBuffer;
    this.batchBuffer = new Map();

    if (currentBatch.size === 0) {
      return [];
    }

    const mergedCommands: MergedCommand[] = [];

    for (const [groupKey, commands] of currentBatch) {
      if (commands.length === 1) {
        // 单个命令，不需要合并
        const cmd = commands[0]!;
        mergedCommands.push({
          method: cmd.method,
          params: cmd.params,
        });
        // 直接 resolve，结果由调用方设置
        continue;
      }

      // 查找适用的合并规则
      let merged: MergedCommand | null = null;
      for (const rule of this.mergeRules) {
        const key = rule.groupKey(commands[0]!.method, commands[0]!.params);
        if (key !== null && key === groupKey.split(':').slice(0, key.split(':').length).join(':')) {
          merged = rule.merge(commands);
          break;
        }
      }

      if (merged === null) {
        // 没有合并规则，逐个处理
        for (const cmd of commands) {
          mergedCommands.push({
            method: cmd.method,
            params: cmd.params,
          });
        }
      } else {
        mergedCommands.push(merged);
      }
    }

    return mergedCommands;
  }

  /** 刷新并执行批处理命令（需要传入执行函数） */
  async flushAndExecute(
    executor: (method: string, params: Record<string, unknown>) => Promise<unknown>,
  ): Promise<void> {
    // 取消定时器
    if (this.flushTimer !== null) {
      clearTimeout(this.flushTimer);
      this.flushTimer = null;
    }

    // 取出当前缓冲区内容
    const currentBatch = this.batchBuffer;
    this.batchBuffer = new Map();

    if (currentBatch.size === 0) {
      return;
    }

    for (const [_groupKey, commands] of currentBatch) {
      try {
        let result: unknown;

        if (commands.length === 1) {
          // 单个命令直接执行
          const cmd = commands[0]!;
          result = await executor(cmd.method, cmd.params);
          cmd.resolve(result);
        } else {
          // 尝试合并执行
          const merged = this._tryMerge(commands);
          if (merged !== null) {
            result = await executor(merged.method, merged.params);
            // 所有命令共享同一个结果
            for (const cmd of commands) {
              cmd.resolve(result);
            }
          } else {
            // 无法合并，逐个执行
            for (const cmd of commands) {
              try {
                result = await executor(cmd.method, cmd.params);
                cmd.resolve(result);
              } catch (err) {
                cmd.reject(err);
              }
            }
          }
        }
      } catch (err) {
        // 整组失败
        for (const cmd of commands) {
          cmd.reject(err);
        }
      }
    }
  }

  /** 尝试合并同组命令 */
  private _tryMerge(commands: PendingCommand[]): MergedCommand | null {
    for (const rule of this.mergeRules) {
      const key = rule.groupKey(commands[0]!.method, commands[0]!.params);
      if (key !== null) {
        // 检查所有命令是否属于同一组
        const allSameGroup = commands.every(
          (cmd) => rule.groupKey(cmd.method, cmd.params) === key,
        );
        if (allSameGroup) {
          return rule.merge(commands);
        }
      }
    }
    return null;
  }

  /** 启用/禁用批处理 */
  setEnabled(enabled: boolean): void {
    this.enabled = enabled;
    if (!enabled) {
      // 禁用时立即刷新
      this.flushAndExecute(async () => null).catch(() => {});
    }
  }

  /** 获取当前缓冲区大小 */
  getBufferSize(): number {
    let total = 0;
    for (const commands of this.batchBuffer.values()) {
      total += commands.length;
    }
    return total;
  }

  /** 添加自定义合并规则 */
  addMergeRule(rule: MergeRule): void {
    this.mergeRules.unshift(rule); // 添加到前面，优先匹配
  }
}
