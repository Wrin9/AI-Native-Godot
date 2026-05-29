/**************************************************************************/
/*  mcp_tool_base.h                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPToolBase — 所有 MCP 工具组的基类                                     */
/*                                                                        */
/* 每个工具组（如 SceneTools、NodeTools）继承此类，                        */
/* 在构造时将自身的方法注册为工具定义。ToolDispatcher 通过                  */
/* get_tool_definitions() 获取所有定义并建立分派映射。                      */
/**************************************************************************/

#ifndef MCP_TOOL_BASE_H
#define MCP_TOOL_BASE_H

#include "core/object/ref_counted.h"
#include "core/variant/callable.h"
#include "core/variant/dictionary.h"

// 前向声明，避免循环 include
class MCPCommandQueue;
class MCPEventBus;
class MCPSnapshot;

class MCPToolBase : public RefCounted {
	GDCLASS(MCPToolBase, RefCounted);

public:
	/** 获取此工具组提供的所有工具定义 */
	virtual Array get_tool_definitions() const = 0;

	// ---- 依赖注入 ----
	void set_command_queue(MCPCommandQueue *p_queue) { _queue = p_queue; }
	void set_event_bus(MCPEventBus *p_bus) { _event_bus = p_bus; }
	void set_snapshot(MCPSnapshot *p_snapshot) { _snapshot = p_snapshot; }

protected:
	static void _bind_methods();

	/** 辅助方法：创建一个工具定义 Dictionary */
	Dictionary make_tool_def(const String &p_name,
			const String &p_description,
			const Dictionary &p_input_schema,
			const Callable &p_handler) const {
		Dictionary def;
		def["name"] = p_name;
		def["description"] = p_description;
		def["input_schema"] = p_input_schema;
		def["handler"] = p_handler;
		return def;
	}

	// ---- 核心组件指针（由 MCPEditorPlugin 注入） ----
	MCPCommandQueue *_queue = nullptr;
	MCPEventBus *_event_bus = nullptr;
	MCPSnapshot *_snapshot = nullptr;
};

#endif // MCP_TOOL_BASE_H
