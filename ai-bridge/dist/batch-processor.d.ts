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
export declare class BatchProcessor {
    private readonly windowMs;
    private batchBuffer;
    private flushTimer;
    private enabled;
    /** 内置的合并规则 */
    private mergeRules;
    constructor(options?: BatchProcessorOptions);
    /** 添加命令到批处理窗口 */
    add(command: PendingCommand): Promise<unknown>;
    /** 调度刷新 */
    private scheduleFlush;
    /** 刷新当前批次，返回合并后的命令列表 */
    flush(): Promise<MergedCommand[]>;
    /** 刷新并执行批处理命令（需要传入执行函数） */
    flushAndExecute(executor: (method: string, params: Record<string, unknown>) => Promise<unknown>): Promise<void>;
    /** 尝试合并同组命令 */
    private _tryMerge;
    /** 启用/禁用批处理 */
    setEnabled(enabled: boolean): void;
    /** 获取当前缓冲区大小 */
    getBufferSize(): number;
    /** 添加自定义合并规则 */
    addMergeRule(rule: MergeRule): void;
}
export {};
//# sourceMappingURL=batch-processor.d.ts.map