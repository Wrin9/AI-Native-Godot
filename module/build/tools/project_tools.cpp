/**************************************************************************/
/*  project_tools.cpp                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ProjectTools 实现 — 项目级管理操作。                                    */
/**************************************************************************/

#include "project_tools.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/version.h"

void ProjectTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_project_info", "args"), &ProjectTools::get_project_info);
	ClassDB::bind_method(D_METHOD("get_project_settings", "args"), &ProjectTools::get_project_settings);
	ClassDB::bind_method(D_METHOD("set_project_setting", "args"), &ProjectTools::set_project_setting);
	ClassDB::bind_method(D_METHOD("get_godot_version", "args"), &ProjectTools::get_godot_version);
	ClassDB::bind_method(D_METHOD("get_export_presets", "args"), &ProjectTools::get_export_presets);
}

Array ProjectTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"project/info",
			"获取项目基本信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_project_info")));

	defs.append(make_tool_def(
			"project/get_settings",
			"获取项目设置",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "filter", Dictionary{ { "type", "string" }, { "description", "设置名称前缀过滤" } } } } },
			},
			Callable(this, "get_project_settings")));

	defs.append(make_tool_def(
			"project/set_setting",
			"设置项目配置项",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "name", Dictionary{ { "type", "string" }, { "description", "设置名称" } } },
					{ "value", Dictionary{ { "description", "设置值" } } } } },
			},
			Callable(this, "set_project_setting")));

	defs.append(make_tool_def(
			"project/godot_version",
			"获取 Godot 引擎版本信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_godot_version")));

	defs.append(make_tool_def(
			"project/export_presets",
			"获取导出预设列表",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_export_presets")));

	return defs;
}

String ProjectTools::get_project_info(const Dictionary &p_args) {
	Dictionary info;
	info["name"] = ProjectSettings::get_singleton()->get_setting("application/config/name", "");
	info["project_path"] = ProjectSettings::get_singleton()->get_resource_path();
	return JSON::stringify(info);
}

String ProjectTools::get_project_settings(const Dictionary &p_args) {
	// TODO: 按前缀过滤并返回设置列表
	return "{\"settings\": {}}";
}

String ProjectTools::set_project_setting(const Dictionary &p_args) {
	String name = p_args.get("name", "");
	// TODO: 设置项目配置
	return "{\"success\": true}";
}

String ProjectTools::get_godot_version(const Dictionary &p_args) {
	Dictionary ver;
	ver["major"] = VERSION_MAJOR;
	ver["minor"] = VERSION_MINOR;
	ver["patch"] = VERSION_PATCH;
	ver["status"] = VERSION_STATUS;
	ver["build"] = VERSION_BUILD;
	ver["full_string"] = String(VERSION_FULL_BUILD);
	return JSON::stringify(ver);
}

String ProjectTools::get_export_presets(const Dictionary &p_args) {
	// TODO: 读取导出预设
	return "{\"presets\": []}";
}
