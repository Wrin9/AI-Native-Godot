# AI-Native Godot

通过 MCP 协议连接 AI-Native Godot 编辑器，让 AI 直接控制游戏开发。

## 前置要求

1. AI-Native Godot 编辑器已编译并运行（参考 `docs/BUILD.md`）
2. MCP Editor 模块已启用（默认启用）
3. AI Bridge 进程可用（Source 自动启动）

## 连接

Source 通过 stdio MCP 协议与 AI Bridge 通信，AI Bridge 通过 WebSocket 连接到 Godot 编辑器。

**通信链路**：
```
Craft Agent → stdio JSON-RPC → AI Bridge → WebSocket → Godot Editor Module
```

**默认端口**：`8765`（WebSocket）

**环境变量**：

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `GODOT_MCP_URL` | `ws://127.0.0.1:8765` | Godot MCP WebSocket 地址 |
| `MCP_RATE_LIMIT` | `30` | 每秒最大请求数 |
| `MCP_BATCH_SIZE` | `10` | 批处理最大合并数 |

## 启动步骤

1. 启动 AI-Native Godot 编辑器
2. 打开或创建项目
3. MCP Server 自动启动（确认输出面板中有 MCP 相关日志）
4. 在 Craft Agent 中启用此 Source

## 功能

### 场景编辑与管理
- 场景创建、打开、保存
- 场景树浏览和搜索
- **增强**：原子事务、自动回滚

### 节点操作
- 节点创建、修改、删除、复制、重命名、重新父级化
- 属性获取和设置（支持批量）
- 变换设置（2D / 3D）
- **增强**：批量操作合并

### 脚本
- GDScript / C# 脚本创建、编辑、补丁
- 脚本验证和错误检测
- .NET 项目信息

### UI / HUD 构建
- CanvasLayer / Control 创建
- Label、Button、Panel、Container 等控件
- 布局系统（锚点、偏移、尺寸标志）
- 主题覆盖、纹理设置
- 信号连接

### 动画
- AnimationPlayer 创建
- 动画剪辑、轨道、关键帧
- 动画播放

### 播放模式
- 进入/退出播放模式
- 输入模拟（键盘、鼠标、动作）
- 时间缩放控制

### 摄像机
- Camera2D / Camera3D 配置

### 材质
- 材质创建和指定

### 项目管理
- 项目设置读写
- InputMap 编辑
- Autoload 管理
- 插件管理

### 文件操作
- 文件列表、搜索、读写、移动、复制、删除

### 诊断与调试
- 脚本错误检测
- 性能快照
- 场景复杂度分析
- 控制台日志
- 视口截图

### 实时事件推送
- 节点增删改通知
- 场景切换通知
- 播放状态变更通知
- 选区变更通知

### 状态快照与增量同步
- 版本化状态快照
- 增量 diff 同步（低带宽）

## 与 funplay-godot-mcp 的区别

| 特性 | funplay-godot-mcp | AI-Native Godot |
|------|-------------------|-----------------|
| 执行方式 | 主线程同步 | 命令队列异步 |
| 崩溃恢复 | 无 | 沙箱隔离 + 自动回滚 |
| 通信协议 | HTTP 单向 | WebSocket 双向 |
| 批量操作 | 不支持 | 批处理合并 |
| 限流 | 无 | 令牌桶限流 |
| 事件推送 | 无 | 实时事件流 |
| 增量同步 | 无 | 版本化增量快照 |
| 实现 | GDScript 插件（需安装） | C++ Module（编译内置） |
| 性能 | GDScript 开销 | C++ 原生 50-100x |
| 多客户端 | 不支持 | 最多 5 连接 |
| 断线重连 | 不支持 | 指数退避重连 |

## 工具 Profile

| Profile | 说明 |
|---------|------|
| `core` | 只读为主，安全，适合信息查询 |
| `full` | 包含写操作，完整的 AI 控制能力（默认） |

## 注意事项

- AI-Native Godot 需要自行编译（参考 `docs/BUILD.md`）
- WebSocket 默认端口 `8765`，如被占用编辑器自动递增
- 支持多客户端同时连接（最多 5 个）
- AI Bridge 断线后自动重连（指数退避，最大 30s）
- 所有写操作受沙箱保护，失败自动回滚
- 请求限流默认 30 req/s，可通过 `MCP_RATE_LIMIT` 环境变量调整
