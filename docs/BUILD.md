# 编译指南

## 环境要求

### 通用依赖

| 依赖 | 最低版本 | 推荐版本 | 说明 |
|------|---------|---------|------|
| Python | 3.6 | 3.12+ | SCons 构建系统 |
| SCons | 4.0 | 4.8+ | Godot 构建工具 |
| C++ 编译器 | 见下方 | — | 平台相关 |
| Node.js | 18.0 | 20 LTS | AI Bridge 进程 |
| npm | 8.0 | 10+ | 包管理器 |

### Windows

| 依赖 | 说明 |
|------|------|
| Visual Studio 2022 | 需安装 "使用 C++ 的桌面开发" 工作负载 |
| Windows SDK | 10.0.19041.0 或更高 |
| MinGW-w64 | 替代方案（不推荐） |

### Linux

| 依赖 | 安装命令（Ubuntu/Debian） |
|------|--------------------------|
| GCC 12+ | `sudo apt install build-essential` |
| Clang 15+ | `sudo apt install clang` |
| pkg-config | `sudo apt install pkg-config` |
| X11 开发库 | `sudo apt install libx11-dev libxcursor-dev libxrandr-dev libxi-dev` |
| Wayland 开发库 | `sudo apt install libwayland-dev` |
| ALSA | `sudo apt install libasound2-dev` |
| PulseAudio | `sudo apt install libpulse-dev` |

### macOS

| 依赖 | 说明 |
|------|------|
| Xcode 14+ | `xcode-select --install` |
| Command Line Tools | `xcode-select --install` |
| Homebrew | 可选，用于安装辅助工具 |

## 编译步骤

### 第一步：准备源码

```bash
# 克隆 AI-Native Godot（含子模块）
git clone --recursive https://github.com/your-org/godot-ai-native.git
cd godot-ai-native

# 如果已克隆但未初始化子模块
git submodule update --init --recursive
```

### 第二步：安装 SCons

```bash
pip install scons
```

验证安装：

```bash
scons --version
```

### 第三步：编译 Godot 编辑器

#### Windows（MSVC）

```bash
# 打开 "x64 Native Tools Command Prompt for VS 2022"
scons platform=windows target=editor arch=x86_64 -j8
```

编译产物位于 `bin/godot.editor.windows.x86_64.exe`。

#### Linux

```bash
scons platform=linuxbsd target=editor arch=x86_64 -j$(nproc)
```

编译产物位于 `bin/godot.editor.linuxbsd.x86_64`。

#### macOS

```bash
scons platform=macos target=editor arch=x86_64 -j$(sysctl -n hw.ncpu)
# 或 Apple Silicon
scons platform=macos target=editor arch=arm64 -j$(sysctl -n hw.ncpu)
```

编译产物位于 `bin/godot.editor.macos.x86_64`（或 `.arm64`）。

### 第四步：编译 AI Bridge

```bash
cd ai-bridge
npm install
npm run build
```

编译产物位于 `ai-bridge/dist/`，入口为 `ai-bridge/bin/godot-ai-bridge.js`。

### 第五步：验证

```bash
# 启动编辑器
./bin/godot.editor.windows.x86_64

# 在另一个终端启动 AI Bridge
cd ai-bridge
node bin/godot-ai-bridge.js

# AI Bridge 应输出连接成功信息
# > Connected to Godot MCP at ws://127.0.0.1:8765
```

## 编译导出模板（可选）

如需导出游戏，还需编译导出模板：

```bash
# Debug 模板
scons platform=windows target=template_debug arch=x86_64 -j8

# Release 模板
scons platform=windows target=template_release arch=x86_64 -j8
```

## 自定义编译选项

### MCP Module 相关选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `mcp_enabled=yes` | `yes` | 启用 MCP Editor Module |
| `mcp_websocket=yes` | `yes` | 启用 WebSocket 通信层 |
| `mcp_sandbox=yes` | `yes` | 启用沙箱隔离 |
| `mcp_events=yes` | `yes` | 启用事件总线 |
| `mcp_snapshot=yes` | `yes` | 启用状态快照 |

禁用 MCP Module：

```bash
scons platform=windows target=editor mcp_enabled=no
```

### 通用 Godot 编译选项

| 选项 | 说明 |
|------|------|
| `-j N` | 并行编译数（推荐 CPU 核心数） |
| `dev_build=yes` | 开发构建（更多调试信息） |
| `debug_symbols=yes` | 保留调试符号 |
| `lto=auto` | 链接时优化（Release 推荐） |
| `use_mingw=yes` | Windows 使用 MinGW（不推荐） |
| `use_llvm=yes` | Linux/macOS 使用 Clang |

## 常见编译问题

### 1. SCons 找不到

```
'scons' is not recognized as an internal or external command
```

**解决**：确保 Python Scripts 目录在 PATH 中：

```bash
pip install scons
python -m SCons --version  # 备选方式
```

### 2. MSVC 编译器未找到

```
No compiler found. Please ensure Visual Studio is installed.
```

**解决**：

- 确认已安装 Visual Studio 2022 及 "使用 C++ 的桌面开发" 工作负载
- 在 "x64 Native Tools Command Prompt for VS 2022" 中运行编译命令

### 3. 内存不足

```
fatal error: out of memory
```

**解决**：

- 减少并行数：`-j4` 替代 `-j8`
- 关闭其他占用内存的程序
- 增加虚拟内存 / 交换空间

### 4. 子模块缺失

```
module/mcp_editor/core/mcp_command_queue.h: No such file or directory
```

**解决**：

```bash
git submodule update --init --recursive
```

### 5. Node.js 版本过低

```
SyntaxError: Unexpected token '...'
```

**解决**：升级 Node.js 到 18+：

```bash
nvm install 20
nvm use 20
```

### 6. WebSocket 连接失败

```
Error: connect ECONNREFUSED 127.0.0.1:8765
```

**解决**：

- 确认 Godot 编辑器已启动
- 检查 `editor_settings` 中 `mcp/server_enabled` 是否为 `true`
- 确认端口 8765 未被占用

## 交叉编译

### Windows → Linux（使用交叉编译工具链）

```bash
# 需要安装交叉编译工具链
sudo apt install mingw-w64
scons platform=windows target=editor arch=x86_64 use_mingw=yes -j8
```

> ⚠️ 不推荐交叉编译。建议在目标平台上直接编译。

## CI/CD 集成

### GitHub Actions 示例

```yaml
name: Build
on: [push, pull_request]
jobs:
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - uses: actions/setup-python@v5
        with:
          python-version: '3.12'
      - run: pip install scons
      - run: scons platform=windows target=editor arch=x86_64 -j4
```

## 编译时间参考

| 平台 | 硬件 | 并行数 | 编译时间（首次） | 增量编译 |
|------|------|--------|----------------|---------|
| Windows | i7-12700K, 32GB | -j8 | ~25 分钟 | ~2 分钟 |
| Linux | Ryzen 9 5900X, 64GB | -j12 | ~20 分钟 | ~1.5 分钟 |
| macOS | M2 Pro, 16GB | -j8 | ~30 分钟 | ~3 分钟 |
