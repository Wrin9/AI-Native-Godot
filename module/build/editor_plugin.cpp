/**************************************************************************/
/*  editor_plugin.cpp                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPEditorPlugin 实现 — 管理 MCP 系统的完整生命周期。                    */
/*                                                                        */
/* 生命周期：                                                             */
/*   _enter_tree → 创建组件 → 启动 WebSocket → 注册工具                  */
/*   _process    → 轮询网络 → 处理命令队列 → 刷新事件总线                 */
/*   _exit_tree  → 停止服务器 → 释放资源                                  */
/**************************************************************************/

#include "editor_plugin.h"

#include "core/config/engine.h"

// ---- 静态单例 ----
MCPEditorPlugin *MCPEditorPlugin::_singleton = nullptr;

void MCPEditorPlugin::_bind_methods() {
	// 核心组件访问器绑定到脚本层
	ClassDB::bind_method(D_METHOD("get_command_queue"), &MCPEditorPlugin::get_command_queue);
	ClassDB::bind_method(D_METHOD("get_event_bus"), &MCPEditorPlugin::get_event_bus);
	ClassDB::bind_method(D_METHOD("get_websocket_server"), &MCPEditorPlugin::get_websocket_server);
	ClassDB::bind_method(D_METHOD("get_sandbox"), &MCPEditorPlugin::get_sandbox);
	ClassDB::bind_method(D_METHOD("get_snapshot"), &MCPEditorPlugin::get_snapshot);
	ClassDB::bind_method(D_METHOD("get_tool_dispatcher"), &MCPEditorPlugin::get_tool_dispatcher);
}

void MCPEditorPlugin::_enter_tree() {
	// 防止重复初始化
	if (_singleton != nullptr) {
		WARN_PRINT("MCPEditorPlugin: 已存在单例实例，跳过重复初始化");
		return;
	}
	_singleton = this;

	// ---- 1. 创建核心组件 ----
	_queue = memnew(MCPCommandQueue);
	_sandbox = memnew(MCPSandbox);
	_event_bus = memnew(MCPEventBus);
	_ws_server = memnew(MCPWebSocketServer);
	_snapshot = memnew(MCPSnapshot);
	_dispatcher = memnew(ToolDispatcher);

	// ---- 2. 关联依赖 ----
	// 沙箱需要命令队列来执行指令
	_sandbox->set_command_queue(_queue);
	// 快照需要事件总线来发布变更通知
	_snapshot->set_event_bus(_event_bus);

	// ---- 3. 注册所有工具到分派器 ----
	_register_tools();

	// ---- 4. 启动 WebSocket 服务器 ----
	// 默认端口 9877，可通过项目设置覆盖
	int port = EDITOR_GET("mcp_editor/server_port").operator int();
	if (port <= 0 || port > 65535) {
		port = 9877;
	}
	_ws_server->start(port);

	// ---- 5. 启用帧回调 ----
	set_process(true);

	print_line("MCPEditorPlugin: 初始化完成，WebSocket 监听端口 ", port);
}

void MCPEditorPlugin::_process(double p_delta) {
	if (!_ws_server) {
		return;
	}

	// 按顺序驱动核心管线：
	//   1. 轮询 WebSocket 连接（接收新消息）
	//   2. 处理命令队列（执行已解析的指令）
	//   3. 刷新事件总线（发送变更通知给客户端）
	_ws_server->poll();
	_queue->process_queue();
	_event_bus->flush_events();
}

void MCPEditorPlugin::_exit_tree() {
	set_process(false);

	// ---- 停止 WebSocket 服务器 ----
	if (_ws_server) {
		_ws_server->stop();
		memdelete(_ws_server);
		_ws_server = nullptr;
	}

	// ---- 释放核心组件 ----
	if (_snapshot) {
		memdelete(_snapshot);
		_snapshot = nullptr;
	}
	if (_event_bus) {
		memdelete(_event_bus);
		_event_bus = nullptr;
	}
	if (_sandbox) {
		memdelete(_sandbox);
		_sandbox = nullptr;
	}
	if (_queue) {
		memdelete(_queue);
		_queue = nullptr;
	}
	if (_dispatcher) {
		memdelete(_dispatcher);
		_dispatcher = nullptr;
	}

	// ---- 清除工具引用（Ref 自动释放） ----
	_scene_tools.unref();
	_node_tools.unref();
	_script_tools.unref();
	_file_tools.unref();
	_play_tools.unref();
	_diagnostic_tools.unref();
	_ui_tools.unref();
	_animation_tools.unref();
	_material_tools.unref();
	_project_tools.unref();

	// ---- 清除单例指针 ----
	if (_singleton == this) {
		_singleton = nullptr;
	}

	print_line("MCPEditorPlugin: 已清理退出");
}

void MCPEditorPlugin::_register_tools() {
	// ---- 创建工具实例 ----
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

	// ---- 设置工具依赖 ----
	_scene_tools->set_command_queue(_queue);
	_scene_tools->set_event_bus(_event_bus);
	_scene_tools->set_snapshot(_snapshot);

	_node_tools->set_command_queue(_queue);
	_node_tools->set_event_bus(_event_bus);

	_script_tools->set_command_queue(_queue);
	_script_tools->set_event_bus(_event_bus);

	_file_tools->set_command_queue(_queue);
	_file_tools->set_event_bus(_event_bus);

	_play_tools->set_command_queue(_queue);
	_play_tools->set_event_bus(_event_bus);

	_diagnostic_tools->set_command_queue(_queue);
	_diagnostic_tools->set_event_bus(_event_bus);

	_ui_tools->set_command_queue(_queue);
	_ui_tools->set_event_bus(_event_bus);

	_animation_tools->set_command_queue(_queue);
	_animation_tools->set_event_bus(_event_bus);

	_material_tools->set_command_queue(_queue);
	_material_tools->set_event_bus(_event_bus);

	_project_tools->set_command_queue(_queue);
	_project_tools->set_event_bus(_event_bus);

	// ---- 注册到分派器 ----
	_dispatcher->register_tools(_scene_tools);
	_dispatcher->register_tools(_node_tools);
	_dispatcher->register_tools(_script_tools);
	_dispatcher->register_tools(_file_tools);
	_dispatcher->register_tools(_play_tools);
	_dispatcher->register_tools(_diagnostic_tools);
	_dispatcher->register_tools(_ui_tools);
	_dispatcher->register_tools(_animation_tools);
	_dispatcher->register_tools(_material_tools);
	_dispatcher->register_tools(_project_tools);

	print_line("MCPEditorPlugin: 已注册 ", _dispatcher->list_tools().size(), " 个工具");
}
