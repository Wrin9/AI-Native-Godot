/**************************************************************************/
/*  diagnostic_tools.cpp                                                  */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* DiagnosticTools 实现 — 诊断与分析。                                     */
/**************************************************************************/

#include "diagnostic_tools.h"

void DiagnosticTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("check_errors", "args"), &DiagnosticTools::check_errors);
	ClassDB::bind_method(D_METHOD("validate_scene", "args"), &DiagnosticTools::validate_scene);
	ClassDB::bind_method(D_METHOD("get_performance_stats", "args"), &DiagnosticTools::get_performance_stats);
	ClassDB::bind_method(D_METHOD("get_debug_info", "args"), &DiagnosticTools::get_debug_info);
}

Array DiagnosticTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"diagnostic/check_errors",
			"检查当前场景/脚本的错误",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "check_errors")));

	defs.append(make_tool_def(
			"diagnostic/validate_scene",
			"验证场景完整性",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "场景文件路径（为空则检查当前场景）" } } } } },
			},
			Callable(this, "validate_scene")));

	defs.append(make_tool_def(
			"diagnostic/performance",
			"获取编辑器性能统计",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_performance_stats")));

	defs.append(make_tool_def(
			"diagnostic/debug_info",
			"获取当前调试信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_debug_info")));

	return defs;
}

String DiagnosticTools::check_errors(const Dictionary &p_args) {
	// TODO: 收集编辑器错误面板中的错误
	return "{\"errors\": [], \"warnings\": []}";
}

String DiagnosticTools::validate_scene(const Dictionary &p_args) {
	// TODO: 验证场景结构
	return "{\"valid\": true, \"issues\": []}";
}

String DiagnosticTools::get_performance_stats(const Dictionary &p_args) {
	// TODO: 采集性能数据
	return "{\"fps\": 0, \"memory\": 0, \"node_count\": 0}";
}

String DiagnosticTools::get_debug_info(const Dictionary &p_args) {
	// TODO: 获取调试信息
	return "{\"info\": {}}";
}
