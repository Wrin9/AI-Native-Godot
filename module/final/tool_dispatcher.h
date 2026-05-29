/**************************************************************************/
/*  tool_dispatcher.h                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ToolDispatcher — 统一的工具分派器                                       */
/*                                                                        */
/* 将 MCP 协议中的 tool_name 映射到具体的 C++ 工具方法。                   */
/* 每个 MCPToolBase 子类在初始化时把自身的方法注册到分派器，               */
/* WebSocket 服务器收到 tool_call 后只需调用 dispatch() 即可。             */
/**************************************************************************/

#ifndef TOOL_DISPATCHER_H
#define TOOL_DISPATCHER_H

#include "core/object/ref_counted.h"
#include "core/variant/callable.h"
#include "core/variant/dictionary.h"

class MCPToolBase;

class ToolDispatcher : public RefCounted {
	GDCLASS(ToolDispatcher, RefCounted);

public:
	/** 注册一个工具分组的所有方法（MCPToolBase 子类） */
	void register_tools(const Ref<MCPToolBase> &p_tool_group);

	/** 分派工具调用：根据名称和参数执行对应方法 */
	String dispatch(const String &p_tool_name, const Dictionary &p_args);

	/** 列出所有已注册工具的名称 */
	Array list_tools() const;

	/** 检查工具是否存在 */
	bool has_tool(const String &p_name) const;

	/** 获取工具的 JSON Schema（用于 MCP protocol 的 tools/list） */
	Dictionary get_tool_schema(const String &p_name) const;

	/** 获取所有工具的 Schema 数组（用于 MCP protocol 的 tools/list） */
	Array get_all_tool_schemas() const;

protected:
	static void _bind_methods();

private:
	/** 工具条目 */
	struct ToolEntry {
		String name; // 工具名称，如 "scene/get_tree"
		String description; // 工具描述
		Dictionary input_schema; // JSON Schema 格式的输入定义
		Callable handler; // 实际执行函数
		Ref<MCPToolBase> owner; // 保持工具组的引用，防止被 GC
	};

	/** 工具名 → 条目映射 */
	HashMap<String, ToolEntry> _tools;
};

#endif // TOOL_DISPATCHER_H
