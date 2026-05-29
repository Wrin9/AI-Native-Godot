/**************************************************************************/
/*  editor_plugin.cpp - MCP Editor Plugin Implementation                  */
/**************************************************************************/

#include "editor_plugin.h"
#include "core/config/engine.h"
#include "core/config/project_settings.h"

MCPEditorPlugin *MCPEditorPlugin::_singleton = nullptr;

void MCPEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_http_server"), &MCPEditorPlugin::get_http_server);
}

void MCPEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (_singleton != nullptr) return;
			_singleton = this;

			_queue = memnew(MCPCommandQueue);
			_sandbox = memnew(MCPSandbox);
			_event_bus = memnew(MCPEventBus);
			_http_server = memnew(MCPHTTPServer);
			_snapshot = memnew(MCPSnapshot);

			_register_tools();

			_http_server->set_message_handler(callable_mp(this, &MCPEditorPlugin::_on_mcp_message));

			int port = 9877;
			if (ProjectSettings::get_singleton()->has_setting("mcp_editor/server_port")) {
				port = ProjectSettings::get_singleton()->get_setting("mcp_editor/server_port");
			}
			if (port <= 0 || port > 65535) port = 9877;
			_http_server->start(port);
			set_process(true);

			Engine::get_singleton()->add_singleton(Engine::Singleton("MCPEditor", this));
			print_line(vformat("MCPEditorPlugin: %d tools registered, listening on port %d", _tool_map.size(), port));
		} break;

		case NOTIFICATION_PROCESS: {
			if (!_http_server) return;
			_http_server->poll();
			if (_queue) _queue->process_queue();
			if (_event_bus) _event_bus->flush_events();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			set_process(false);
			if (_http_server) { _http_server->stop(); memdelete(_http_server); _http_server = nullptr; }
			if (_snapshot) { memdelete(_snapshot); _snapshot = nullptr; }
			if (_sandbox) { memdelete(_sandbox); _sandbox = nullptr; }
			if (_event_bus) { memdelete(_event_bus); _event_bus = nullptr; }
			if (_queue) { memdelete(_queue); _queue = nullptr; }
			_tool_map.clear();
			if (_singleton == this) _singleton = nullptr;
		} break;
	}
}

