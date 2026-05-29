/**
 * Godot WebSocket 连接管理
 * 负责与 Godot 编辑器模块的 WebSocket 通信
 * 支持自动重连、心跳检测、请求超时和断连消息缓存
 */

import WebSocket from 'ws';
import { EventEmitter } from 'events';
import type { GodotMessage, PendingRequest, ConnectionState, GodotEvent } from './types.js';

/** 连接配置 */
export interface GodotConnectionOptions {
  /** 请求超时时间（毫秒） */
  requestTimeout?: number;
  /** 心跳间隔（毫秒） */
  heartbeatInterval?: number;
  /** 重连间隔（毫秒） */
  reconnectInterval?: number;
  /** 最大重连次数（0 = 无限） */
  maxReconnectAttempts?: number;
  /** 断连消息队列最大长度 */
  maxQueueSize?: number;
}

/** 默认配置 */
const DEFAULTS: Required<GodotConnectionOptions> = {
  requestTimeout: 10_000,
  heartbeatInterval: 30_000,
  reconnectInterval: 2_000,
  maxReconnectAttempts: 0,
  maxQueueSize: 1_000,
};

export class GodotConnection extends EventEmitter {
  private ws: WebSocket | null = null;
  private url: string;
  private options: Required<GodotConnectionOptions>;
  private state: ConnectionState = 'disconnected';
  private pendingRequests: Map<string | number, PendingRequest> = new Map();
  private reconnectTimer: NodeJS.Timeout | null = null;
  private heartbeatTimer: NodeJS.Timeout | null = null;
  private messageQueue: GodotMessage[] = [];
  private requestId = 0;
  private reconnectAttempts = 0;
  private pingTimeout: NodeJS.Timeout | null = null;

  constructor(url: string, options: GodotConnectionOptions = {}) {
    super();
    this.url = url;
    this.options = { ...DEFAULTS, ...options };
  }

  /** 获取当前连接状态 */
  getState(): ConnectionState {
    return this.state;
  }

  /** 是否已连接 */
  isConnected(): boolean {
    return this.state === 'connected';
  }

  /** 连接到 Godot */
  async connect(): Promise<void> {
    if (this.state === 'connected' || this.state === 'connecting') {
      return;
    }

    this.state = 'connecting';
    this.emit('stateChange', this.state);

    return new Promise((resolve, reject) => {
      try {
        this.ws = new WebSocket(this.url);

        this.ws.on('open', () => {
          this.state = 'connected';
          this.reconnectAttempts = 0;
          this.emit('connected');
          this.emit('stateChange', this.state);

          // 启动心跳
          this._startHeartbeat();

          // 发送队列中的消息
          this._flushQueue();

          resolve();
        });

        this.ws.on('message', (data: WebSocket.Data) => {
          this._handleMessage(data);
        });

        this.ws.on('close', (code: number, reason: Buffer) => {
          this._handleDisconnect(code, reason.toString());
          if (this.state === 'connecting') {
            reject(new Error(`Connection failed: ${reason.toString() || `code ${code}`}`));
          }
        });

        this.ws.on('error', (error: Error) => {
          console.error(`[godot] WebSocket error: ${error.message}`);
          if (this.state === 'connecting') {
            reject(error);
          }
        });

        this.ws.on('ping', () => {
          // 收到 ping 响应，重置超时
          this._resetPingTimeout();
        });

        this.ws.on('pong', () => {
          this._resetPingTimeout();
        });
      } catch (error) {
        this.state = 'disconnected';
        reject(error);
      }
    });
  }

  /** 断开连接 */
  disconnect(): void {
    this._stopHeartbeat();
    this._stopReconnect();
    this._clearPendingRequests('连接已断开');

    if (this.ws) {
      try {
        this.ws.close(1000, 'Client disconnect');
      } catch {
        // 忽略关闭错误
      }
      this.ws = null;
    }

    this.state = 'disconnected';
    this.emit('disconnected');
    this.emit('stateChange', this.state);
  }

