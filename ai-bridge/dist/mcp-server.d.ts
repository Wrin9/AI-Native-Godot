/**
 * MCP stdio 服务器
 * 实现 Model Context Protocol（JSON-RPC 2.0 over stdio）
 * 与 Craft Agent 的 MCP 客户端完全兼容
 */
import type { GodotConnection } from './godot-connection.js';
import type { StateManager } from './state-manager.js';
import type { RateLimiter } from './rate-limiter.js';
import type { BatchProcessor } from './batch-processor.js';
import type { RequestValidator } from './request-validator.js';
/** MCP 服务器配置 */
export interface MCPServerOptions {
    godotConnection: GodotConnection;
    stateManager: StateManager;
    rateLimiter: RateLimiter;
    batchProcessor: BatchProcessor;
    validator: RequestValidator;
    /** 服务器名称 */
    name?: string;
    /** 服务器版本 */
    version?: string;
}
export declare class MCPServer {
    private readonly godot;
    private readonly stateManager;
    private readonly rateLimiter;
    private readonly validator;
    private readonly serverName;
    private readonly serverVersion;
    private buffer;
    private queue;
    private running;
    constructor(options: MCPServerOptions);
    /** 启动 MCP 服务器 */
    start(): void;
    /** 连接到 Godot */
    private _connectToGodot;
    /** 排空输入缓冲区 */
    private _drainBuffer;
    /** 将一行加入处理队列 */
    private _enqueueLine;
    /** 处理一行输入 */
    private _handleLine;
    /** 处理单条消息 */
    private _handleMessage;
    /** 处理请求 */
    private _handleRequest;
    /** 处理通知 */
    private _handleNotification;
    /** 处理 initialize 请求 */
    private _handleInitialize;
    /** 处理 tools/list 请求 */
    private _handleToolsList;
    /** 处理 tools/call 请求 */
    private _handleToolsCall;
    /** 格式化工具调用结果为 MCP 标准格式 */
    private _formatToolResult;
    /** 工具调用后更新缓存 */
    private _updateCacheAfterToolCall;
    /** 处理 resources/list 请求 */
    private _handleResourcesList;
    /** 处理 resources/read 请求 */
    private _handleResourcesRead;
    /** 处理 prompts/list 请求 */
    private _handlePromptsList;
    /** 处理 prompts/get 请求 */
    private _handlePromptsGet;
    /** 构建成功响应 */
    private _buildResult;
    /** 构建错误响应 */
    private _buildError;
    /** 写消息到 stdout */
    private _writeMessage;
}
//# sourceMappingURL=mcp-server.d.ts.map