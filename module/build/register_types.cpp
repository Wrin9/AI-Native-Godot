/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* Godot 模块注册实现 — 将核心类注册到 ClassDB，                          */
/* 并以单例形式暴露给脚本层。仅在编辑器级别初始化。                        */
/**************************************************************************/

#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "editor_plugin.h"

void initialize_mcp_editor_module(ModuleInitializationLevel p_level) {
	// 仅在编辑器初始化级别注册，导出模板不需要 MCP
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	// 注册编辑器插件类
	ClassDB::register_class<MCPEditorPlugin>();

	// 注册核心组件类
	ClassDB::register_class<MCPCommandQueue>();
	ClassDB::register_class<MCPSandbox>();
	ClassDB::register_class<MCPEventBus>();
	ClassDB::register_class<MCPWebSocketServer>();
	ClassDB::register_class<MCPSnapshot>();
	ClassDB::register_class<ToolDispatcher>();

	// 添加全局单例，脚本可通过 MCPEditor 访问
	Engine::get_singleton()->add_singleton(
			Engine::Singleton("MCPEditor", MCPEditorPlugin::get_singleton()));
}

void uninitialize_mcp_editor_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	// 移除全局单例
	Engine::get_singleton()->remove_singleton("MCPEditor");

	// 注意：类的反注册由引擎统一处理，无需手动操作
}

// Godot 4.x 模块注册宏
MODULE_REGISTRATION_CLASS(mcp_editor_module)
