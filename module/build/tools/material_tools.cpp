/**************************************************************************/
/*  material_tools.cpp                                                    */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MaterialTools 实现 — 材质与着色器操作。                                 */
/**************************************************************************/

#include "material_tools.h"

void MaterialTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material", "args"), &MaterialTools::get_material);
	ClassDB::bind_method(D_METHOD("set_material_property", "args"), &MaterialTools::set_material_property);
	ClassDB::bind_method(D_METHOD("get_shader_code", "args"), &MaterialTools::get_shader_code);
	ClassDB::bind_method(D_METHOD("set_shader_code", "args"), &MaterialTools::set_shader_code);
	ClassDB::bind_method(D_METHOD("list_materials", "args"), &MaterialTools::list_materials);
}

Array MaterialTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"material/get",
			"获取节点材质信息",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "节点路径" } } },
					{ "slot", Dictionary{ { "type", "integer" }, { "description", "材质槽位索引" } } } } },
			},
			Callable(this, "get_material")));

	defs.append(make_tool_def(
			"material/set_property",
			"设置材质属性",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "resource_path", Dictionary{ { "type", "string" }, { "description", "材质资源路径" } } },
					{ "property", Dictionary{ { "type", "string" }, { "description", "属性名" } } },
					{ "value", Dictionary{ { "description", "属性值" } } } } },
			},
			Callable(this, "set_material_property")));

	defs.append(make_tool_def(
			"material/get_shader",
			"获取着色器代码",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "resource_path", Dictionary{ { "type", "string" }, { "description", "着色器资源路径" } } } } },
			},
			Callable(this, "get_shader_code")));

	defs.append(make_tool_def(
			"material/set_shader",
			"设置着色器代码",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "resource_path", Dictionary{ { "type", "string" }, { "description", "着色器资源路径" } } },
					{ "code", Dictionary{ { "type", "string" }, { "description", "着色器代码" } } } } },
			},
			Callable(this, "set_shader_code")));

	defs.append(make_tool_def(
			"material/list",
			"列出项目中的材质资源",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "list_materials")));

	return defs;
}

String MaterialTools::get_material(const Dictionary &p_args) {
	// TODO: 获取材质信息
	return "{\"material\": {}}";
}

String MaterialTools::set_material_property(const Dictionary &p_args) {
	// TODO: 设置材质属性
	return "{\"success\": true}";
}

String MaterialTools::get_shader_code(const Dictionary &p_args) {
	// TODO: 读取着色器代码
	return "{\"code\": \"\"}";
}

String MaterialTools::set_shader_code(const Dictionary &p_args) {
	// TODO: 写入着色器代码
	return "{\"success\": true}";
}

String MaterialTools::list_materials(const Dictionary &p_args) {
	// TODO: 列出材质资源
	return "{\"materials\": []}";
}
