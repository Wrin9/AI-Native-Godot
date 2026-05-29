#!/bin/bash
# ====================================================================
#  AI-Native Godot — 一键环境搭建脚本
#
#  功能:
#    1. 克隆 Godot 源码（浅克隆，节省空间和时间）
#    2. 将 MCP Editor 模块复制到 Godot 模块目录
#    3. 验证构建依赖
#
#  用法: ./setup.sh [godot-version] [target-dir]
#  示例: ./setup.sh
#        ./setup.sh 4.4-stable
#        ./setup.sh 4.6-stable ~/my-godot
# ====================================================================

set -euo pipefail

# ---- 配置 ----
GODOT_VERSION="${1:-4.6-stable}"
GODOT_DIR="${2:-godot-source}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "========================================"
echo " AI-Native Godot 环境搭建"
echo "========================================"
echo " Godot 版本: ${GODOT_VERSION}"
echo " 目标目录:   ${GODOT_DIR}"
echo ""

# ---- 1. 克隆 Godot 源码 ----
if [ -d "$GODOT_DIR" ]; then
    echo "[1/4] 目录 '${GODOT_DIR}' 已存在，跳过克隆"
else
    echo "[1/4] 克隆 Godot ${GODOT_VERSION} ..."
    echo "      (浅克隆，仅下载最新提交)"
    git clone --depth 1 --branch "${GODOT_VERSION}" \
        https://github.com/godotengine/godot.git "${GODOT_DIR}"
    echo "      完成"
fi

# ---- 2. 复制模块 ----
echo ""
echo "[2/4] 安装 MCP Editor 模块..."
MODULE_DIR="${GODOT_DIR}/modules/mcp_editor"

# 清理旧版本
if [ -d "$MODULE_DIR" ]; then
    echo "      清理旧的模块目录..."
    rm -rf "$MODULE_DIR"
fi

# 复制模块文件
cp -r "$SCRIPT_DIR/module" "$MODULE_DIR"

# 确保 doc_classes 目录存在
mkdir -p "$MODULE_DIR/doc_classes"

echo "      完成"

# ---- 3. 检查构建依赖 ----
echo ""
echo "[3/4] 检查构建依赖..."

check_command() {
    if command -v "$1" &>/dev/null; then
        echo "       ✓ $1 ($(command -v "$1"))"
        return 0
    else
        echo "       ✗ $1 — 未找到"
        return 1
    fi
}

DEPS_OK=true
check_command scons || DEPS_OK=false
check_command python3 || check_command python || DEPS_OK=false

# C++ 编译器
if [ "$(uname)" = "Darwin" ]; then
    check_command clang++ || DEPS_OK=false
else
    check_command g++ || check_command clang++ || DEPS_OK=false
fi

if [ "$DEPS_OK" = false ]; then
    echo ""
    echo "      ⚠ 缺少必要的构建依赖，请先安装："
    echo ""
    echo "      Ubuntu/Debian:"
    echo "        sudo apt install scons build-essential python3"
    echo ""
    echo "      macOS:"
    echo "        brew install scons python3"
    echo "        xcode-select --install"
    echo ""
    echo "      安装完依赖后，重新运行此脚本"
fi

# ---- 4. 完成 ----
echo ""
echo "[4/4] 环境搭建${DEPS_OK}!"
echo ""
echo "========================================"
echo " 后续步骤:"
echo ""
echo "   1. 编译 Godot:"
echo "      cd ${GODOT_DIR}"
echo "      scons platform=linuxbsd target=editor module_mcp_editor_enabled=yes -j\$(nproc)"
echo ""
echo "   2. 或者使用快捷脚本:"
echo "      cd ${SCRIPT_DIR}"
echo "      ./build.sh ../${GODOT_DIR}"
echo ""
echo "   3. 运行编译后的编辑器:"
echo "      ${GODOT_DIR}/bin/godot.linuxbsd.editor.x86_64"
echo "========================================"
