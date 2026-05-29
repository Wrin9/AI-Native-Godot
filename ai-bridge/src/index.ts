/**
 * AI Bridge 主入口
 * 启动 MCP stdio 服务器并连接到 Godot 编辑器
 */

import { MCPServer } from './mcp-server.js';
import { GodotConnection } from './godot-connection.js';
import { StateManager } from './state-manager.js';
import { RateLimiter } from './rate-limiter.js';
import { BatchProcessor } from './batch-processor.js';
import { RequestValidator } from './request-validator.js';

/** 配置常量 */
const GODOT_URL = process.env.GODOT_MCP_URL || 'ws://127.0.0.1:8765';
const BRIDGE_NAME = 'godot-ai-bridge';
const BRIDGE_VERSION = '1.0.0';

/**
 * 主函数 - 初始化并启动 AI Bridge
 */
export function main(): void {
  // 初始化各模块
  const stateManager = new StateManager();
  const rateLimiter = new RateLimiter({
    maxTokens: 10,   // 最大 10 个令牌
    refillRate: 5,    // 每秒补充 5 个
  });
  const batchProcessor = new BatchProcessor({
    windowMs: 100,    // 100ms 批处理窗口
  });
  const validator = new RequestValidator();

  // 创建 Godot 连接
  const godot = new GodotConnection(GODOT_URL);

  // 监听 Godot 事件
  godot.on('connected', () => {
    console.error('[bridge] 已连接到 Godot 编辑器');

    // 连接后获取工具列表，更新缓存和校验器
    godot.request('tools/list')
      .then((result) => {
        const tools = (result as { tools?: unknown[] })?.tools;
        if (Array.isArray(tools)) {
          stateManager.setCachedTools(tools as import('./types.js').ToolDefinition[]);
          validator.updateSchemas(tools as import('./types.js').ToolDefinition[]);
          console.error(`[bridge] 已缓存 ${tools.length} 个工具定义`);
        }
      })
      .catch((err: Error) => {
        console.error(`[bridge] 获取工具列表失败: ${err.message}`);
      });
  });

  godot.on('disconnected', () => {
    console.error('[bridge] 已断开与 Godot 编辑器的连接');
  });

  godot.on('reconnecting', (attempt: number) => {
    console.error(`[bridge] 正在重连 Godot (第 ${attempt} 次)...`);
  });

  godot.on('event', (event: import('./types.js').GodotEvent) => {
    // 处理 Godot 推送的事件，更新缓存
    stateManager.applyEvent(event);
    console.error(`[bridge] 收到 Godot 事件: ${event.type}`);
  });

  // 创建 MCP 服务器
  const mcp = new MCPServer({
    godotConnection: godot,
    stateManager,
    rateLimiter,
    batchProcessor,
    validator,
    name: BRIDGE_NAME,
    version: BRIDGE_VERSION,
  });

  // 启动 MCP 服务器
  mcp.start();

  console.error(`[bridge] AI Bridge 已启动`);
  console.error(`[bridge] Godot WebSocket: ${GODOT_URL}`);
  console.error(`[bridge] 版本: ${BRIDGE_VERSION}`);

  // 优雅退出
  const cleanup = (): void => {
    console.error('[bridge] 正在关闭...');
    godot.disconnect();
    process.exit(0);
  };

  process.on('SIGINT', cleanup);
  process.on('SIGTERM', cleanup);
  process.on('SIGHUP', cleanup);
}

// 直接运行时启动
main();
