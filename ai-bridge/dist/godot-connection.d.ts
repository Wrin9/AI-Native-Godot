/**
 * Godot WebSocket 连接管理
 * 负责与 Godot 编辑器模块的 WebSocket 通信
 * 支持自动重连、心跳检测、请求超时和断连消息缓存
 */
import { EventEmitter } from 'events';
import type { ConnectionState } from './types.js';
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
export declare class GodotConnection extends EventEmitter {
    private ws;
    private url;
    private options;
    private state;
    private pendingRequests;
    private reconnectTimer;
    private heartbeatTimer;
    private messageQueue;
    private requestId;
    private reconnectAttempts;
    private pingTimeout;
    constructor(url: string, options?: GodotConnectionOptions);
    /** 获取当前连接状态 */
    getState(): ConnectionState;
    /** 是否已连接 */
    isConnected(): boolean;
    /** 连接到 Godot */
    connect(): Promise<void>;
    /** 断开连接 */
    disconnect(): void;
    /** 发送 JSON-RPC 请求并等待响应 */
    request(method: string, params?: Record<string, unknown>): Promise<unknown>;
    /** 发送通知（不等待响应） */
    notify(method: string, params?: Record<string, unknown>): void;
    /** 发送原始消息 */
    private _send;
    /** 处理收到的消息 */
    private _handleMessage;
    /** 处理断开连接 */
    private _handleDisconnect;
    /** 自动重连 */
    private _reconnect;
    /** 停止重连 */
    private _stopReconnect;
    /** 启动心跳检测 */
    private _startHeartbeat;
    /** 停止心跳检测 */
    private _stopHeartbeat;
    /** 重置 pong 超时 */
    private _resetPingTimeout;
    /** 发送队列中的消息 */
    private _flushQueue;
    /** 清除所有待处理的请求 */
    private _clearPendingRequests;
    /** 获取统计信息 */
    getStats(): {
        state: ConnectionState;
        pendingRequests: number;
        queuedMessages: number;
        reconnectAttempts: number;
    };
}
//# sourceMappingURL=godot-connection.d.ts.map