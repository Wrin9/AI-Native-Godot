/**
 * MCP stdio 服务器
 * 实现 Model Context Protocol（JSON-RPC 2.0 over stdio）
 * 与 Craft Agent 的 MCP 客户端完全兼容
 */
import { stdin, stdout, stderr } from 'node:process';
import { isJsonRpcRequest, isJsonRpcNotification } from './types.js';
/** 支持的 MCP 协议版本 */
const SUPPORTED_PROTOCOL_VERSIONS = ['2025-03-26', '2024-11-05'];
/** JSON-RPC 标准错误码 */
const ERROR_CODES = {
    PARSE_ERROR: -32700,
    INVALID_REQUEST: -32600,
    METHOD_NOT_FOUND: -32601,
    INVALID_PARAMS: -32602,
    INTERNAL_ERROR: -32603,
    RATE_LIMITED: -32000,
    TIMEOUT: -32001,
    NOT_CONNECTED: -32002,
};
export class MCPServer {
    godot;
    stateManager;
    rateLimiter;
    validator;
    serverName;
    serverVersion;
    buffer = '';
    queue = Promise.resolve();
    running = false;
    constructor(options) {
        this.godot = options.godotConnection;
        this.stateManager = options.stateManager;
        this.rateLimiter = options.rateLimiter;
        this.validator = options.validator;
        this.serverName = options.name ?? 'godot-ai-bridge';
        this.serverVersion = options.version ?? '1.0.0';
    }
    /** 启动 MCP 服务器 */
    start() {
        if (this.running) {
            return;
        }
        this.running = true;
        // 设置 stdin 读取
        stdin.setEncoding('utf8');
        stdin.on('data', (chunk) => {
            this.buffer += chunk;
            this._drainBuffer(false);
        });
        stdin.on('end', () => {
            this._drainBuffer(true);
            this.running = false;
        });
        stdin.on('error', (error) => {
            stderr.write(`[mcp] stdin 错误: ${error.message}\n`);
        });
        // 初始连接 Godot
        this._connectToGodot();
        stderr.write(`[mcp] AI Bridge 已启动 (${this.serverName} v${this.serverVersion})\n`);
    }
    /** 连接到 Godot */
    async _connectToGodot() {
        try {
            await this.godot.connect();
        }
        catch (error) {
            const msg = error instanceof Error ? error.message : String(error);
            stderr.write(`[mcp] 初始连接 Godot 失败: ${msg}，将自动重连\n`);
        }
    }
    /** 排空输入缓冲区 */
    _drainBuffer(flush) {
        while (true) {
            const index = this.buffer.indexOf('\n');
            if (index < 0) {
                break;
            }
            const line = this.buffer.slice(0, index);
            this.buffer = this.buffer.slice(index + 1);
            this._enqueueLine(line);
        }
        if (flush && this.buffer.trim() !== '') {
            this._enqueueLine(this.buffer);
            this.buffer = '';
        }
    }
    /** 将一行加入处理队列 */
    _enqueueLine(line) {
        this.queue = this.queue
            .then(() => this._handleLine(line))
            .catch((error) => {
            stderr.write(`[mcp] 处理消息错误: ${error.stack ?? error.message}\n`);
        });
    }
    /** 处理一行输入 */
    async _handleLine(line) {
        const trimmed = line.trim();
        if (trimmed === '') {
            return;
        }
        let message;
        try {
            message = JSON.parse(trimmed);
        }
        catch {
            this._writeMessage({
                jsonrpc: '2.0',
                id: null,
                error: {
                    code: ERROR_CODES.PARSE_ERROR,
                    message: 'Parse error: 无法解析 JSON',
                },
            });
            return;
        }
        // 批量消息
        if (Array.isArray(message)) {
            const responses = [];
            for (const item of message) {
                const response = await this._handleMessage(item);
                if (response !== null) {
                    responses.push(response);
                }
            }
            if (responses.length > 0) {
                this._writeMessage(responses);
            }
            return;
        }
        // 单条消息
        const response = await this._handleMessage(message);
        if (response !== null) {
            this._writeMessage(response);
        }
    }
    /** 处理单条消息 */
    async _handleMessage(message) {
        if (message === null || typeof message !== 'object' || Array.isArray(message)) {
            return this._buildError(null, ERROR_CODES.INVALID_REQUEST, 'Invalid Request');
        }
        const msg = message;
        // JSON-RPC 2.0 版本检查
        if (msg.jsonrpc !== '2.0') {
            return this._buildError(msg.id ?? null, ERROR_CODES.INVALID_REQUEST, 'Invalid Request: jsonrpc 必须为 "2.0"');
        }
        // 通知消息（没有 id）— 不需要响应
        if (isJsonRpcNotification(msg)) {
            await this._handleNotification(msg.method, msg.params);
            return null;
        }
        // 请求消息 — 需要响应
        if (isJsonRpcRequest(msg)) {
            return this._handleRequest(msg.id, msg.method, msg.params);
        }
        // 响应消息 — 忽略
        return null;
    }
    /** 处理请求 */
    async _handleRequest(id, method, params) {
        switch (method) {
            case 'initialize':
                return this._handleInitialize(id, params);
            case 'notifications/initialized':
                // 客户端完成初始化的通知（作为请求处理是错误的，但容错）
                return this._buildResult(id, {});
            case 'ping':
                return this._buildResult(id, {});
            case 'tools/list':
                return this._handleToolsList(id);
            case 'tools/call':
                return this._handleToolsCall(id, params);
            case 'resources/list':
                return this._handleResourcesList(id);
            case 'resources/read':
                return this._handleResourcesRead(id, params);
            case 'resources/templates/list':
                return this._buildResult(id, { resourceTemplates: [] });
            case 'prompts/list':
                return this._handlePromptsList(id);
            case 'prompts/get':
                return this._handlePromptsGet(id, params);
            case 'logging/setLevel':
                // 日志级别设置（暂不实现）
                return this._buildResult(id, {});
            case 'completion/complete':
                // 自动补全（暂不实现）
                return this._buildResult(id, { completion: { values: [], total: 0, hasMore: false } });
            default:
                return this._buildError(id, ERROR_CODES.METHOD_NOT_FOUND, `方法未找到: ${method}`);
        }
    }
    /** 处理通知 */
    async _handleNotification(method, _params) {
        switch (method) {
            case 'notifications/initialized':
                stderr.write('[mcp] 客户端已完成初始化\n');
                break;
            case 'notifications/cancelled':
                // 请求取消通知（暂不实现）
                stderr.write('[mcp] 收到请求取消通知\n');
                break;
            case 'notifications/progress':
                // 进度通知（暂不实现）
                break;
            default:
                stderr.write(`[mcp] 未知通知: ${method}\n`);
        }
    }
    /** 处理 initialize 请求 */
    _handleInitialize(id, params) {
        const clientProtocolVersion = params?.protocolVersion;
        // 协议版本协商
        let protocolVersion = '2025-03-26';
        if (clientProtocolVersion && SUPPORTED_PROTOCOL_VERSIONS.includes(clientProtocolVersion)) {
            protocolVersion = clientProtocolVersion;
        }
        const result = {
            protocolVersion,
            capabilities: {
                tools: { listChanged: true },
                resources: { subscribe: true, listChanged: true },
                prompts: { listChanged: true },
                logging: {},
            },
            serverInfo: {
                name: this.serverName,
                version: this.serverVersion,
            },
        };
        return this._buildResult(id, result);
    }
    /** 处理 tools/list 请求 */
    async _handleToolsList(id) {
        // 先尝试从缓存获取
        const cachedTools = this.stateManager.getCachedTools();
        if (cachedTools) {
            return this._buildResult(id, { tools: cachedTools });
        }
        // 从 Godot 获取
        if (!this.godot.isConnected()) {
            // 未连接时返回空列表
            return this._buildResult(id, { tools: [] });
        }
        try {
            const result = await this.godot.request('tools/list');
            const tools = result?.tools ?? [];
            // 更新缓存和校验器
            this.stateManager.setCachedTools(tools);
            this.validator.updateSchemas(tools);
            return this._buildResult(id, { tools });
        }
        catch (error) {
            const msg = error instanceof Error ? error.message : String(error);
            stderr.write(`[mcp] 获取工具列表失败: ${msg}\n`);
            return this._buildError(id, ERROR_CODES.INTERNAL_ERROR, `获取工具列表失败: ${msg}`);
        }
    }
    /** 处理 tools/call 请求 */
    async _handleToolsCall(id, params) {
        if (!params?.name) {
            return this._buildError(id, ERROR_CODES.INVALID_PARAMS, '缺少工具名称');
        }
        const { name, arguments: args } = params;
        const toolArgs = args ?? {};
        // 1. 参数校验
        const validation = this.validator.validate(name, toolArgs);
        if (!validation.valid) {
            const toolResult = {
                content: [{
                        type: 'text',
                        text: `参数校验失败:\n${validation.errors.map((e) => `  - ${e}`).join('\n')}`,
                    }],
                isError: true,
            };
            return this._buildResult(id, toolResult);
        }
        // 2. 连接检查
        if (!this.godot.isConnected()) {
            const toolResult = {
                content: [{
                        type: 'text',
                        text: 'Godot 编辑器未连接。请确保 Godot 已启动且 AI Native 模块已启用。',
                    }],
                isError: true,
            };
            return this._buildResult(id, toolResult);
        }
        // 3. 限流检查
        if (!this.rateLimiter.canExecute()) {
            const waitTime = this.rateLimiter.getWaitTime();
            const toolResult = {
                content: [{
                        type: 'text',
                        text: `请求过于频繁，请 ${waitTime}ms 后重试。`,
                    }],
                isError: true,
            };
            return this._buildResult(id, toolResult);
        }
        // 4. 消耗令牌
        this.rateLimiter.tryConsume();
        // 5. 转发到 Godot
        try {
            const result = await this.godot.request(`tools/${name}`, toolArgs);
            // 将结果转换为 MCP ToolCallResult 格式
            const toolResult = this._formatToolResult(result);
            // 6. 缓存相关状态
            this._updateCacheAfterToolCall(name, toolArgs, result);
            return this._buildResult(id, toolResult);
        }
        catch (error) {
            const msg = error instanceof Error ? error.message : String(error);
            const toolResult = {
                content: [{
                        type: 'text',
                        text: `工具调用失败: ${msg}`,
                    }],
                isError: true,
            };
            return this._buildResult(id, toolResult);
        }
    }
    /** 格式化工具调用结果为 MCP 标准格式 */
    _formatToolResult(result) {
        if (result === null || result === undefined) {
            return {
                content: [{ type: 'text', text: '操作完成（无返回值）' }],
            };
        }
        // 如果结果已经是 MCP 格式
        if (typeof result === 'object' && result !== null && 'content' in result) {
            return result;
        }
        // 字符串结果
        if (typeof result === 'string') {
            return {
                content: [{ type: 'text', text: result }],
            };
        }
        // 其他类型转为 JSON 文本
        return {
            content: [{
                    type: 'text',
                    text: typeof result === 'object' ? JSON.stringify(result, null, 2) : String(result),
                }],
        };
    }
    /** 工具调用后更新缓存 */
    _updateCacheAfterToolCall(toolName, _args, _result) {
        // 根据工具类型使相关缓存失效
        if (['create_node', 'delete_node', 'set_node_property'].includes(toolName)) {
            this.stateManager.invalidate('scene_tree');
        }
        if (toolName === 'save_scene') {
            this.stateManager.invalidate('scene_tree');
            this.stateManager.invalidate('project_info');
        }
    }
    /** 处理 resources/list 请求 */
    async _handleResourcesList(id) {
        // 尝试从 Godot 获取资源列表
        if (!this.godot.isConnected()) {
            return this._buildResult(id, { resources: [] });
        }
        try {
            const result = await this.godot.request('resources/list');
            return this._buildResult(id, { resources: result?.resources ?? [] });
        }
        catch {
            // 失败时返回空列表
            return this._buildResult(id, { resources: [] });
        }
    }
    /** 处理 resources/read 请求 */
    async _handleResourcesRead(id, params) {
        if (!params?.uri) {
            return this._buildError(id, ERROR_CODES.INVALID_PARAMS, '缺少资源 URI');
        }
        if (!this.godot.isConnected()) {
            return this._buildError(id, ERROR_CODES.NOT_CONNECTED, 'Godot 编辑器未连接');
        }
        try {
            const result = await this.godot.request('resources/read', { uri: params.uri });
            return this._buildResult(id, result);
        }
        catch (error) {
            const msg = error instanceof Error ? error.message : String(error);
            return this._buildError(id, ERROR_CODES.INTERNAL_ERROR, `读取资源失败: ${msg}`);
        }
    }
    /** 处理 prompts/list 请求 */
    async _handlePromptsList(id) {
        // 内置 prompts
        const prompts = [
            {
                name: 'godot-development',
                description: 'Godot 游戏开发辅助提示',
                arguments: [
                    { name: 'task', description: '开发任务描述', required: true },
                    { name: 'context', description: '额外上下文信息' },
                ],
            },
            {
                name: 'scene-analysis',
                description: '场景结构分析提示',
                arguments: [
                    { name: 'scene_path', description: '场景路径' },
                ],
            },
        ];
        return this._buildResult(id, { prompts });
    }
    /** 处理 prompts/get 请求 */
    _handlePromptsGet(id, params) {
        const name = params?.name;
        if (!name) {
            return this._buildError(id, ERROR_CODES.INVALID_PARAMS, '缺少 prompt 名称');
        }
        switch (name) {
            case 'godot-development': {
                const task = params?.arguments?.task ?? '';
                const context = params?.arguments?.context ?? '';
                return this._buildResult(id, {
                    description: 'Godot 游戏开发辅助提示',
                    messages: [
                        {
                            role: 'user',
                            content: {
                                type: 'text',
                                text: `你是一个 Godot 游戏开发专家。\n\n任务: ${task}${context ? `\n\n上下文: ${context}` : ''}`,
                            },
                        },
                    ],
                });
            }
            case 'scene-analysis': {
                const scenePath = params?.arguments?.scene_path ?? '当前场景';
                return this._buildResult(id, {
                    description: '场景结构分析提示',
                    messages: [
                        {
                            role: 'user',
                            content: {
                                type: 'text',
                                text: `分析 Godot 场景: ${scenePath}\n\n请提供场景的节点结构、关键组件和潜在优化建议。`,
                            },
                        },
                    ],
                });
            }
            default:
                return this._buildError(id, ERROR_CODES.METHOD_NOT_FOUND, `Prompt 未找到: ${name}`);
        }
    }
    /** 构建成功响应 */
    _buildResult(id, result) {
        return {
            jsonrpc: '2.0',
            id,
            result,
        };
    }
    /** 构建错误响应 */
    _buildError(id, code, message, data) {
        const error = { code, message };
        if (data !== undefined) {
            error.data = data;
        }
        return { jsonrpc: '2.0', id, error };
    }
    /** 写消息到 stdout */
    _writeMessage(message) {
        stdout.write(`${JSON.stringify(message)}\n`);
    }
}
//# sourceMappingURL=mcp-server.js.map