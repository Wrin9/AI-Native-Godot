#!/bin/bash
# ====================================================================
#  AI-Native Godot — Linux/macOS 构建脚本
#
#  用法: ./build.sh <godot-source-path> [scons-options...]
#  示例: ./build.sh ~/godot-source
#        ./build.sh ~/godot-source dev_build=yes
# ====================================================================

set -euo pipefail

echo "========================================"
echo " Building AI-Native Godot (Unix)"
echo "========================================"

GODOT_SOURCE="${1:-}"
if [ -z "$GODOT_SOURCE" ]; then
    echo ""
    echo "错误: 未指定 Godot 源码路径"
    echo ""
    echo "用法: $0 <path-to-godot-source> [scons-options...]"
    echo "示例: $0 ~/godot-source"
    echo "      $0 ~/godot-source dev_build=yes"
    exit 1
fi

# ---- 检查 Godot 源码目录 ----
if [ ! -f "$GODOT_SOURCE/SConstruct" ]; then
    echo ""
    echo "错误: '$GODOT_SOURCE' 不是有效的 Godot 源码目录"
    echo "      找不到 SConstruct 文件"
    exit 1
fi

# ---- 复制模块到 Godot 源码 ----
echo ""
echo "[1/3] 复制模块到 Godot 源码..."
MODULE_DIR="$GODOT_SOURCE/modules/mcp_editor"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# 清理旧版本
if [ -d "$MODULE_DIR" ]; then
    echo "      清理旧的模块目录..."
    rm -rf "$MODULE_DIR"
fi

# 复制所有文件
cp -r "$SCRIPT_DIR/module" "$MODULE_DIR"

# 创建 doc_classes 目录（Godot 文档系统需要）
mkdir -p "$MODULE_DIR/doc_classes"

echo "      完成"

# ---- 收集额外 SCons 参数 ----
shift 2>/dev/null || true
SCONS_EXTRA="$*"

# ---- 确定平台 ----
PLATFORM="linuxbsd"
if [ "$(uname)" = "Darwin" ]; then
    PLATFORM="macos"
fi

# ---- 编译 ----
echo ""
echo "[2/3] 编译 Godot (启用 MCP Editor 模块)..."
echo "      目标平台: $PLATFORM"
echo "      并行度:   $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) 个 CPU 核心"
echo "      额外参数: $SCONS_EXTRA"

cd "$GODOT_SOURCE"
scons platform=$PLATFORM target=editor module_mcp_editor_enabled=yes -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) $SCONS_EXTRA

# ---- 完成 ----
echo ""
echo "[3/3] 编译成功!"
echo "========================================"
echo " 构建完成"
echo "========================================"
