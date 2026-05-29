/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* Godot 模块注册 — 将 MCP 编辑器插件注册到引擎。                         */
/**************************************************************************/

#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "editor/plugins/editor_plugin.h"
#include "editor_plugin.h"

void initialize_mcp_editor_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	// 注册为编辑器插件（Godot 自动实例化并添加到编辑器节点树）
	EditorPlugins::add_by_type<MCPEditorPlugin>();

	// 注册核心组件类（供 GDScript 使用）
	GDREGISTER_CLASS(MCPCommandQueue);
	GDREGISTER_CLASS(MCPSandbox);
	GDREGISTER_CLASS(MCPEventBus);
	GDREGISTER_CLASS(MCPHTTPServer);
	GDREGISTER_CLASS(MCPSnapshot);

	print_line("MCP Editor Module: 已注册");
}

void uninitialize_mcp_editor_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}
	Engine::get_singleton()->remove_singleton("MCPEditor");
}
