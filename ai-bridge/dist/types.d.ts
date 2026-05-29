/**
 * AI Bridge 共享类型定义
 * 所有公开接口的类型都在此文件中定义
 */
/** JSON-RPC 2.0 请求 */
export interface JsonRpcRequest {
    jsonrpc: '2.0';
    id?: string | number | null;
    method: string;
    params?: Record<string, unknown>;
}
/** JSON-RPC 2.0 响应 */
export interface JsonRpcResponse {
    jsonrpc: '2.0';
    id: string | number | null;
    result?: unknown;
    error?: JsonRpcError;
}
/** JSON-RPC 2.0 错误 */
export interface JsonRpcError {
    code: number;
    message: string;
    data?: unknown;
}
/** JSON-RPC 2.0 通知（无 id） */
export interface JsonRpcNotification {
    jsonrpc: '2.0';
    method: string;
    params?: Record<string, unknown>;
}
/** JSON-RPC 消息（请求 | 响应 | 通知） */
export type JsonRpcMessage = JsonRpcRequest | JsonRpcResponse | JsonRpcNotification;
/** 判断是否为 JSON-RPC 请求（有 id 的消息） */
export declare function isJsonRpcRequest(msg: JsonRpcMessage): msg is JsonRpcRequest;
/** 判断是否为 JSON-RPC 响应 */
export declare function isJsonRpcResponse(msg: JsonRpcMessage): msg is JsonRpcResponse;
/** 判断是否为 JSON-RPC 通知（无 id） */
export declare function isJsonRpcNotification(msg: JsonRpcMessage): msg is JsonRpcNotification;
/** MCP 服务器信息 */
export interface ServerInfo {
    name: string;
    version: string;
}
/** MCP 协议能力 */
export interface ServerCapabilities {
    tools?: {
        listChanged?: boolean;
    };
    resources?: {
        subscribe?: boolean;
        listChanged?: boolean;
    };
    prompts?: {
        listChanged?: boolean;
    };
    logging?: Record<string, unknown>;
}
/** MCP 初始化结果 */
export interface InitializeResult {
    protocolVersion: string;
    capabilities: ServerCapabilities;
    serverInfo: ServerInfo;
}
/** MCP 工具定义 */
export interface ToolDefinition {
    name: string;
    description?: string;
    inputSchema?: {
        type: 'object';
        properties?: Record<string, unknown>;
        required?: string[];
    };
}
/** MCP 工具调用参数 */
export interface ToolCallParams {
    name: string;
    arguments?: Record<string, unknown>;
}
/** MCP 工具调用结果 */
export interface ToolCallResult {
    content: Array<{
        type: 'text' | 'image' | 'resource';
        text?: string;
        data?: string;
        mimeType?: string;
        resource?: {
            uri: string;
            name: string;
            mimeType?: string;
        };
    }>;
    isError?: boolean;
}
/** MCP 资源定义 */
export interface ResourceDefinition {
    uri: string;
    name: string;
    description?: string;
    mimeType?: string;
}
/** MCP 资源读取结果 */
export interface ResourceReadResult {
    contents: Array<{
        uri: string;
        mimeType?: string;
        text?: string;
        blob?: string;
    }>;
}
/** MCP Prompt 定义 */
export interface PromptDefinition {
    name: string;
    description?: string;
    arguments?: Array<{
        name: string;
        description?: string;
        required?: boolean;
    }>;
}
/** MCP Prompt 获取结果 */
export interface PromptGetResult {
    description?: string;
    messages: Array<{
        role: 'user' | 'assistant';
        content: {
            type: 'text' | 'image' | 'resource';
            text?: string;
        };
    }>;
}
/** Godot WebSocket 消息 */
export interface GodotMessage {
    jsonrpc: '2.0';
    id?: string | number;
    method?: string;
    params?: Record<string, unknown>;
    result?: unknown;
    error?: JsonRpcError;
}
/** Godot 推送事件 */
export interface GodotEvent {
    type: string;
    data: Record<string, unknown>;
    timestamp?: number;
}
/** 待处理请求 */
export interface PendingRequest {
    resolve: (value: unknown) => void;
    reject: (reason: unknown) => void;
    timer: NodeJS.Timeout;
}
/** 连接状态 */
export type ConnectionState = 'disconnected' | 'connecting' | 'connected' | 'reconnecting';
/** 优先级级别 */
export type Priority = 'low' | 'normal' | 'high' | 'critical';
/** 令牌桶配置 */
export interface RateLimiterOptions {
    maxTokens?: number;
    refillRate?: number;
}
/** 批处理配置 */
export interface BatchProcessorOptions {
    windowMs?: number;
}
/** 待处理命令 */
export interface PendingCommand {
    method: string;
    params: Record<string, unknown>;
    resolve: (value: unknown) => void;
    reject: (reason: unknown) => void;
}
/** 合并后的命令 */
export interface MergedCommand {
    method: string;
    params: Record<string, unknown>;
}
/** 缓存条目 */
export interface CacheEntry<T = unknown> {
    value: T;
    timestamp: number;
    ttl: number;
}
/** 增量更新 */
export interface DeltaUpdate {
    added: Record<string, unknown>;
    modified: Record<string, unknown>;
    removed: string[];
    timestamp: number;
}
//# sourceMappingURL=types.d.ts.map