  /** 发送 JSON-RPC 请求并等待响应 */
  async request(method: string, params?: Record<string, unknown>): Promise<unknown> {
    const id = ++this.requestId;
    const message: GodotMessage = {
      jsonrpc: '2.0',
      id,
      method,
      ...(params !== undefined ? { params } : {}),
    };

    // 如果未连接，加入队列并等待
    if (!this.isConnected()) {
      if (this.messageQueue.length >= this.options.maxQueueSize) {
        throw new Error('消息队列已满，请等待重连');
      }
      this.messageQueue.push(message);
      // 等待连接和响应
      return new Promise((resolve, reject) => {
        this.pendingRequests.set(id, {
          resolve,
          reject,
          timer: setTimeout(() => {
            this.pendingRequests.delete(id);
            reject(new Error(`请求超时: ${method}`));
          }, this.options.requestTimeout * 2), // 队列中的请求给更长的超时
        });
      });
    }

    // 发送请求
    this._send(message);

    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        this.pendingRequests.delete(id);
        reject(new Error(`请求超时 (${this.options.requestTimeout}ms): ${method}`));
      }, this.options.requestTimeout);

      this.pendingRequests.set(id, { resolve, reject, timer });
    });
  }

  /** 发送通知（不等待响应） */
  notify(method: string, params?: Record<string, unknown>): void {
    const message: GodotMessage = {
      jsonrpc: '2.0',
      method,
      ...(params !== undefined ? { params } : {}),
    };

    if (this.isConnected()) {
      this._send(message);
    } else if (this.messageQueue.length < this.options.maxQueueSize) {
      this.messageQueue.push(message);
      console.error(`[godot] 已缓存通知 (队列: ${this.messageQueue.length})`);
    }
  }

  /** 发送原始消息 */
  private _send(message: GodotMessage): void {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      console.error('[godot] 尝试发送消息时连接未就绪');
      return;
    }

    const data = JSON.stringify(message);
    this.ws.send(data);
  }

  /** 处理收到的消息 */
  private _handleMessage(data: WebSocket.Data): void {
    let message: GodotMessage;
    try {
      message = JSON.parse(data.toString());
    } catch {
      console.error('[godot] 无法解析消息:', data.toString());
      return;
    }

    // 响应消息（有 id 的）
    if ('id' in message && message.id !== undefined && message.id !== null) {
      const pending = this.pendingRequests.get(message.id);
      if (pending) {
        clearTimeout(pending.timer);
        this.pendingRequests.delete(message.id);

        if (message.error) {
          pending.reject(new Error(
            `Godot 错误 [${message.error.code}]: ${message.error.message}`,
          ));
        } else {
          pending.resolve(message.result);
        }
      }
      return;
    }

    // 通知/事件消息
    if (message.method) {
      // Godot 推送的事件
      if (message.method.startsWith('event/')) {
        const eventType = message.method.slice('event/'.length);
        const event: GodotEvent = {
          type: eventType,
          data: (message.params ?? {}) as Record<string, unknown>,
          timestamp: Date.now(),
        };
        this.emit('event', event);
      } else {
        // 其他通知
        this.emit('notification', message);
      }

      // 处理服务器发起的请求（如果 Godot 向 bridge 发送请求）
      if (message.method && !('id' in message)) {
        this.emit('serverNotification', message.method, message.params);
      }
    }
  }

  /** 处理断开连接 */
  private _handleDisconnect(code: number, reason: string): void {
    console.error(`[godot] 连接断开 (code: ${code}, reason: ${reason})`);
    this._stopHeartbeat();

    const wasConnected = this.state === 'connected';
    this.state = 'disconnected';
    this.ws = null;

    this.emit('disconnected', { code, reason });
    this.emit('stateChange', this.state);

    // 自动重连
    if (wasConnected || code !== 1000) {
      this._reconnect();
    }
  }

  /** 自动重连 */
  private _reconnect(): void {
    if (this.reconnectTimer !== null) {
      return;
    }

    this.reconnectAttempts++;

    // 检查最大重连次数
    if (
      this.options.maxReconnectAttempts > 0 &&
      this.reconnectAttempts > this.options.maxReconnectAttempts
    ) {
      console.error(`[godot] 已达最大重连次数 (${this.options.maxReconnectAttempts})，停止重连`);
      this._clearPendingRequests('已达最大重连次数，连接失败');
      return;
    }

    this.state = 'reconnecting';
    this.emit('stateChange', this.state);
    this.emit('reconnecting', this.reconnectAttempts);

    console.error(
      `[godot] ${this.options.reconnectInterval}ms 后尝试第 ${this.reconnectAttempts} 次重连...`,
    );

    this.reconnectTimer = setTimeout(async () => {
      this.reconnectTimer = null;
      try {
        await this.connect();
        console.error('[godot] 重连成功');
      } catch {
        // 连接失败，继续重连
        this._reconnect();
      }
    }, this.options.reconnectInterval);
  }

  /** 停止重连 */
  private _stopReconnect(): void {
    if (this.reconnectTimer !== null) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
  }

  /** 启动心跳检测 */
  private _startHeartbeat(): void {
    this._stopHeartbeat();

    this.heartbeatTimer = setInterval(() => {
      if (this.ws && this.ws.readyState === WebSocket.OPEN) {
        this.ws.ping();

        // 设置 pong 超时
        this.pingTimeout = setTimeout(() => {
          console.error('[godot] 心跳超时，断开连接');
          this.ws?.terminate();
        }, 10_000); // 10 秒内没收到 pong 就断开
      }
    }, this.options.heartbeatInterval);
  }

  /** 停止心跳检测 */
  private _stopHeartbeat(): void {
    if (this.heartbeatTimer !== null) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = null;
    }
    this._resetPingTimeout();
  }

  /** 重置 pong 超时 */
  private _resetPingTimeout(): void {
    if (this.pingTimeout !== null) {
      clearTimeout(this.pingTimeout);
      this.pingTimeout = null;
    }
  }

  /** 发送队列中的消息 */
  private _flushQueue(): void {
    if (this.messageQueue.length === 0) {
      return;
    }

    const count = this.messageQueue.length;
    console.error(`[godot] 发送队列中的 ${count} 条消息`);

    while (this.messageQueue.length > 0) {
      const message = this.messageQueue.shift()!;
      this._send(message);
    }
  }

  /** 清除所有待处理的请求 */
  private _clearPendingRequests(reason: string): void {
    for (const pending of this.pendingRequests.values()) {
      clearTimeout(pending.timer);
      pending.reject(new Error(reason));
    }
    this.pendingRequests.clear();
  }

  /** 获取统计信息 */
  getStats(): {
    state: ConnectionState;
    pendingRequests: number;
    queuedMessages: number;
    reconnectAttempts: number;
  } {
    return {
      state: this.state,
      pendingRequests: this.pendingRequests.size,
      queuedMessages: this.messageQueue.length,
      reconnectAttempts: this.reconnectAttempts,
    };
  }
}
