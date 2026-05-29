/**************************************************************************/
/*  script_tools.cpp                                                      */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ScriptTools 实现 — 脚本文件操作。                                       */
/**************************************************************************/

#include "script_tools.h"

void ScriptTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_script", "args"), &ScriptTools::read_script);
	ClassDB::bind_method(D_METHOD("write_script", "args"), &ScriptTools::write_script);
	ClassDB::bind_method(D_METHOD("list_methods", "args"), &ScriptTools::list_methods);
	ClassDB::bind_method(D_METHOD("list_signals", "args"), &ScriptTools::list_signals);
	ClassDB::bind_method(D_METHOD("attach_script", "args"), &ScriptTools::attach_script);
	ClassDB::bind_method(D_METHOD("detach_script", "args"), &ScriptTools::detach_script);
}

Array ScriptTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"script/read",
			"读取脚本文件内容",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "脚本文件路径" } } } } },
			},
			Callable(this, "read_script")));

	defs.append(make_tool_def(
			"script/write",
			"写入脚本文件内容",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "脚本文件路径" } } },
					{ "content", Dictionary{ { "type", "string" }, { "description", "脚本内容" } } } } },
			},
			Callable(this, "write_script")));

	defs.append(make_tool_def(
			"script/list_methods",
			"列出脚本中定义的方法",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "脚本文件路径" } } } } },
			},
			Callable(this, "list_methods")));

	defs.append(make_tool_def(
			"script/list_signals",
			"列出脚本中定义的信号",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "脚本文件路径" } } } } },
			},
			Callable(this, "list_signals")));

	defs.append(make_tool_def(
			"script/attach",
			"将脚本附加到节点",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "目标节点路径" } } },
					{ "script_path", Dictionary{ { "type", "string" }, { "description", "脚本文件路径" } } } } },
			},
			Callable(this, "attach_script")));

	defs.append(make_tool_def(
			"script/detach",
			"分离节点上的脚本",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "目标节点路径" } } } } },
			},
			Callable(this, "detach_script")));

	return defs;
}

String ScriptTools::read_script(const Dictionary &p_args) {
	String path = p_args.get("path", "");
	// TODO: 读取脚本文件内容
	return vformat("{\"path\": \"%s\", \"content\": \"\"}", path);
}

String ScriptTools::write_script(const Dictionary &p_args) {
	// TODO: 写入脚本文件
	return "{\"success\": true}";
}

String ScriptTools::list_methods(const Dictionary &p_args) {
	// TODO: 解析脚本，列出方法
	return "{\"methods\": []}";
}

String ScriptTools::list_signals(const Dictionary &p_args) {
	// TODO: 解析脚本，列出信号
	return "{\"signals\": []}";
}

String ScriptTools::attach_script(const Dictionary &p_args) {
	// TODO: 附加脚本到节点
	return "{\"success\": true}";
}

String ScriptTools::detach_script(const Dictionary &p_args) {
	// TODO: 分离脚本
	return "{\"success\": true}";
}
