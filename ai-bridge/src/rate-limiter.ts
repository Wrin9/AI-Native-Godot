/**
 * 令牌桶限流器
 * 支持优先级和自动补充令牌
 */

import type { RateLimiterOptions, Priority } from './types.js';

/** 优先级对应的令牌消耗倍率 */
const PRIORITY_COST: Record<Priority, number> = {
  low: 2,       // 低优先级消耗 2 个令牌
  normal: 1,    // 普通优先级消耗 1 个令牌
  high: 0.5,    // 高优先级消耗 0.5 个令牌
  critical: 0,  // 关键操作不消耗令牌
};

export class RateLimiter {
  private tokens: number;
  private readonly maxTokens: number;
  private readonly refillRate: number;  // 每秒补充的令牌数
  private lastRefillTime: number;
  private waitQueue: Array<{
    priority: Priority;
    resolve: () => void;
  }> = [];

  constructor(options: RateLimiterOptions = {}) {
    this.maxTokens = options.maxTokens ?? 10;
    this.refillRate = options.refillRate ?? 5; // 默认每秒补充 5 个令牌
    this.tokens = this.maxTokens;
    this.lastRefillTime = Date.now();
  }

  /** 补充令牌 */
  private refill(): void {
    const now = Date.now();
    const elapsed = (now - this.lastRefillTime) / 1000;
    const tokensToAdd = elapsed * this.refillRate;
    this.tokens = Math.min(this.maxTokens, this.tokens + tokensToAdd);
    this.lastRefillTime = now;
  }

  /** 获取指定优先级所需的令牌数 */
  private getCost(priority: Priority): number {
    return PRIORITY_COST[priority];
  }

  /** 检查是否可以执行（不消耗令牌） */
  canExecute(priority: Priority = 'normal'): boolean {
    this.refill();
    return this.tokens >= this.getCost(priority);
  }

  /** 尝试消耗令牌并执行 */
  tryConsume(priority: Priority = 'normal'): boolean {
    this.refill();
    const cost = this.getCost(priority);
    if (this.tokens >= cost) {
      this.tokens -= cost;
      // 唤醒等待队列中可能的请求
      this.processWaitQueue();
      return true;
    }
    return false;
  }

  /** 等待直到获取令牌 */
  async waitForToken(priority: Priority = 'normal'): Promise<void> {
    if (this.tryConsume(priority)) {
      return;
    }

    // 排队等待
    return new Promise<void>((resolve) => {
      this.waitQueue.push({ priority, resolve });
    });
  }

  /** 处理等待队列 */
  private processWaitQueue(): void {
    // 按优先级排序：critical > high > normal > low
    const priorityOrder: Record<Priority, number> = {
      critical: 0,
      high: 1,
      normal: 2,
      low: 3,
    };

    this.waitQueue.sort((a, b) => priorityOrder[a.priority] - priorityOrder[b.priority]);

    const remaining: typeof this.waitQueue = [];

    for (const waiter of this.waitQueue) {
      if (this.tryConsume(waiter.priority)) {
        waiter.resolve();
      } else {
        remaining.push(waiter);
      }
    }

    this.waitQueue = remaining;
  }

  /** 获取当前可用令牌数 */
  getAvailableTokens(): number {
    this.refill();
    return Math.floor(this.tokens);
  }

  /** 计算等待时间（毫秒） */
  getWaitTime(priority: Priority = 'normal'): number {
    this.refill();
    const cost = this.getCost(priority);
    if (this.tokens >= cost) {
      return 0;
    }
    const deficit = cost - this.tokens;
    return Math.ceil((deficit / this.refillRate) * 1000);
  }

  /** 重置令牌桶 */
  reset(): void {
    this.tokens = this.maxTokens;
    this.lastRefillTime = Date.now();
    // 唤醒所有等待者
    for (const waiter of this.waitQueue) {
      waiter.resolve();
    }
    this.waitQueue = [];
  }
}
