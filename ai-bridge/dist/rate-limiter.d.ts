/**
 * 令牌桶限流器
 * 支持优先级和自动补充令牌
 */
import type { RateLimiterOptions, Priority } from './types.js';
export declare class RateLimiter {
    private tokens;
    private readonly maxTokens;
    private readonly refillRate;
    private lastRefillTime;
    private waitQueue;
    constructor(options?: RateLimiterOptions);
    /** 补充令牌 */
    private refill;
    /** 获取指定优先级所需的令牌数 */
    private getCost;
    /** 检查是否可以执行（不消耗令牌） */
    canExecute(priority?: Priority): boolean;
    /** 尝试消耗令牌并执行 */
    tryConsume(priority?: Priority): boolean;
    /** 等待直到获取令牌 */
    waitForToken(priority?: Priority): Promise<void>;
    /** 处理等待队列 */
    private processWaitQueue;
    /** 获取当前可用令牌数 */
    getAvailableTokens(): number;
    /** 计算等待时间（毫秒） */
    getWaitTime(priority?: Priority): number;
    /** 重置令牌桶 */
    reset(): void;
}
//# sourceMappingURL=rate-limiter.d.ts.map