# 贡献指南

感谢你对 AI-Native Godot 项目的关注！本文档介绍如何参与项目开发。

## 项目结构

```
godot-ai-native/
├── ai-bridge/          # Node.js MCP Bridge
│   ├── src/            # TypeScript 源码
│   └── dist/           # 编译输出
├── module/             # Godot C++ Module
│   ├── core/           # 核心组件
│   ├── tools/          # 工具分派
│   └── build/          # 构建配置
├── reference/          # 原版参考实现（只读）
└── docs/               # 文档
```

## 开发环境设置

### 1. Fork 并克隆

```bash
git clone --recursive https://github.com/YOUR_USERNAME/godot-ai-native.git
cd godot-ai-native
git remote add upstream https://github.com/your-org/godot-ai-native.git
```

### 2. 编译项目

参见 [BUILD.md](./BUILD.md)。

### 3. 开发模式

```bash
# AI Bridge 开发模式（热重载）
cd ai-bridge
npm run dev

# Godot Module 需要重新编译
scons platform=windows target=editor dev_build=yes -j8
```

## 贡献流程

### 提交 Issue

- 🐛 **Bug 报告**：使用 Bug 模板，包含复现步骤、预期行为和实际行为
- ✨ **功能请求**：描述使用场景和期望行为
- 📖 **文档改进**：指出不清楚或缺失的部分

### 提交 Pull Request

1. **创建分支**：
   ```bash
   git checkout -b feature/my-feature
   # 或
   git checkout -b fix/my-fix
   ```

2. **编码规范**：
   - C++：遵循 [Godot 代码风格](https://docs.godotengine.org/en/latest/contributing/development/code_style_guidelines.html)
   - TypeScript：遵循项目 ESLint 配置
   - 提交信息：使用[约定式提交](https://www.conventionalcommits.org/)

3. **测试**：
   - 确保编译通过
   - 手动测试变更功能
   - 验证不影响现有功能

4. **提交 PR**：
   - 标题清晰描述变更
   - 关联相关 Issue
   - 包含变更说明

### 提交信息规范

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Type**:
- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档变更
- `style`: 代码格式（不影响功能）
- `refactor`: 重构
- `perf`: 性能优化
- `test`: 测试
- `chore`: 构建或辅助工具

**Scope**:
- `module`: Godot C++ Module
- `bridge`: AI Bridge (Node.js)
- `tools`: MCP 工具
- `docs`: 文档

**示例**:
```
feat(tools): add batch_node_create tool for bulk node creation
fix(module): prevent crash when sandbox rollback encounters deleted parent
docs(bridge): update WebSocket reconnection strategy
```

## C++ Module 开发指南

### 目录规范

- `module/core/` — 核心组件（CommandQueue, Sandbox, EventBus, WebSocketServer, Snapshot）
- `module/tools/` — 工具实现（每个工具组一个文件）
- `module/build/` — 构建脚本和配置

### 新增工具

1. 在 `module/tools/` 中实现工具函数：
   ```cpp
   // module/tools/scene_tools.h
   Dictionary execute_create_node(const Dictionary &arguments);
   ```

2. 在 `ToolDispatcher` 中注册：
   ```cpp
   dispatcher.register_tool("create_node", execute_create_node, "nodes", "full");
   ```

3. 确保工具在沙箱中可安全执行。

### 安全规范

- **所有写操作**必须通过 Sandbox 执行
- **所有文件操作**必须在项目目录内
- **命令超时**必须可配置
- **回滚**必须覆盖所有变更类型

## AI Bridge 开发指南

### 目录规范

- `src/index.ts` — 入口，MCP stdio 服务
- `src/websocket-client.ts` — WebSocket 客户端
- `src/rate-limiter.ts` — 令牌桶限流
- `src/batch-processor.ts` — 批处理合并
- `src/state-manager.ts` — 状态管理和增量同步

### 新增功能

1. TypeScript 严格模式，不允许 `any`
2. 所有异步操作使用 `async/await`
3. 错误处理使用自定义错误类
4. 日志输出到 stderr（stdout 保留给 MCP 协议）

## 代码审查标准

- [ ] 编译通过（目标平台）
- [ ] 遵循代码风格
- [ ] 不引入新的编译警告
- [ ] 沙箱安全（写操作）
- [ ] 限流兼容（高频率调用）
- [ ] 事件推送兼容（状态变更通知）
- [ ] 向后兼容（原版工具 API 不破坏）

## 文档贡献

文档使用中文编写，位于 `docs/` 目录：

- [README.md](./README.md) — 项目主文档
- [ARCHITECTURE.md](./ARCHITECTURE.md) — 架构设计
- [BUILD.md](./BUILD.md) — 编译指南
- [MCP-TOOLS.md](./MCP-TOOLS.md) — 工具参考

文档更新应与代码变更同步提交。

## 社区规范

- 友善、尊重、建设性
- 专注于问题本身，而非个人
- 欢迎不同水平的贡献者
- 提问前先搜索现有 Issue

## 许可证

贡献的代码遵循 MIT License，提交 PR 即表示同意。