void MCPEditorPlugin::_register_tools() {
	_scene_tools.instantiate();
	_node_tools.instantiate();
	_script_tools.instantiate();
	_file_tools.instantiate();
	_play_tools.instantiate();
	_diagnostic_tools.instantiate();
	_ui_tools.instantiate();
	_animation_tools.instantiate();
	_material_tools.instantiate();
	_project_tools.instantiate();
	_physics_tools.instantiate();
	_shader_tools.instantiate();
	_tilemap_tools.instantiate();

	_scene_tools->set_editor_plugin(this);
	_node_tools->set_editor_plugin(this);
	_script_tools->set_editor_plugin(this);
	_file_tools->set_editor_plugin(this);
	_play_tools->set_editor_plugin(this);
	_diagnostic_tools->set_editor_plugin(this);
	_ui_tools->set_editor_plugin(this);
	_animation_tools->set_editor_plugin(this);
	_material_tools->set_editor_plugin(this);
	_project_tools->set_editor_plugin(this);
	_physics_tools->set_editor_plugin(this);
	_shader_tools->set_editor_plugin(this);
	_tilemap_tools->set_editor_plugin(this);

	_resource_tools.instantiate();
	_editor_tools.instantiate();
	_threed_tools.instantiate();
	_extra_tools.instantiate();

	_resource_tools->set_editor_plugin(this);
	_editor_tools->set_editor_plugin(this);
	_threed_tools->set_editor_plugin(this);
	_extra_tools->set_editor_plugin(this);

	_tool_map["get_scene_info"].object = _scene_tools;
	_tool_map["get_scene_info"].method = "get_scene_info";
	_tool_map["get_scene_info"].group = "scene";
	_tool_map["get_scene_info"].description = "Get current scene information";
	_tool_map["get_scene_tree"].object = _scene_tools;
	_tool_map["get_scene_tree"].method = "get_scene_tree";
	_tool_map["get_scene_tree"].group = "scene";
	_tool_map["get_scene_tree"].description = "Get the scene tree structure as JSON";
	_tool_map["list_scenes"].object = _scene_tools;
	_tool_map["list_scenes"].method = "list_scenes";
	_tool_map["list_scenes"].group = "scene";
	_tool_map["list_scenes"].description = "List all scene files";
	_tool_map["list_open_scenes"].object = _scene_tools;
	_tool_map["list_open_scenes"].method = "list_open_scenes";
	_tool_map["list_open_scenes"].group = "scene";
	_tool_map["list_open_scenes"].description = "List open scenes in editor";
	_tool_map["open_scene"].object = _scene_tools;
	_tool_map["open_scene"].method = "open_scene";
	_tool_map["open_scene"].group = "scene";
	_tool_map["open_scene"].description = "Open a scene file";
	_tool_map["create_new_scene"].object = _scene_tools;
	_tool_map["create_new_scene"].method = "create_new_scene";
	_tool_map["create_new_scene"].group = "scene";
	_tool_map["create_new_scene"].description = "Create a new empty scene";
	_tool_map["save_scene"].object = _scene_tools;
	_tool_map["save_scene"].method = "save_scene";
	_tool_map["save_scene"].group = "scene";
	_tool_map["save_scene"].description = "Save the current scene";
	_tool_map["save_scene_as"].object = _scene_tools;
	_tool_map["save_scene_as"].method = "save_scene_as";
	_tool_map["save_scene_as"].group = "scene";
	_tool_map["save_scene_as"].description = "Save scene to new path";
	_tool_map["instantiate_scene"].object = _scene_tools;
	_tool_map["instantiate_scene"].method = "instantiate_scene";
	_tool_map["instantiate_scene"].group = "scene";
	_tool_map["instantiate_scene"].description = "Instantiate a scene";
	_tool_map["create_packed_scene_from_node"].object = _scene_tools;
	_tool_map["create_packed_scene_from_node"].method = "create_packed_scene_from_node";
	_tool_map["create_packed_scene_from_node"].group = "scene";
	_tool_map["create_packed_scene_from_node"].description = "Create packed scene from node";
	_tool_map["get_packed_scene_info"].object = _scene_tools;
	_tool_map["get_packed_scene_info"].method = "get_packed_scene_info";
	_tool_map["get_packed_scene_info"].group = "scene";
	_tool_map["get_packed_scene_info"].description = "Get packed scene info";
	_tool_map["get_selection"].object = _scene_tools;
	_tool_map["get_selection"].method = "get_selection";
	_tool_map["get_selection"].group = "scene";
	_tool_map["get_selection"].description = "Get selected nodes";
	_tool_map["get_node_info"].object = _node_tools;
	_tool_map["get_node_info"].method = "get_node_info";
	_tool_map["get_node_info"].group = "node";
	_tool_map["get_node_info"].description = "Get node information";
	_tool_map["find_nodes"].object = _node_tools;
	_tool_map["find_nodes"].method = "find_nodes";
	_tool_map["find_nodes"].group = "node";
	_tool_map["find_nodes"].description = "Find nodes by pattern";
	_tool_map["select_node"].object = _node_tools;
	_tool_map["select_node"].method = "select_node";
	_tool_map["select_node"].group = "node";
	_tool_map["select_node"].description = "Select a node";
	_tool_map["create_node"].object = _node_tools;
	_tool_map["create_node"].method = "create_node";
	_tool_map["create_node"].group = "node";
	_tool_map["create_node"].description = "Create a new node";
	_tool_map["duplicate_node"].object = _node_tools;
	_tool_map["duplicate_node"].method = "duplicate_node";
	_tool_map["duplicate_node"].group = "node";
	_tool_map["duplicate_node"].description = "Duplicate a node";
	_tool_map["rename_node"].object = _node_tools;
	_tool_map["rename_node"].method = "rename_node";
	_tool_map["rename_node"].group = "node";
	_tool_map["rename_node"].description = "Rename a node";
	_tool_map["reparent_node"].object = _node_tools;
	_tool_map["reparent_node"].method = "reparent_node";
	_tool_map["reparent_node"].group = "node";
	_tool_map["reparent_node"].description = "Change node parent";
	_tool_map["remove_node"].object = _node_tools;
	_tool_map["remove_node"].method = "remove_node";
	_tool_map["remove_node"].group = "node";
	_tool_map["remove_node"].description = "Remove a node";
	_tool_map["set_node_property"].object = _node_tools;
	_tool_map["set_node_property"].method = "set_node_property";
	_tool_map["set_node_property"].group = "node";
	_tool_map["set_node_property"].description = "Set node property";
	_tool_map["set_node_properties"].object = _node_tools;
	_tool_map["set_node_properties"].method = "set_node_properties";
	_tool_map["set_node_properties"].group = "node";
	_tool_map["set_node_properties"].description = "Set multiple node properties";
	_tool_map["set_transform_2d"].object = _node_tools;
	_tool_map["set_transform_2d"].method = "set_transform_2d";
	_tool_map["set_transform_2d"].group = "node";
	_tool_map["set_transform_2d"].description = "Set 2D transform";
	_tool_map["set_transform_3d"].object = _node_tools;
	_tool_map["set_transform_3d"].method = "set_transform_3d";
	_tool_map["set_transform_3d"].group = "node";
	_tool_map["set_transform_3d"].description = "Set 3D transform";
	_tool_map["set_node_script"].object = _node_tools;
	_tool_map["set_node_script"].method = "set_node_script";
	_tool_map["set_node_script"].group = "node";
	_tool_map["set_node_script"].description = "Attach script to node";
	_tool_map["list_node_properties"].object = _node_tools;
	_tool_map["list_node_properties"].method = "list_node_properties";
	_tool_map["list_node_properties"].group = "node";
	_tool_map["list_node_properties"].description = "List node properties";
	_tool_map["list_node_signals"].object = _node_tools;
	_tool_map["list_node_signals"].method = "list_node_signals";
	_tool_map["list_node_signals"].group = "node";
	_tool_map["list_node_signals"].description = "List node signals";
	_tool_map["list_node_methods"].object = _node_tools;
	_tool_map["list_node_methods"].method = "list_node_methods";
	_tool_map["list_node_methods"].group = "node";
	_tool_map["list_node_methods"].description = "List node methods";
	_tool_map["create_script"].object = _script_tools;
	_tool_map["create_script"].method = "create_script";
	_tool_map["create_script"].group = "script";
	_tool_map["create_script"].description = "Create a script file";
	_tool_map["edit_script"].object = _script_tools;
	_tool_map["edit_script"].method = "edit_script";
	_tool_map["edit_script"].group = "script";
	_tool_map["edit_script"].description = "Open script in editor";
	_tool_map["patch_script"].object = _script_tools;
	_tool_map["patch_script"].method = "patch_script";
	_tool_map["patch_script"].group = "script";
	_tool_map["patch_script"].description = "Apply patches to script";
	_tool_map["list_scripts"].object = _script_tools;
	_tool_map["list_scripts"].method = "list_scripts";
	_tool_map["list_scripts"].group = "script";
	_tool_map["list_scripts"].description = "List script files";
	_tool_map["open_script"].object = _script_tools;
	_tool_map["open_script"].method = "open_script";
	_tool_map["open_script"].group = "script";
	_tool_map["open_script"].description = "Open script in editor";
	_tool_map["get_script_errors"].object = _script_tools;
	_tool_map["get_script_errors"].method = "get_script_errors";
	_tool_map["get_script_errors"].group = "script";
	_tool_map["get_script_errors"].description = "Get script errors";
	_tool_map["validate_script"].object = _script_tools;
	_tool_map["validate_script"].method = "validate_script";
	_tool_map["validate_script"].group = "script";
	_tool_map["validate_script"].description = "Validate a script";
	_tool_map["request_script_reload"].object = _script_tools;
	_tool_map["request_script_reload"].method = "request_script_reload";
	_tool_map["request_script_reload"].group = "script";
	_tool_map["request_script_reload"].description = "Reload a script";
	_tool_map["list_files"].object = _file_tools;
	_tool_map["list_files"].method = "list_files";
	_tool_map["list_files"].group = "file";
	_tool_map["list_files"].description = "List files in directory";
	_tool_map["search_files"].object = _file_tools;
	_tool_map["search_files"].method = "search_files";
	_tool_map["search_files"].group = "file";
	_tool_map["search_files"].description = "Search files by pattern";
	_tool_map["file_exists"].object = _file_tools;
	_tool_map["file_exists"].method = "file_exists";
	_tool_map["file_exists"].group = "file";
	_tool_map["file_exists"].description = "Check if file exists";
	_tool_map["read_file"].object = _file_tools;
	_tool_map["read_file"].method = "read_file";
	_tool_map["read_file"].group = "file";
	_tool_map["read_file"].description = "Read file contents";
	_tool_map["write_file"].object = _file_tools;
	_tool_map["write_file"].method = "write_file";
	_tool_map["write_file"].group = "file";
	_tool_map["write_file"].description = "Write to a file";
	_tool_map["delete_file"].object = _file_tools;
	_tool_map["delete_file"].method = "delete_file";
	_tool_map["delete_file"].group = "file";
	_tool_map["delete_file"].description = "Delete a file";
	_tool_map["move_file"].object = _file_tools;
	_tool_map["move_file"].method = "move_file";
	_tool_map["move_file"].group = "file";
	_tool_map["move_file"].description = "Move a file";
	_tool_map["copy_file"].object = _file_tools;
	_tool_map["copy_file"].method = "copy_file";
	_tool_map["copy_file"].group = "file";
	_tool_map["copy_file"].description = "Copy a file";
	_tool_map["find_usages"].object = _file_tools;
	_tool_map["find_usages"].method = "find_usages";
	_tool_map["find_usages"].group = "file";
	_tool_map["find_usages"].description = "Find resource usages";
	_tool_map["get_play_state"].object = _play_tools;
	_tool_map["get_play_state"].method = "get_play_state";
	_tool_map["get_play_state"].group = "play";
	_tool_map["get_play_state"].description = "Get play mode state";
	_tool_map["enter_play_mode"].object = _play_tools;
	_tool_map["enter_play_mode"].method = "enter_play_mode";
	_tool_map["enter_play_mode"].group = "play";
	_tool_map["enter_play_mode"].description = "Enter play mode";
	_tool_map["play_main_scene"].object = _play_tools;
	_tool_map["play_main_scene"].method = "play_main_scene";
	_tool_map["play_main_scene"].group = "play";
	_tool_map["play_main_scene"].description = "Play main scene";
	_tool_map["exit_play_mode"].object = _play_tools;
	_tool_map["exit_play_mode"].method = "exit_play_mode";
	_tool_map["exit_play_mode"].group = "play";
	_tool_map["exit_play_mode"].description = "Exit play mode";
	_tool_map["simulate_action"].object = _play_tools;
	_tool_map["simulate_action"].method = "simulate_action";
	_tool_map["simulate_action"].group = "play";
	_tool_map["simulate_action"].description = "Simulate input action";
	_tool_map["simulate_key_event"].object = _play_tools;
	_tool_map["simulate_key_event"].method = "simulate_key_event";
	_tool_map["simulate_key_event"].group = "play";
	_tool_map["simulate_key_event"].description = "Simulate key event";
	_tool_map["simulate_mouse_button"].object = _play_tools;
	_tool_map["simulate_mouse_button"].method = "simulate_mouse_button";
	_tool_map["simulate_mouse_button"].group = "play";
	_tool_map["simulate_mouse_button"].description = "Simulate mouse click";
	_tool_map["simulate_mouse_drag"].object = _play_tools;
	_tool_map["simulate_mouse_drag"].method = "simulate_mouse_drag";
	_tool_map["simulate_mouse_drag"].group = "play";
	_tool_map["simulate_mouse_drag"].description = "Simulate mouse drag";
	_tool_map["simulate_input_sequence"].object = _play_tools;
	_tool_map["simulate_input_sequence"].method = "simulate_input_sequence";
	_tool_map["simulate_input_sequence"].group = "play";
	_tool_map["simulate_input_sequence"].description = "Simulate input sequence";
	_tool_map["get_time_scale"].object = _play_tools;
	_tool_map["get_time_scale"].method = "get_time_scale";
	_tool_map["get_time_scale"].group = "play";
	_tool_map["get_time_scale"].description = "Get time scale";
	_tool_map["set_time_scale"].object = _play_tools;
	_tool_map["set_time_scale"].method = "set_time_scale";
	_tool_map["set_time_scale"].group = "play";
	_tool_map["set_time_scale"].description = "Set time scale";
	_tool_map["capture_editor_view"].object = _play_tools;
	_tool_map["capture_editor_view"].method = "capture_editor_view";
	_tool_map["capture_editor_view"].group = "play";
	_tool_map["capture_editor_view"].description = "Capture editor viewport";
	_tool_map["get_console_logs"].object = _diagnostic_tools;
	_tool_map["get_console_logs"].method = "get_console_logs";
	_tool_map["get_console_logs"].group = "diagnostic";
	_tool_map["get_console_logs"].description = "Get console output";
	_tool_map["get_performance_snapshot"].object = _diagnostic_tools;
	_tool_map["get_performance_snapshot"].method = "get_performance_snapshot";
	_tool_map["get_performance_snapshot"].group = "diagnostic";
	_tool_map["get_performance_snapshot"].description = "Get performance metrics";
	_tool_map["analyze_scene_complexity"].object = _diagnostic_tools;
	_tool_map["analyze_scene_complexity"].method = "analyze_scene_complexity";
	_tool_map["analyze_scene_complexity"].group = "diagnostic";
	_tool_map["analyze_scene_complexity"].description = "Analyze scene complexity";
	_tool_map["get_project_info"].object = _diagnostic_tools;
	_tool_map["get_project_info"].method = "get_project_info";
	_tool_map["get_project_info"].group = "diagnostic";
	_tool_map["get_project_info"].description = "Get project info";
	_tool_map["map_project"].object = _diagnostic_tools;
	_tool_map["map_project"].method = "map_project";
	_tool_map["map_project"].group = "diagnostic";
	_tool_map["map_project"].description = "Map project structure";
	_tool_map["create_ui_root"].object = _ui_tools;
	_tool_map["create_ui_root"].method = "create_ui_root";
	_tool_map["create_ui_root"].group = "ui";
	_tool_map["create_ui_root"].description = "Create root Control node";
	_tool_map["create_control"].object = _ui_tools;
	_tool_map["create_control"].method = "create_control";
	_tool_map["create_control"].group = "ui";
	_tool_map["create_control"].description = "Create Control node";
	_tool_map["create_label"].object = _ui_tools;
	_tool_map["create_label"].method = "create_label";
	_tool_map["create_label"].group = "ui";
	_tool_map["create_label"].description = "Create Label node";
	_tool_map["create_button"].object = _ui_tools;
	_tool_map["create_button"].method = "create_button";
	_tool_map["create_button"].group = "ui";
	_tool_map["create_button"].description = "Create Button node";
	_tool_map["create_panel"].object = _ui_tools;
	_tool_map["create_panel"].method = "create_panel";
	_tool_map["create_panel"].group = "ui";
	_tool_map["create_panel"].description = "Create Panel node";
	_tool_map["create_texture_rect"].object = _ui_tools;
	_tool_map["create_texture_rect"].method = "create_texture_rect";
	_tool_map["create_texture_rect"].group = "ui";
	_tool_map["create_texture_rect"].description = "Create TextureRect node";
	_tool_map["create_container"].object = _ui_tools;
	_tool_map["create_container"].method = "create_container";
	_tool_map["create_container"].group = "ui";
	_tool_map["create_container"].description = "Create container node";
	_tool_map["set_control_layout"].object = _ui_tools;
	_tool_map["set_control_layout"].method = "set_control_layout";
	_tool_map["set_control_layout"].group = "ui";
	_tool_map["set_control_layout"].description = "Set control layout";
	_tool_map["set_control_size_flags"].object = _ui_tools;
	_tool_map["set_control_size_flags"].method = "set_control_size_flags";
	_tool_map["set_control_size_flags"].group = "ui";
	_tool_map["set_control_size_flags"].description = "Set control size flags";
	_tool_map["set_control_text"].object = _ui_tools;
	_tool_map["set_control_text"].method = "set_control_text";
	_tool_map["set_control_text"].group = "ui";
	_tool_map["set_control_text"].description = "Set control text";
	_tool_map["set_control_theme_override"].object = _ui_tools;
	_tool_map["set_control_theme_override"].method = "set_control_theme_override";
	_tool_map["set_control_theme_override"].group = "ui";
	_tool_map["set_control_theme_override"].description = "Override theme";
	_tool_map["connect_node_signal"].object = _ui_tools;
	_tool_map["connect_node_signal"].method = "connect_node_signal";
	_tool_map["connect_node_signal"].group = "ui";
	_tool_map["connect_node_signal"].description = "Connect a signal";
	_tool_map["create_animation_player"].object = _animation_tools;
	_tool_map["create_animation_player"].method = "create_animation_player";
	_tool_map["create_animation_player"].group = "animation";
	_tool_map["create_animation_player"].description = "Create AnimationPlayer";
	_tool_map["create_animation_clip"].object = _animation_tools;
	_tool_map["create_animation_clip"].method = "create_animation_clip";
	_tool_map["create_animation_clip"].group = "animation";
	_tool_map["create_animation_clip"].description = "Create animation clip";
	_tool_map["add_animation_track"].object = _animation_tools;
	_tool_map["add_animation_track"].method = "add_animation_track";
	_tool_map["add_animation_track"].group = "animation";
	_tool_map["add_animation_track"].description = "Add animation track";
	_tool_map["list_animations"].object = _animation_tools;
	_tool_map["list_animations"].method = "list_animations";
	_tool_map["list_animations"].group = "animation";
	_tool_map["list_animations"].description = "List animations";
	_tool_map["play_animation"].object = _animation_tools;
	_tool_map["play_animation"].method = "play_animation";
	_tool_map["play_animation"].group = "animation";
	_tool_map["play_animation"].description = "Play animation";
	_tool_map["create_material"].object = _material_tools;
	_tool_map["create_material"].method = "create_material";
	_tool_map["create_material"].group = "material";
	_tool_map["create_material"].description = "Create material";
	_tool_map["assign_material"].object = _material_tools;
	_tool_map["assign_material"].method = "assign_material";
	_tool_map["assign_material"].group = "material";
	_tool_map["assign_material"].description = "Assign material";
	_tool_map["get_project_info"].object = _project_tools;
	_tool_map["get_project_info"].method = "get_project_info";
	_tool_map["get_project_info"].group = "project";
	_tool_map["get_project_info"].description = "Get project info";
	_tool_map["list_project_settings"].object = _project_tools;
	_tool_map["list_project_settings"].method = "list_project_settings";
	_tool_map["list_project_settings"].group = "project";
	_tool_map["list_project_settings"].description = "List project settings";
	_tool_map["get_project_setting"].object = _project_tools;
	_tool_map["get_project_setting"].method = "get_project_setting";
	_tool_map["get_project_setting"].group = "project";
	_tool_map["get_project_setting"].description = "Get project setting value";
	_tool_map["set_project_setting"].object = _project_tools;
	_tool_map["set_project_setting"].method = "set_project_setting";
	_tool_map["set_project_setting"].group = "project";
	_tool_map["set_project_setting"].description = "Set project setting";
	_tool_map["list_project_features"].object = _project_tools;
	_tool_map["list_project_features"].method = "list_project_features";
	_tool_map["list_project_features"].group = "project";
	_tool_map["list_project_features"].description = "List project features";
	_tool_map["list_addons"].object = _project_tools;
	_tool_map["list_addons"].method = "list_addons";
	_tool_map["list_addons"].group = "project";
	_tool_map["list_addons"].description = "List addons";
	_tool_map["set_addon_enabled"].object = _project_tools;
	_tool_map["set_addon_enabled"].method = "set_addon_enabled";
	_tool_map["set_addon_enabled"].group = "project";
	_tool_map["set_addon_enabled"].description = "Enable or disable addon";
	_tool_map["list_autoloads"].object = _project_tools;
	_tool_map["list_autoloads"].method = "list_autoloads";
	_tool_map["list_autoloads"].group = "project";
	_tool_map["list_autoloads"].description = "List autoloads";
	_tool_map["set_autoload"].object = _project_tools;
	_tool_map["set_autoload"].method = "set_autoload";
	_tool_map["set_autoload"].group = "project";
	_tool_map["set_autoload"].description = "Set autoload";
	_tool_map["remove_autoload"].object = _project_tools;
	_tool_map["remove_autoload"].method = "remove_autoload";
	_tool_map["remove_autoload"].group = "project";
	_tool_map["remove_autoload"].description = "Remove autoload";
	_tool_map["list_input_actions"].object = _project_tools;
	_tool_map["list_input_actions"].method = "list_input_actions";
	_tool_map["list_input_actions"].group = "project";
	_tool_map["list_input_actions"].description = "List input actions";
	_tool_map["get_input_action"].object = _project_tools;
	_tool_map["get_input_action"].method = "get_input_action";
	_tool_map["get_input_action"].group = "project";
	_tool_map["get_input_action"].description = "Get input action details";
	_tool_map["add_input_action"].object = _project_tools;
	_tool_map["add_input_action"].method = "add_input_action";
	_tool_map["add_input_action"].group = "project";
	_tool_map["add_input_action"].description = "Add input action";
	_tool_map["remove_input_action"].object = _project_tools;
	_tool_map["remove_input_action"].method = "remove_input_action";
	_tool_map["remove_input_action"].group = "project";
	_tool_map["remove_input_action"].description = "Remove input action";

	// -- TileMap 工具 --
	_tool_map["create_tile_map"].object = _tilemap_tools;
	_tool_map["create_tile_map"].method = "create_tile_map";
	_tool_map["create_tile_map"].group = "tilemap";
	_tool_map["create_tile_map"].description = "Create a TileMap node";
	_tool_map["set_cell"].object = _tilemap_tools;
	_tool_map["set_cell"].method = "set_cell";
	_tool_map["set_cell"].group = "tilemap";
	_tool_map["set_cell"].description = "Set a single tile cell";
	_tool_map["set_cells_terrain_connect"].object = _tilemap_tools;
	_tool_map["set_cells_terrain_connect"].method = "set_cells_terrain_connect";
	_tool_map["set_cells_terrain_connect"].group = "tilemap";
	_tool_map["set_cells_terrain_connect"].description = "Set cells with terrain connection";
	_tool_map["clear_layer"].object = _tilemap_tools;
	_tool_map["clear_layer"].method = "clear_layer";
	_tool_map["clear_layer"].group = "tilemap";
	_tool_map["clear_layer"].description = "Clear a TileMap layer";
	_tool_map["get_tile_map_data"].object = _tilemap_tools;
	_tool_map["get_tile_map_data"].method = "get_tile_map_data";
	_tool_map["get_tile_map_data"].group = "tilemap";
	_tool_map["get_tile_map_data"].description = "Get TileMap cell data";
	_tool_map["create_tile_set"].object = _tilemap_tools;
	_tool_map["create_tile_set"].method = "create_tile_set";
	_tool_map["create_tile_set"].group = "tilemap";
	_tool_map["create_tile_set"].description = "Create a TileSet resource";
	_tool_map["add_tile_atlas_source"].object = _tilemap_tools;
	_tool_map["add_tile_atlas_source"].method = "add_tile_atlas_source";
	_tool_map["add_tile_atlas_source"].group = "tilemap";
	_tool_map["add_tile_atlas_source"].description = "Add atlas source to TileSet";
	_tool_map["set_tile_set_collision"].object = _tilemap_tools;
	_tool_map["set_tile_set_collision"].method = "set_tile_set_collision";
	_tool_map["set_tile_set_collision"].group = "tilemap";
	_tool_map["set_tile_set_collision"].description = "Set TileSet collision";

	// -- 物理工具 --
	_tool_map["create_collision_shape"].object = _physics_tools;
	_tool_map["create_collision_shape"].method = "create_collision_shape";
	_tool_map["create_collision_shape"].group = "physics";
	_tool_map["create_collision_shape"].description = "Create collision shape node";
	_tool_map["set_collision_shape_data"].object = _physics_tools;
	_tool_map["set_collision_shape_data"].method = "set_collision_shape_data";
	_tool_map["set_collision_shape_data"].group = "physics";
	_tool_map["set_collision_shape_data"].description = "Set collision shape data";
	_tool_map["create_ray_cast"].object = _physics_tools;
	_tool_map["create_ray_cast"].method = "create_ray_cast";
	_tool_map["create_ray_cast"].group = "physics";
	_tool_map["create_ray_cast"].description = "Create RayCast2D node";
	_tool_map["create_navigation_region"].object = _physics_tools;
	_tool_map["create_navigation_region"].method = "create_navigation_region";
	_tool_map["create_navigation_region"].group = "physics";
	_tool_map["create_navigation_region"].description = "Create NavigationRegion2D node";
	_tool_map["set_physics_material"].object = _physics_tools;
	_tool_map["set_physics_material"].method = "set_physics_material";
	_tool_map["set_physics_material"].group = "physics";
	_tool_map["set_physics_material"].description = "Set physics material";

	// -- Shader 工具 --
	_tool_map["create_shader"].object = _shader_tools;
	_tool_map["create_shader"].method = "create_shader";
	_tool_map["create_shader"].group = "shader";
	_tool_map["create_shader"].description = "Create a Shader resource";
	_tool_map["set_shader_code"].object = _shader_tools;
	_tool_map["set_shader_code"].method = "set_shader_code";
	_tool_map["set_shader_code"].group = "shader";
	_tool_map["set_shader_code"].description = "Set shader code";
	_tool_map["set_shader_param"].object = _shader_tools;
	_tool_map["set_shader_param"].method = "set_shader_param";
	_tool_map["set_shader_param"].group = "shader";
	_tool_map["set_shader_param"].description = "Set shader parameter";

	// Resource tools
	_tool_map["import_resource"].object = _resource_tools;
	_tool_map["import_resource"].method = "import_resource";
	_tool_map["import_resource"].group = "resource";
	_tool_map["import_resource"].description = "Import/copy resource into project";

	_tool_map["get_resource_info"].object = _resource_tools;
	_tool_map["get_resource_info"].method = "get_resource_info";
	_tool_map["get_resource_info"].group = "resource";
	_tool_map["get_resource_info"].description = "Get resource file information";

	_tool_map["set_resource_property"].object = _resource_tools;
	_tool_map["set_resource_property"].method = "set_resource_property";
	_tool_map["set_resource_property"].group = "resource";
	_tool_map["set_resource_property"].description = "Set resource property";

	_tool_map["list_resources"].object = _resource_tools;
	_tool_map["list_resources"].method = "list_resources";
	_tool_map["list_resources"].group = "resource";
	_tool_map["list_resources"].description = "List resource files by type";

	// Editor tools
	_tool_map["undo"].object = _editor_tools;
	_tool_map["undo"].method = "undo";
	_tool_map["undo"].group = "editor";
	_tool_map["undo"].description = "Undo last action";

	_tool_map["redo"].object = _editor_tools;
	_tool_map["redo"].method = "redo";
	_tool_map["redo"].group = "editor";
	_tool_map["redo"].description = "Redo last action";

	_tool_map["run_code"].object = _editor_tools;
	_tool_map["run_code"].method = "run_code";
	_tool_map["run_code"].group = "editor";
	_tool_map["run_code"].description = "Run GDScript code snippet";

	_tool_map["get_editor_state"].object = _editor_tools;
	_tool_map["get_editor_state"].method = "get_editor_state";
	_tool_map["get_editor_state"].group = "editor";
	_tool_map["get_editor_state"].description = "Get current editor state";

	_tool_map["set_editor_setting"].object = _editor_tools;
	_tool_map["set_editor_setting"].method = "set_editor_setting";
	_tool_map["set_editor_setting"].group = "editor";
	_tool_map["set_editor_setting"].description = "Set editor setting";

	// 3D/Audio/Particle tools
	_tool_map["create_audio_player"].object = _threed_tools;
	_tool_map["create_audio_player"].method = "create_audio_player";
	_tool_map["create_audio_player"].group = "3d";
	_tool_map["create_audio_player"].description = "Create audio player node";

	_tool_map["create_particles"].object = _threed_tools;
	_tool_map["create_particles"].method = "create_particles";
	_tool_map["create_particles"].group = "3d";
	_tool_map["create_particles"].description = "Create particle system";

	_tool_map["create_mesh_instance"].object = _threed_tools;
	_tool_map["create_mesh_instance"].method = "create_mesh_instance";
	_tool_map["create_mesh_instance"].group = "3d";
	_tool_map["create_mesh_instance"].description = "Create 3D mesh instance";

	_tool_map["create_light"].object = _threed_tools;
	_tool_map["create_light"].method = "create_light";
	_tool_map["create_light"].group = "3d";
	_tool_map["create_light"].description = "Create light node";

	_tool_map["create_camera_3d"].object = _threed_tools;
	_tool_map["create_camera_3d"].method = "create_camera_3d";
	_tool_map["create_camera_3d"].group = "3d";
	_tool_map["create_camera_3d"].description = "Create 3D camera";

	_tool_map["set_environment"].object = _threed_tools;
	_tool_map["set_environment"].method = "set_environment";
	_tool_map["set_environment"].group = "3d";
	_tool_map["set_environment"].description = "Set environment (sky, fog, ambient)";

	// Extra tools
	_tool_map["create_viewport"].object = _extra_tools;
	_tool_map["create_viewport"].method = "create_viewport";
	_tool_map["create_viewport"].group = "extra";
	_tool_map["create_viewport"].description = "Create SubViewport node";

	_tool_map["disconnect_node_signal"].object = _extra_tools;
	_tool_map["disconnect_node_signal"].method = "disconnect_node_signal";
	_tool_map["disconnect_node_signal"].group = "extra";
	_tool_map["disconnect_node_signal"].description = "Disconnect a signal";

	_tool_map["get_node_connections"].object = _extra_tools;
	_tool_map["get_node_connections"].method = "get_node_connections";
	_tool_map["get_node_connections"].group = "extra";
	_tool_map["get_node_connections"].description = "Get node signal connections";

	_tool_map["add_node_to_group"].object = _extra_tools;
	_tool_map["add_node_to_group"].method = "add_node_to_group";
	_tool_map["add_node_to_group"].group = "extra";
	_tool_map["add_node_to_group"].description = "Add node to group";

	_tool_map["remove_node_from_group"].object = _extra_tools;
	_tool_map["remove_node_from_group"].method = "remove_node_from_group";
	_tool_map["remove_node_from_group"].group = "extra";
	_tool_map["remove_node_from_group"].description = "Remove node from group";

	_tool_map["list_groups"].object = _extra_tools;
	_tool_map["list_groups"].method = "list_groups";
	_tool_map["list_groups"].group = "extra";
	_tool_map["list_groups"].description = "List all node groups";

	_tool_map["export_project"].object = _extra_tools;
	_tool_map["export_project"].method = "export_project";
	_tool_map["export_project"].group = "extra";
	_tool_map["export_project"].description = "Export project";

	_tool_map["set_export_preset"].object = _extra_tools;
	_tool_map["set_export_preset"].method = "set_export_preset";
	_tool_map["set_export_preset"].group = "extra";
	_tool_map["set_export_preset"].description = "Configure export preset";

	// 注册所有工具的参数 schema
	_register_tool_schemas();
}

