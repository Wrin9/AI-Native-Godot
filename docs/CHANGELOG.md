# 变更日志

本项目遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [0.1.0] - 2025-05-28

### 新增

#### Godot C++ Module（MCP Editor Module）
- **MCPCommandQueue**：优先级命令队列，支持高/中/低三级优先级调度
- **MCPSandbox**：沙箱隔离执行环境，支持快照创建和自动回滚
- **MCPEventBus**：事件总线系统，发布/订阅模式支持实时状态通知
- **MCPWebSocketServer**：WebSocket 服务器，替代 HTTP 传输层实现双向通信
- **MCPSnapshot**：状态快照引擎，支持版本化增量同步
- **ToolDispatcher**：C++ 原生工具分派器，比 GDScript Callable 快 50-100 倍

#### AI Bridge（Node.js MCP stdio Server）
- MCP stdio 协议实现，兼容 JSON-RPC 2.0
- WebSocket 客户端，连接 Godot 编辑器模块
- 令牌桶限流器，防止请求过载
- 批处理合并器，短时间内的同类请求自动合并
- 状态管理器，缓存编辑器状态并支持增量同步
- 断线自动重连（指数退避策略）

#### MCP 工具（兼容 funplay-godot-mcp 70+ 工具）
- 代码执行：`execute_code`, `capture_editor_view`, `log_message`, `wait_msec`
- 帮助引导：`funplay_help`, `list_tool_catalog`, `get_capability_status`, `list_workflow_coverage`
- 项目地图：`map_project`, `find_usages`
- 诊断：`get_editor_protocol_status`, `get_script_errors`, `validate_script`, `request_script_reload`, `get_console_logs`, `get_performance_snapshot`, `analyze_scene_complexity`
- 项目：`get_project_info`, `list_project_features`, `list_project_settings`, `get_project_setting`, `set_project_setting`, `get_project_skills_status`, `generate_project_skills`
- 输入映射：`list_input_actions`, `get_input_action`, `add_input_action`, `remove_input_action`, `add_input_event_to_action`, `clear_input_events`
- 运行时：`list_autoloads`, `get_runtime_bridge_status`, `set_autoload`, `remove_autoload`, `install_runtime_bridge`, `remove_runtime_bridge`
- 撤销重做：`get_undo_redo_status`, `editor_undo`, `editor_redo`
- 场景：`get_scene_info`, `get_scene_tree`, `list_scenes`, `open_scene`, `save_scene`, `save_scene_as`, `create_new_scene`, `instantiate_scene`, `create_packed_scene_from_node`, `get_packed_scene_info`
- 节点：`get_node_info`, `get_selection`, `find_nodes`, `select_node`, `list_node_properties`, `list_node_signals`, `list_node_methods`, `create_node`, `duplicate_node`, `rename_node`, `reparent_node`, `remove_node`, `set_node_property`, `set_node_properties`, `set_transform_2d`, `set_transform_3d`, `set_node_script`
- 脚本：`create_script`, `list_scripts`, `open_script`, `edit_script`, `patch_script`, `get_dotnet_project_info`
- 播放：`get_play_state`, `enter_play_mode`, `play_main_scene`, `exit_play_mode`, `simulate_action`, `simulate_key_event`, `simulate_mouse_button`, `simulate_mouse_drag`, `simulate_input_sequence`, `get_time_scale`, `set_time_scale`
- 断言：`assert_node_exists`, `assert_node_property`, `assert_signal_connected`
- 动画：`create_animation_player`, `create_animation_clip`, `add_animation_track`, `list_animations`, `play_animation`
- 摄像机：`get_camera_info`, `set_camera_2d`, `set_camera_3d`
- 材质：`create_material`, `assign_material`
- UI：`create_ui_root`, `create_control`, `create_label`, `create_button`, `create_panel`, `create_texture_rect`, `create_container`, `set_control_layout`, `set_control_size_flags`, `set_control_text`, `set_control_theme_override`, `set_control_texture`, `connect_node_signal`
- 文件：`list_files`, `search_files`, `file_exists`, `read_file`, `write_file`, `select_file`, `delete_file`, `move_file`, `copy_file`
- 插件：`list_addons`, `set_addon_enabled`

#### 文档
- 项目主文档 `docs/README.md`
- 架构设计文档 `docs/ARCHITECTURE.md`
- 编译指南 `docs/BUILD.md`
- MCP 工具参考 `docs/MCP-TOOLS.md`
- 贡献指南 `docs/CONTRIBUTING.md`

#### Craft Agent 集成
- Source 配置 `craft-source/config.json`
- 使用指南 `craft-source/guide.md`

### 技术栈
- Godot 4.x（C++ Module）
- Node.js 18+ / TypeScript 5.4+（AI Bridge）
- WebSocket（ws 8.16+）
- MCP Protocol（2024-11-05 / 2025-03-26 / 2025-06-18 / 2025-11-25）
