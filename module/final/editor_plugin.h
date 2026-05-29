/**************************************************************************/
/*  editor_plugin.h                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPEditorPlugin — 编辑器插件主类，作为模块的入口点和协调者。            */
/* 管理所有核心组件的生命周期，帧驱动轮询，工具注册。                      */
/**************************************************************************/

#ifndef MCP_EDITOR_PLUGIN_H
#define MCP_EDITOR_PLUGIN_H

#include "editor/editor_plugin.h"

#include "core/mcp_command_queue.h"
#include "core/mcp_event_bus.h"
#include "core/mcp_sandbox.h"
#include "core/mcp_snapshot.h"
#include "core/mcp_websocket_server.h"
#include "tool_dispatcher.h"
#include "tools/animation_tools.h"
#include "tools/diagnostic_tools.h"
#include "tools/file_tools.h"
#include "tools/material_tools.h"
#include "tools/node_tools.h"
#include "tools/play_tools.h"
#include "tools/project_tools.h"
#include "tools/scene_tools.h"
#include "tools/script_tools.h"
#include "tools/ui_tools.h"

class MCPEditorPlugin : public EditorPlugin {
	GDCLASS(MCPEditorPlugin, EditorPlugin);

public:
	/** 获取全局单例 */
	static MCPEditorPlugin *get_singleton() { return _singleton; }

	/** 生命周期：进入场景树 */
	void _enter_tree() override;
	/** 生命周期：退出场景树 */
	void _exit_tree() override;
	/** 帧回调：驱动所有核心组件 */
	void _process(double p_delta) override;

	// ---- 核心组件访问器 ----
	MCPCommandQueue *get_command_queue() const { return _queue; }
	MCPEventBus *get_event_bus() const { return _event_bus; }
	MCPWebSocketServer *get_websocket_server() const { return _ws_server; }
	MCPSandbox *get_sandbox() const { return _sandbox; }
	MCPSnapshot *get_snapshot() const { return _snapshot; }
	ToolDispatcher *get_tool_dispatcher() const { return _dispatcher; }

	// ---- EditorPlugin 接口 ----
	bool has_main_screen() const override { return false; }
	String get_plugin_name() const override { return "MCP Editor"; }

protected:
	static void _bind_methods();

private:
	static MCPEditorPlugin *_singleton;

	// ---- 核心组件 ----
	MCPCommandQueue *_queue = nullptr;
	MCPSandbox *_sandbox = nullptr;
	MCPEventBus *_event_bus = nullptr;
	MCPWebSocketServer *_ws_server = nullptr;
	MCSnapshot *_snapshot = nullptr;
	ToolDispatcher *_dispatcher = nullptr;

	// ---- 工具实例 ----
	Ref<SceneTools> _scene_tools;
	Ref<NodeTools> _node_tools;
	Ref<ScriptTools> _script_tools;
	Ref<FileTools> _file_tools;
	Ref<PlayTools> _play_tools;
	Ref<DiagnosticTools> _diagnostic_tools;
	Ref<UITools> _ui_tools;
	Ref<AnimationTools> _animation_tools;
	Ref<MaterialTools> _material_tools;
	Ref<ProjectTools> _project_tools;

	/** 创建并注册所有工具 */
	void _register_tools();
};

#endif // MCP_EDITOR_PLUGIN_H
