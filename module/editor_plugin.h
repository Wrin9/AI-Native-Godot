/**************************************************************************/
/*  editor_plugin.h                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPEditorPlugin — 编辑器插件主类，MCP 服务入口点和协调者。              */
/* 管理 HTTP 服务器、工具注册、消息路由。                                  */
/**************************************************************************/

#ifndef MCP_EDITOR_PLUGIN_H
#define MCP_EDITOR_PLUGIN_H

#include "editor/plugins/editor_plugin.h"

#include "core/mcp_command_queue.h"
#include "core/mcp_event_bus.h"
#include "core/mcp_http_server.h"
#include "core/mcp_sandbox.h"
#include "core/mcp_snapshot.h"
#include "tools/animation_tools.h"
#include "tools/diagnostic_tools.h"
#include "tools/file_tools.h"
#include "tools/material_tools.h"
#include "tools/node_tools.h"
#include "tools/physics_tools.h"
#include "tools/play_tools.h"
#include "tools/project_tools.h"
#include "tools/scene_tools.h"
#include "tools/script_tools.h"
#include "tools/shader_tools.h"
#include "tools/tilemap_tools.h"
#include "tools/resource_tools.h"
#include "tools/editor_tools.h"
#include "tools/threed_tools.h"
#include "tools/extra_tools.h"
#include "tools/ui_tools.h"

class MCPEditorPlugin : public EditorPlugin {
	GDCLASS(MCPEditorPlugin, EditorPlugin);

public:
	static MCPEditorPlugin *get_singleton() { return _singleton; }

	void _notification(int p_what);

	// 核心组件访问器
	MCPHTTPServer *get_http_server() const { return _http_server; }

	// EditorPlugin 接口
	bool has_main_screen() const override { return false; }
	String get_plugin_name() const override { return "MCP Editor"; }

protected:
	static void _bind_methods();

private:
	static MCPEditorPlugin *_singleton;

	// 核心组件
	MCPCommandQueue *_queue = nullptr;
	MCPSandbox *_sandbox = nullptr;
	MCPEventBus *_event_bus = nullptr;
	MCPHTTPServer *_http_server = nullptr;
	MCPSnapshot *_snapshot = nullptr;

	// 工具实例
	Ref<SceneTools> _scene_tools;
	Ref<NodeTools> _node_tools;
	Ref<ScriptTools> _script_tools;
	Ref<FileTools> _file_tools;
	Ref<PlayTools> _play_tools;
	Ref<DiagnosticTools> _diagnostic_tools;
	Ref<UITools> _ui_tools;
	Ref<AnimationTools> _animation_tools;
	Ref<MaterialTools> _material_tools;
	Ref<PhysicsTools> _physics_tools;
	Ref<ProjectTools> _project_tools;
	Ref<ShaderTools> _shader_tools;
	Ref<TileMapTools> _tilemap_tools;
	Ref<ResourceTools> _resource_tools;
	Ref<EditorTools> _editor_tools;
	Ref<ThreeDTools> _threed_tools;
	Ref<ExtraTools> _extra_tools;

	// 工具路由表：tool_name -> (tool_object, method_name)
	struct ToolEntry {
		Ref<RefCounted> object;
		String method;
		String group;
		String description;
		Dictionary input_schema; // JSON Schema for tool parameters
	};
	HashMap<String, ToolEntry> _tool_map;
	HashMap<String, Dictionary> _tool_schemas; // tool_name -> inputSchema

	/** 初始化所有工具并注册路由 */
	void _register_tools();

	/** 注册所有工具的参数 schema */
	void _register_tool_schemas();

	/** MCP 消息回调 — 处理 tools/list, tools/call 等 */
	void _on_mcp_message(int p_client_id, const Dictionary &p_message);

	/** 构建 tools/list 响应 */
	Array _build_tool_list();

	/** 调用指定工具 */
	String _call_tool(const String &p_tool_name, const Dictionary &p_args);
};

#endif // MCP_EDITOR_PLUGIN_H