void MCPEditorPlugin::_register_tool_schemas() {
	// Auto-generated tool parameter schemas

	{  // add_animation_track
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "library name"; props["library_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "track type"; props["track_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "keys"; props["keys"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["add_animation_track"] = schema;
	}

	{  // add_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "deadzone"; props["deadzone"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of input events for the action"; props["events"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["add_input_action"] = schema;
	}

	{  // add_node_to_group
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "group"; props["group"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["add_node_to_group"] = schema;
	}

	{  // add_tile_atlas_source
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "texture path"; props["texture_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size x"; props["tile_size_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size y"; props["tile_size_y"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "separation x"; props["separation_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "separation y"; props["separation_y"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "margin x"; props["margin_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "margin y"; props["margin_y"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("tileset_path");
		req.push_back("texture_path");
		schema["required"] = req; }
		_tool_schemas["add_tile_atlas_source"] = schema;
	}

	{  // assign_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "material path"; props["material_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "surface index"; props["surface_index"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["assign_material"] = schema;
	}

	{  // capture_editor_view
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "view"; props["view"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "index"; props["index"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save to file"; props["save_to_file"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "save path"; props["save_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "return data uri"; props["return_data_uri"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["capture_editor_view"] = schema;
	}

	{  // clear_layer
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["clear_layer"] = schema;
	}

	{  // connect_node_signal
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "signal name"; props["signal_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "method name"; props["method_name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "flags"; props["flags"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["connect_node_signal"] = schema;
	}

	{  // copy_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from path"; props["from_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to path"; props["to_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["copy_file"] = schema;
	}

	{  // create_animation_clip
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "library name"; props["library_name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "length"; props["length"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "loop mode"; props["loop_mode"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "step"; props["step"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "set current"; props["set_current"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["create_animation_clip"] = schema;
	}

	{  // create_animation_player
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_animation_player"] = schema;
	}

	{  // create_audio_player
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "stream path"; props["stream_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "autoplay"; props["autoplay"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_audio_player"] = schema;
	}

	{  // create_camera_3d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "fov"; props["fov"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "near"; props["near"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "far"; props["far"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_camera_3d"] = schema;
	}

	{  // create_collision_shape
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "width"; props["width"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "height"; props["height"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "radius"; props["radius"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "from"; props["from"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to"; props["to"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "disabled"; props["disabled"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_collision_shape"] = schema;
	}

	{  // create_container
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "container type"; props["container_type"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("container_type");
		schema["required"] = req; }
		_tool_schemas["create_container"] = schema;
	}

	{  // create_control
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "control type"; props["control_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("control_type");
		schema["required"] = req; }
		_tool_schemas["create_control"] = schema;
	}

	{  // create_light
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "light type"; props["light_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "color"; props["color"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "energy"; props["energy"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_light"] = schema;
	}

	{  // create_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "material type"; props["material_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "properties"; props["properties"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_material"] = schema;
	}

	{  // create_mesh_instance
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mesh path"; props["mesh_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_mesh_instance"] = schema;
	}

	{  // create_navigation_region
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "vertices"; props["vertices"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "polygons"; props["polygons"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_navigation_region"] = schema;
	}

	{  // create_new_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "root type"; props["root_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "root name"; props["root_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "open after"; props["open_after"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_new_scene"] = schema;
	}

	{  // create_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node type"; props["node_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_type");
		schema["required"] = req; }
		_tool_schemas["create_node"] = schema;
	}

	{  // create_packed_scene_from_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select file"; props["select_file"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_packed_scene_from_node"] = schema;
	}

	{  // create_particles
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "particle type"; props["particle_type"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "amount"; props["amount"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_particles"] = schema;
	}

	{  // create_ray_cast
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target position"; props["target_position"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_ray_cast"] = schema;
	}

	{  // create_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "language"; props["language"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "class name"; props["class_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "namespace"; props["namespace"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "extends"; props["extends"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "body"; props["body"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "tool"; props["tool"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "partial"; props["partial"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include system"; props["include_system"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "open in editor"; props["open_in_editor"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_script"] = schema;
	}

	{  // create_shader
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shader type"; props["shader_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "code"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_shader"] = schema;
	}

	{  // create_tile_map
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layers"; props["layers"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_tile_map"] = schema;
	}

	{  // create_tile_set
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size x"; props["tile_size_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size y"; props["tile_size_y"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_tile_set"] = schema;
	}

	{  // create_ui_root
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "kind"; props["kind"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "control name"; props["control_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "layout preset"; props["layout_preset"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_ui_root"] = schema;
	}

	{  // create_viewport
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "size w"; props["size_w"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "size h"; props["size_h"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_viewport"] = schema;
	}

	{  // delete_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["delete_file"] = schema;
	}

	{  // disconnect_node_signal
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "signal name"; props["signal_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "method name"; props["method_name"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["disconnect_node_signal"] = schema;
	}

	{  // duplicate_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new name"; props["new_name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["duplicate_node"] = schema;
	}

	{  // edit_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "content"; props["content"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["edit_script"] = schema;
	}

	{  // enter_play_mode
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "scene path"; props["scene_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("scene_path");
		schema["required"] = req; }
		_tool_schemas["enter_play_mode"] = schema;
	}

	{  // export_project
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "preset name"; props["preset_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "export path"; props["export_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "debug"; props["debug"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["export_project"] = schema;
	}

	{  // file_exists
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["file_exists"] = schema;
	}

	{  // find_nodes
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name contains"; props["name_contains"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "class name"; props["class_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["find_nodes"] = schema;
	}

	{  // find_usages
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "symbol"; props["symbol"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "case sensitive"; props["case_sensitive"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("symbol");
		schema["required"] = req; }
		_tool_schemas["find_usages"] = schema;
	}

	{  // get_console_logs
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max lines"; props["max_lines"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include rotated"; props["include_rotated"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "filter"; props["filter"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "severity"; props["severity"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_console_logs"] = schema;
	}

	{  // get_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["get_input_action"] = schema;
	}

	{  // get_node_connections
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_node_connections"] = schema;
	}

	{  // get_node_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_node_info"] = schema;
	}

	{  // get_packed_scene_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max depth"; props["max_depth"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["get_packed_scene_info"] = schema;
	}

	{  // get_project_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("key");
		schema["required"] = req; }
		_tool_schemas["get_project_setting"] = schema;
	}

	{  // get_resource_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_resource_info"] = schema;
	}

	{  // get_script_errors
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max files"; props["max_files"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_script_errors"] = schema;
	}

	{  // get_tile_map_data
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_tile_map_data"] = schema;
	}

	{  // import_resource
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "dest path"; props["dest_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["import_resource"] = schema;
	}

	{  // instantiate_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "scene path"; props["scene_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("scene_path");
		schema["required"] = req; }
		_tool_schemas["instantiate_scene"] = schema;
	}

	{  // list_animations
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_animations"] = schema;
	}

	{  // list_files
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include hidden"; props["include_hidden"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_files"] = schema;
	}

	{  // list_node_methods
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include private"; props["include_private"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_methods"] = schema;
	}

	{  // list_node_properties
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include usage"; props["include_usage"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_properties"] = schema;
	}

	{  // list_node_signals
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_signals"] = schema;
	}

	{  // list_project_settings
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "prefix"; props["prefix"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include internal"; props["include_internal"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_project_settings"] = schema;
	}

	{  // list_resources
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "type filter"; props["type_filter"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_resources"] = schema;
	}

	{  // list_scenes
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_scenes"] = schema;
	}

	{  // list_scripts
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "language"; props["language"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_scripts"] = schema;
	}

	{  // map_project
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "format"; props["format"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include scripts"; props["include_scripts"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max files"; props["max_files"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["map_project"] = schema;
	}

	{  // move_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from path"; props["from_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to path"; props["to_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["move_file"] = schema;
	}

	{  // open_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "set inherited"; props["set_inherited"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["open_scene"] = schema;
	}

	{  // open_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "line"; props["line"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "column"; props["column"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["open_script"] = schema;
	}

	{  // patch_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "find"; props["find"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "replace"; props["replace"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "prepend"; props["prepend"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "append"; props["append"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["patch_script"] = schema;
	}

	{  // play_animation
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "custom blend"; props["custom_blend"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "custom speed"; props["custom_speed"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "from end"; props["from_end"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["play_animation"] = schema;
	}

	{  // read_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max chars"; props["max_chars"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["read_file"] = schema;
	}

	{  // remove_autoload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("name");
		schema["required"] = req; }
		_tool_schemas["remove_autoload"] = schema;
	}

	{  // remove_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["remove_input_action"] = schema;
	}

	{  // remove_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["remove_node"] = schema;
	}

	{  // remove_node_from_group
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "group"; props["group"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["remove_node_from_group"] = schema;
	}

	{  // rename_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new name"; props["new_name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["rename_node"] = schema;
	}

	{  // reparent_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new parent path"; props["new_parent_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "keep global transform"; props["keep_global_transform"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["reparent_node"] = schema;
	}

	{  // request_script_reload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["request_script_reload"] = schema;
	}

	{  // run_code
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "GDScript code to execute"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("code");
		schema["required"] = req; }
		_tool_schemas["run_code"] = schema;
	}

	{  // save_scene_as
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "with preview"; props["with_preview"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["save_scene_as"] = schema;
	}

	{  // search_files
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "pattern"; props["pattern"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("pattern");
		schema["required"] = req; }
		_tool_schemas["search_files"] = schema;
	}

	{  // select_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["select_node"] = schema;
	}

	{  // set_addon_enabled
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "addon"; props["addon"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "enabled"; props["enabled"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("addon");
		req.push_back("enabled");
		schema["required"] = req; }
		_tool_schemas["set_addon_enabled"] = schema;
	}

	{  // set_autoload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_autoload"] = schema;
	}

	{  // set_cell
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "coords"; props["coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "atlas coords"; props["atlas_coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "alternative tile"; props["alternative_tile"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_cell"] = schema;
	}

	{  // set_cells_terrain_connect
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "terrain set"; props["terrain_set"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "terrain"; props["terrain"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "cells"; props["cells"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_cells_terrain_connect"] = schema;
	}

	{  // set_collision_shape_data
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "width"; props["width"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "height"; props["height"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "radius"; props["radius"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_collision_shape_data"] = schema;
	}

	{  // set_control_layout
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_layout"] = schema;
	}

	{  // set_control_size_flags
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_size_flags"] = schema;
	}

	{  // set_control_text
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "text"; props["text"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_text"] = schema;
	}

	{  // set_control_theme_override
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "override type"; props["override_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "resource path"; props["resource_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_theme_override"] = schema;
	}

	{  // set_editor_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_editor_setting"] = schema;
	}

	{  // set_environment
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "sky color"; props["sky_color"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "ambient color"; props["ambient_color"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_environment"] = schema;
	}

	{  // set_export_preset
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "preset name"; props["preset_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "platform"; props["platform"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "export path"; props["export_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "settings"; props["settings"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_export_preset"] = schema;
	}

	{  // set_node_properties
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "properties"; props["properties"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_node_properties"] = schema;
	}

	{  // set_node_property
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_node_property"] = schema;
	}

	{  // set_node_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_node_script"] = schema;
	}

	{  // set_physics_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_physics_material"] = schema;
	}

	{  // set_project_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("key");
		req.push_back("value");
		schema["required"] = req; }
		_tool_schemas["set_project_setting"] = schema;
	}

	{  // set_resource_property
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_resource_property"] = schema;
	}

	{  // set_shader_code
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "code"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		req.push_back("code");
		schema["required"] = req; }
		_tool_schemas["set_shader_code"] = schema;
	}

	{  // set_shader_param
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shader path"; props["shader_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "param"; props["param"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "params"; props["params"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_shader_param"] = schema;
	}

	{  // set_tile_set_collision
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "atlas coords"; props["atlas_coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "physics layer"; props["physics_layer"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of collision points"; props["points"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "one way collision"; props["one_way_collision"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("tileset_path");
		schema["required"] = req; }
		_tool_schemas["set_tile_set_collision"] = schema;
	}

	{  // set_time_scale
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("value");
		schema["required"] = req; }
		_tool_schemas["set_time_scale"] = schema;
	}

	{  // set_transform_2d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_transform_2d"] = schema;
	}

	{  // set_transform_3d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_transform_3d"] = schema;
	}

	{  // simulate_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "strength"; props["strength"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["simulate_action"] = schema;
	}

	{  // simulate_input_sequence
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of input events"; props["events"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_input_sequence"] = schema;
	}

	{  // simulate_key_event
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "physical key"; props["physical_key"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("physical_key");
		schema["required"] = req; }
		_tool_schemas["simulate_key_event"] = schema;
	}

	{  // simulate_mouse_button
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "button"; props["button"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "position"; props["position"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_mouse_button"] = schema;
	}

	{  // simulate_mouse_drag
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from position"; props["from_position"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to position"; props["to_position"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "steps"; props["steps"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "button"; props["button"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_mouse_drag"] = schema;
	}

	{  // validate_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["validate_script"] = schema;
	}

	{  // write_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "content"; props["content"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["write_file"] = schema;
	}

}

void MCPEditorPlugin::_on_mcp_message(int p_client_id, const Dictionary &p_message) {
	String method = p_message.get("method", "");
	Variant id = p_message.get("id", Variant());
	Dictionary params = p_message.get("params", Dictionary());

	if (method == "tools/list") {
		Array tools = _build_tool_list();
		Dictionary result;
		result["tools"] = tools;
		_http_server->send_response(p_client_id, id, result);
	} else if (method == "tools/call") {
		String tool_name = params.get("name", "");
		Dictionary arguments = params.get("arguments", Dictionary());
		if (tool_name.is_empty()) {
			_http_server->send_error_response(p_client_id, id, -32602, "Missing tool name");
			return;
		}
		String result_str = _call_tool(tool_name, arguments);
		Dictionary content_item;
		content_item["type"] = "text";
		content_item["text"] = result_str;
		Array content;
		content.push_back(content_item);
		Dictionary result;
		result["content"] = content;
		_http_server->send_response(p_client_id, id, result);
	} else if (method == "resources/list" || method == "prompts/list") {
		Dictionary result;
		result["resources"] = Array();
		result["prompts"] = Array();
		_http_server->send_response(p_client_id, id, result);
	} else if (method == "ping") {
		Dictionary result;
		_http_server->send_response(p_client_id, id, result);
	} else {
		_http_server->send_error_response(p_client_id, id, -32601, vformat("Method not found: %s", method));
	}
}

Array MCPEditorPlugin::_build_tool_list() {
	Array tools;
	for (const KeyValue<String, ToolEntry> &E : _tool_map) {
		const ToolEntry &entry = E.value;
		Dictionary tool;
		tool["name"] = E.key;
		tool["description"] = entry.description;

		// Look up schema from separate map
		Dictionary input_schema;
		if (_tool_schemas.has(E.key)) {
			input_schema = _tool_schemas[E.key];
		} else {
			input_schema["type"] = "object";
			input_schema["properties"] = Dictionary();
			input_schema["required"] = Array();
		}
		tool["inputSchema"] = input_schema;

		tools.push_back(tool);
	}
	return tools;
}

String MCPEditorPlugin::_call_tool(const String &p_tool_name, const Dictionary &p_args) {
	if (!_tool_map.has(p_tool_name)) {
		return vformat("{\"error\": \"Unknown tool: %s\"}", p_tool_name);
	}
	const ToolEntry &entry = _tool_map[p_tool_name];
	if (entry.object.is_null()) {
		return vformat("{\"error\": \"Tool object is null: %s\"}", p_tool_name);
	}
	Variant result;
	result = entry.object->call(entry.method, p_args);
	if (result.get_type() == Variant::STRING) {
		return result;
	}
	return String(result);
}
