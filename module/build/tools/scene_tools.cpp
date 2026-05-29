/**************************************************************************/
/*  scene_tools.cpp                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* SceneTools 实现 — 场景管理工具。                                        */
/**************************************************************************/

#include "scene_tools.h"

void SceneTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_tree", "args"), &SceneTools::get_tree);
	ClassDB::bind_method(D_METHOD("load_scene", "args"), &SceneTools::load_scene);
	ClassDB::bind_method(D_METHOD("save_scene", "args"), &SceneTools::save_scene);
	ClassDB::bind_method(D_METHOD("get_scene_properties", "args"), &SceneTools::get_scene_properties);
}

Array SceneTools::get_tool_definitions() const {
	Array defs;

	// scene/get_tree — 获取场景树结构
	defs.append(make_tool_def(
			"scene/get_tree",
			"获取当前编辑器场景树的完整结构",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_tree")));

	// scene/load_scene — 加载场景
	Dictionary load_schema;
	load_schema["type"] = "object";
	load_schema["properties"] = Dictionary{
		{ "path", Dictionary{ { "type", "string" }, { "description", "场景文件路径（如 res://scenes/main.tscn）" } } }
	};
	load_schema["required"] = PackedStringArray{ "path" };
	defs.append(make_tool_def(
			"scene/load_scene",
			"在编辑器中加载指定场景",
			load_schema,
			Callable(this, "load_scene")));

	// scene/save_scene — 保存当前场景
	defs.append(make_tool_def(
			"scene/save_scene",
			"保存当前正在编辑的场景",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "save_scene")));

	// scene/get_scene_properties — 获取场景属性
	defs.append(make_tool_def(
			"scene/get_scene_properties",
			"获取当前场景的属性信息",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "get_scene_properties")));

	return defs;
}

String SceneTools::get_tree(const Dictionary &p_args) {
	// TODO: 通过 MCPSnapshot 采集场景树
	if (_snapshot) {
		Dictionary tree = _snapshot->capture_scene_tree();
		return JSON::stringify(tree);
	}
	return "{\"error\": \"快照服务不可用\"}";
}

String SceneTools::load_scene(const Dictionary &p_args) {
	String path = p_args.get("path", "");
	if (path.is_empty()) {
		return "{\"error\": \"缺少 path 参数\"}";
	}
	// TODO: 调用 EditorNode::get_singleton()->load_scene(path)
	return vformat("{\"success\": true, \"path\": \"%s\"}", path);
}

String SceneTools::save_scene(const Dictionary &p_args) {
	// TODO: 调用 EditorNode::get_singleton()->save_scene()
	return "{\"success\": true}";
}

String SceneTools::get_scene_properties(const Dictionary &p_args) {
	// TODO: 获取场景根节点属性
	return "{\"properties\": {}}";
}
