@echo off
REM ====================================================================
REM  AI-Native Godot — Windows 构建脚本
REM
REM  用法: build.bat <godot-source-path> [scons-options...]
REM  示例: build.bat F:\game\godot-source
REM        build.bat F:\game\godot-source dev_build=yes
REM ====================================================================

echo ========================================
echo  Building AI-Native Godot (Windows)
echo ========================================

set GODOT_SOURCE=%1
if "%GODOT_SOURCE%"=="" (
    echo.
    echo 错误: 未指定 Godot 源码路径
    echo.
    echo 用法: build.bat ^<path-to-godot-source^> [scons-options...]
    echo 示例: build.bat F:\game\godot-source
    echo       build.bat F:\game\godot-source dev_build=yes
    exit /b 1
)

REM ---- 检查 Godot 源码目录是否存在 ----
if not exist "%GODOT_SOURCE%\SConstruct" (
    echo.
    echo 错误: '%GODOT_SOURCE%' 不是有效的 Godot 源码目录
    echo       找不到 SConstruct 文件
    exit /b 1
)

REM ---- 复制模块到 Godot 源码 ----
echo.
echo [1/3] 复制模块到 Godot 源码...
set MODULE_DIR=%GODOT_SOURCE%\modules\mcp_editor

REM 先删除旧版本（确保干净）
if exist "%MODULE_DIR%" (
    echo       清理旧的模块目录...
    rmdir /s /q "%MODULE_DIR%"
)

REM 复制所有文件
xcopy /E /I /Y /Q "%~dp0module" "%MODULE_DIR%\"

REM 创建 doc_classes 目录（Godot 文档系统需要）
if not exist "%MODULE_DIR%\doc_classes" (
    mkdir "%MODULE_DIR%\doc_classes"
)

echo       完成

REM ---- 收集额外 SCons 参数 ----
shift
set SCONS_EXTRA=
:collect_args
if "%~1"=="" goto done_args
set SCONS_EXTRA=%SCONS_EXTRA% %~1
shift
goto collect_args
:done_args

REM ---- 编译 ----
echo.
echo [2/3] 编译 Godot (启用 MCP Editor 模块)...
echo       目标平台: windows
echo       并行度:   %NUMBER_OF_PROCESSORS% 个 CPU 核心
echo       额外参数: %SCONS_EXTRA%

cd /D "%GODOT_SOURCE%"
scons platform=windows target=editor module_mcp_editor_enabled=yes -j%NUMBER_OF_PROCESSORS% %SCONS_EXTRA%

if %ERRORLEVEL% neq 0 (
    echo.
    echo ========================================
    echo  编译失败! 错误码: %ERRORLEVEL%
    echo ========================================
    exit /b %ERRORLEVEL%
)

REM ---- 完成 ----
echo.
echo [3/3] 编译成功!
echo       生成文件: %GODOT_SOURCE%\bin\godot.windows.editor.x86_64.exe
echo ========================================
echo  构建完成
echo ========================================
