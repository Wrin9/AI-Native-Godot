# MCP Editor Module — Compilation Fix Plan

## Errors Summary & Fix Strategy

| # | Error | Fix |
|---|-------|-----|
| 1 | `mcp_websocket_server.cpp:14` — WebSocket headers don't exist | Rewrite as `MCPHTTPServer` using `TCPServer` + `StreamPeerTCP` + HTTP JSON-RPC |
| 2 | `mcp_sandbox.cpp:90` — `GDScript::_new()` wrong signature | Fix to `script->_new(args, 0, err)` with `Callable::CallError` |
| 3 | `mcp_command_queue.cpp` — BIND_ENUM_CONSTANT GetTypeInfo | Remove all `BIND_ENUM_CONSTANT`, change enum params to `int` |
| 4 | `mcp_snapshot.cpp:18` — Wrong include path | `editor/editor_file_system.h` → `editor/file_system/editor_file_system.h` |
| 5 | `scene_tools.cpp:14` — Wrong include path | Same fix as #4 |
| 6 | `mcp_event_bus.cpp:316` — `is_debugging` not in SceneTree | Replace with `editor->is_playing_scene()` via EditorInterface |
| 7 | `node_tools.cpp:15` — Wrong include for EditorSelection | `editor/editor_selection.h` → `editor/editor_data.h` |
| 8 | `editor_plugin.h:69` — MCSnapshot vs MCPSnapshot typo | Fix: `MCSnapshot` → `MCPSnapshot` |
| 9 | `editor_plugin.h` — Virtual override mismatch | Use `_notification(int p_what) override` with switch on `NOTIFICATION_*` |
| 10 | `register_types.cpp` — MODULE_REGISTRATION_CLASS doesn't exist | Remove the macro line |
| 11 | `Mutex *memnew(Mutex)` — Wrong Mutex API | Use `Mutex _mutex;` member variable + `MutexLock lock(_mutex)` |
| 12 | `ui_tools.cpp` — `editor/editor_selection.h` | → `editor/editor_data.h` |
| 13 | `animation_tools.cpp` — `editor/editor_selection.h` | → `editor/editor_data.h` |
| 14 | `file_tools.cpp` — `editor/editor_file_system.h` | → `editor/file_system/editor_file_system.h` |
| 15 | `diagnostic_tools.cpp` — `editor/editor_file_system.h` | → `editor/file_system/editor_file_system.h` |
| 16 | `material_tools.cpp` — `editor/editor_file_system.h` | → `editor/file_system/editor_file_system.h` |
| 17 | `script_tools.cpp` — `editor/editor_file_system.h` | → `editor/file_system/editor_file_system.h` |
| 18 | `mcp_websocket_server.h/cpp` — Complete rewrite | New `MCPHTTPServer` class using TCPServer |
| 19 | `editor_plugin.h/cpp` — Class rename | `MCPWebSocketServer` → `MCPHTTPServer` everywhere |
| 20 | `SCsub` — File rename | `mcp_websocket_server.cpp` → `mcp_http_server.cpp` |
| 21 | `config.py` — Doc class name | `MCPWebSocketServer` → `MCPHTTPServer` |
