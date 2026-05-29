@echo off
REM AI-Native Godot - 一键启动脚本
REM 自动启动 MCP 编辑器并等待 MCP HTTP 服务就绪

echo ========================================
echo   AI-Native Godot MCP Editor
echo   Godot 4.6.3 + MCP HTTP Server
echo ========================================
echo.

REM 默认项目路径（可通过参数覆盖）
set PROJECT=%~1
if "%PROJECT%"=="" set PROJECT=F:\game\test-mcp-project

REM 创建默认项目（如果不存在）
if not exist "%PROJECT%\project.godot" (
    echo Creating default project at %PROJECT%...
    mkdir "%PROJECT%" 2>nul
    echo [application] > "%PROJECT%\project.godot"
    echo config/name=MCP Test Project >> "%PROJECT%\project.godot"
    echo. >> "%PROJECT%\project.godot"
    echo [rendering] >> "%PROJECT%\project.godot"
    echo renderer/rendering_method=gl_compatibility >> "%PROJECT%\project.godot"
)

echo Starting AI-Native Godot...
echo   Project: %PROJECT%
echo   MCP Port: 9877
echo   MCP Endpoint: http://127.0.0.1:9877/
echo.

start "" "%~dp0godot-mcp-editor.exe" --path "%PROJECT%" -e

echo Waiting for MCP server...
:wait
timeout /t 1 /nobreak >nul 2>&1
powershell -Command "try { $c = New-Object System.Net.Sockets.TcpClient; $c.Connect('127.0.0.1', 9877); $c.Close(); exit 0 } catch { exit 1 }" >nul 2>&1
if errorlevel 1 goto wait

echo.
echo ========================================
echo   MCP Server is READY on port 9877!
echo ========================================
echo.
echo Test: curl -X POST http://127.0.0.1:9877/ ^
  -H "Content-Type: application/json" ^
  -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/list\",\"params\":{}}"
echo.
