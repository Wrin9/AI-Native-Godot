/**************************************************************************/
/*  ui_tools.cpp                                                          */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* UITools 实现 — 编辑器 UI 操作。                                         */
/**************************************************************************/

#include "ui_tools.h"

void UITools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_editor_layout", "args"), &UITools::get_editor_layout);
	ClassDB::bind_method(D_METHOD("screenshot_editor", "args"), &UITools::screenshot_editor);
	ClassDB::bind_method(D_METHOD("get_dock_info", "args"), &UITools::get_dock_info);
	ClassDB::bind_method(D_METHOD("select_node_in_tree", "args"), &UITools::select_node_in_tree);
}

Array UITools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"ui/get_editor_layout",
			"获取当前编辑器布局信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_editor_layout")));

	defs.append(make_tool_def(
			"ui/screenshot",
			"截取编辑器窗口截图",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "region", Dictionary{ { "type", "string" }, { "description", "截图区域: full, viewport, inspector" } } } } },
			},
			Callable(this, "screenshot_editor")));

	defs.append(make_tool_def(
			"ui/get_dock_info",
			"获取停靠面板信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_dock_info")));

	defs.append(make_tool_def(
			"ui/select_node",
			"在场景树面板中选中指定节点",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "要选中的节点路径" } } } } },
			},
			Callable(this, "select_node_in_tree")));

	return defs;
}

String UITools::get_editor_layout(const Dictionary &p_args) {
	// TODO: 获取编辑器当前布局
	return "{\"layout\": {}}";
}

String UITools::screenshot_editor(const Dictionary &p_args) {
	// TODO: 实现编辑器截图
	return "{\"image_data\": \"\", \"width\": 0, \"height\": 0}";
}

String UITools::get_dock_info(const Dictionary &p_args) {
	// TODO: 获取面板布局
	return "{\"docks\": []}";
}

String UITools::select_node_in_tree(const Dictionary &p_args) {
	// TODO: 在场景树中选中节点
	return "{\"success\": true}";
}
