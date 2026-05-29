/**************************************************************************/
/*  tool_dispatcher.cpp                                                   */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ToolDispatcher 实现 — 工具注册、查找、分派的核心逻辑。                  */
/**************************************************************************/

#include "tool_dispatcher.h"

#include "core/error/error_macros.h"
#include "core/variant/variant.h"

void ToolDispatcher::_bind_methods() {
	ClassDB::bind_method(D_METHOD("dispatch", "tool_name", "args"), &ToolDispatcher::dispatch);
	ClassDB::bind_method(D_METHOD("list_tools"), &ToolDispatcher::list_tools);
	ClassDB::bind_method(D_METHOD("has_tool", "name"), &ToolDispatcher::has_tool);
	ClassDB::bind_method(D_METHOD("get_tool_schema", "name"), &ToolDispatcher::get_tool_schema);
	ClassDB::bind_method(D_METHOD("get_all_tool_schemas"), &ToolDispatcher::get_all_tool_schemas);
}

void ToolDispatcher::register_tools(const Ref<MCPToolBase> &p_tool_group) {
	ERR_FAIL_COND(p_tool_group.is_null());

	// 从工具组获取所有工具定义并注册
	Array tool_defs = p_tool_group->get_tool_definitions();
	for (int i = 0; i < tool_defs.size(); i++) {
		Dictionary def = tool_defs[i];
		String name = def["name"];

		if (_tools.has(name)) {
			WARN_PRINT(vformat("ToolDispatcher: 工具 '%s' 已注册，将被覆盖", name));
		}

		ToolEntry entry;
		entry.name = name;
		entry.description = def.get("description", "");
		entry.input_schema = def.get("input_schema", Dictionary());
		entry.handler = def["handler"];
		entry.owner = p_tool_group;

		_tools.insert(name, entry);
	}
}

String ToolDispatcher::dispatch(const String &p_tool_name, const Dictionary &p_args) {
	// 查找工具
	HashMap<String, ToolEntry>::Iterator it = _tools.find(p_tool_name);
	if (!it) {
		return vformat("{\"error\": \"工具 '%s' 不存在\"}", p_tool_name);
	}

	const ToolEntry &entry = it->value;

	// 调用处理函数
	Variant result;
	Callable::CallError call_error;
	Variant args[] = { p_args };
	entry.handler.callp(args, 1, result, call_error);

	if (call_error.error != Callable::CallError::CALL_OK) {
		return vformat("{\"error\": \"工具 '%s' 调用失败: error=%d\"}",
				p_tool_name, (int)call_error.error);
	}

	// 返回结果（确保是字符串）
	if (result.get_type() == Variant::STRING) {
		return result.operator String();
	}

	// 非 string 结果序列化为 JSON
	return JSON::stringify(result);
}

Array ToolDispatcher::list_tools() const {
	Array names;
	for (const KeyValue<String, ToolEntry> &kv : _tools) {
		names.append(kv.key);
	}
	return names;
}

bool ToolDispatcher::has_tool(const String &p_name) const {
	return _tools.has(p_name);
}

Dictionary ToolDispatcher::get_tool_schema(const String &p_name) const {
	HashMap<String, ToolEntry>::ConstIterator it = _tools.find(p_name);
	if (!it) {
		return Dictionary();
	}

	const ToolEntry &entry = it->value;
	Dictionary schema;
	schema["name"] = entry.name;
	schema["description"] = entry.description;
	schema["inputSchema"] = entry.input_schema;
	return schema;
}

Array ToolDispatcher::get_all_tool_schemas() const {
	Array schemas;
	for (const KeyValue<String, ToolEntry> &kv : _tools) {
		Dictionary schema;
		schema["name"] = kv.value.name;
		schema["description"] = kv.value.description;
		schema["inputSchema"] = kv.value.input_schema;
		schemas.append(schema);
	}
	return schemas;
}
