# ====================================================================
#  AI-Native Godot — CMake 构建配置（供 IDE 支持）
#
#  此文件不用于实际编译（Godot 使用 SCons），
#  而是为 VS Code / CLion 等提供代码补全和索引支持。
#
#  用法:
#    cmake -DGODOT_SOURCE=F:/game/godot-source -B build
#    然后在 IDE 中打开此项目即可获得代码补全
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(godot-ai-native CXX)

# C++17 标准（与 Godot 引擎保持一致）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Godot 源码路径（需手动指定）
set(GODOT_SOURCE "" CACHE PATH "Path to Godot source code root")

# ---- 收集模块源文件 ----
file(GLOB_RECURSE MODULE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/module/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/module/*.h"
)

# ---- 创建静态库目标（仅用于 IDE 索引，不实际链接） ----
add_library(mcp_editor STATIC ${MODULE_SOURCES})

# ---- Include 路径 ----
# 提供所有 Godot 头文件路径，确保 IDE 能正确解析
target_include_directories(mcp_editor PRIVATE
    # 模块自身
    ${CMAKE_CURRENT_SOURCE_DIR}/module

    # Godot 核心头文件
    ${GODOT_SOURCE}
    ${GODOT_SOURCE}/core
    ${GODOT_SOURCE}/editor
    ${GODOT_SOURCE}/modules
    ${GODOT_SOURCE}/platform
    ${GODOT_SOURCE}/drivers

    # Godot 生成的头文件（编译后产生）
    ${GODOT_SOURCE}/.scons-cache

    # 平台特定头文件 (Windows)
    ${GODOT_SOURCE}/platform/windows

    # 第三方库
    ${GODOT_SOURCE}/thirdparty
    ${GODOT_SOURCE}/thirdparty/zlib
    ${GODOT_SOURCE}/thirdparty/libwebsockets
)

# ---- 预处理宏 ----
target_compile_definitions(mcp_editor PRIVATE
    TOOLS_ENABLED           # 编辑器构建
    DEBUG_ENABLED           # 调试模式
    WINDOWS_ENABLED         # Windows 平台
    MODULE_MCP_EDITOR_ENABLED  # 本模块已启用
)

# ---- 编译选项 ----
if(MSVC)
    # Windows/MSVC 选项
    target_compile_options(mcp_editor PRIVATE /W3 /utf-8)
else()
    # GCC/Clang 选项
    target_compile_options(mcp_editor PRIVATE -Wall -Wextra -fPIC)
endif()
