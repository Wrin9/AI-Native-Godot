# AI-Native Godot MCP Editor

基于 Godot 4.6.3 的 AI 原生编辑器，内置 MCP HTTP Server。

## 快速启动

### 方式一：双击启动
```
双击 start-mcp-editor.bat
```

### 方式二：命令行启动
```bash
# 直接运行
godot-mcp-editor.exe --path "你的项目路径" -e

# 或指定端口
godot-mcp-editor.exe --path "你的项目路径" -e
# 然后在项目设置中添加 mcp_editor/server_port = 自定义端口
```

## MCP 协议

- **端点**: `http://127.0.0.1:9877/`
- **协议**: HTTP JSON-RPC (MCP 2024-11-05 / 2025-11-25)
- **工具数**: 94 个

### 支持的 MCP 方法

| 方法 | 说明 |
|------|------|
| `initialize` | MCP 握手 |
| `tools/list` | 列出所有 94 个工具 |
| `tools/call` | 调用工具 |
| `resources/list` | 列出资源（返回空） |
| `prompts/list` | 列出提示（返回空） |
| `ping` | 心跳检测 |

### 工具分类

| 分组 | 工具数 | 说明 |
|------|--------|------|
| scene | 12 | 场景文件管理、场景树操作 |
| node | 18 | 节点创建、删除、修改、查找 |
| script | 8 | 脚本创建、编辑、验证 |
| file | 9 | 文件系统操作 |
| play | 13 | 播放模式控制、输入模拟 |
| diagnostic | 6 | 控制台、性能、场景分析 |
| ui | 12 | UI/Control 节点创建和布局 |
| animation | 5 | 动画播放器管理 |
| material | 2 | 材质创建和分配 |
| project | 15 | 项目设置、插件、自动加载 |

### 示例请求

```bash
# 初始化
curl -X POST http://127.0.0.1:9877/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{},"clientInfo":{"name":"my-client","version":"1.0"}}}'

# 列出工具
curl -X POST http://127.0.0.1:9877/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}'

# 获取场景树
curl -X POST http://127.0.0.1:9877/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"get_scene_tree","arguments":{}}}'

# 创建节点
curl -X POST http://127.0.0.1:9877/ \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{"name":"create_node","arguments":{"parent_path":"root","node_type":"Sprite2D","name":"MySprite"}}}'
```

## 技术细节

- **引擎**: Godot 4.6.3 (commit 35e80b3)
- **模块**: `mcp_editor` C++ Module（编译进引擎）
- **协议**: HTTP + JSON-RPC（无 WebSocket 依赖）
- **线程安全**: 主线程轮询，无多线程竞争
- **端口**: 默认 9877，可通过 `ProjectSettings:mcp_editor/server_port` 配置

## 构建

如需重新编译：
```bash
cd F:\game\godot-source
python -m SCons platform=windows target=editor d3d12=no module_mcp_editor_enabled=yes -j8
```
