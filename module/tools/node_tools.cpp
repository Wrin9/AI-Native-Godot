/**
 * node_tools.cpp - 节点操作工具实现
 *
 * 节点的创建、删除、属性设置、变换等操作工具。
 * 从 funplay_core_tools.gd 的节点相关方法迁移而来。
 */

#include "node_tools.h"

#include "editor/editor_node.h"
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/editor_selection.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/gui/control.h"
#include "scene/resources/script.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/object/undo_redo.h"
#include "core/config/project_settings.h"

// ============================================================
// 设置编辑器插件引用
// ============================================================
void NodeTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 获取节点详细信息
// ============================================================
String NodeTools::get_node_info(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Dictionary info = _build_node_info(node);
	return JSON::stringify(info, "\t");
}

// ============================================================
// 查找节点
// ============================================================
String NodeTools::find_nodes(const Dictionary &p_args) {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No scene is currently open in the editor."})";
	}

	String name_contains = String(p_args.get("name_contains", "")).to_lower();
	String class_name = String(p_args.get("class_name", "")).strip_edges();
	String script_path = _normalize_path(p_args.get("script_path", ""));
	int max_results = CLAMP(int(p_args.get("max_results", 100)), 1, 2000);

	Array results;
	_find_nodes_recursive(scene_root, name_contains, class_name, script_path, max_results, results);

	Dictionary result;
	result["count"] = results.size();
	result["results"] = results;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 选择节点
// ============================================================
String NodeTools::select_node(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	_select_node(node);

	return vformat(R"({"selected": "%s"})", node_path);
}

// ============================================================
// 创建节点
// ============================================================
String NodeTools::create_node(const Dictionary &p_args) {
	String node_type = String(p_args.get("node_type", "")).strip_edges();
	if (node_type.is_empty()) {
		return R"({"error": "'node_type' is required."})";
	}

	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No edited scene is open."})";
	}

	// 获取父节点
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = _resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	// 验证类型
	if (!ClassDB::class_exists(node_type)) {
		return vformat(R"({"error": "Unknown Godot class '%s'.'})", node_type);
	}

	// 实例化节点
	Object *obj = ClassDB::instantiate(node_type.utf8().get_data());
	if (!obj || !Object::cast_to<Node>(obj)) {
		if (obj) {
			memdelete(obj);
		}
		return vformat(R"({"error": "'%s' is not instantiable as a Node."})", node_type);
	}

	Node *node = Object::cast_to<Node>(obj);
	String name = _safe_name(String(p_args.get("name", node_type)).strip_edges(), node_type);
	node->set_name(name);

	// 添加到场景树
	parent->add_child(node);
	_assign_owner_recursive(node, scene_root);

	// 附加脚本（如果有）
	if (p_args.has("script_path")) {
		String script_path = _normalize_path(p_args["script_path"]);
		if (!script_path.is_empty()) {
			Ref<Resource> script_res = ResourceLoader::load(script_path);
			if (script_res.is_null()) {
				// 脚本加载失败，回退
				parent->remove_child(node);
				memdelete(node);
				return vformat(R"({"error": "Script not found or invalid: %s"})", script_path);
			}
			node->set_script(script_res);
		}
	}

	// 可选选中新节点
	if (bool(p_args.get("select_new_node", true))) {
		_select_node(node);
	}

	Dictionary result;
	result["created"] = _node_to_summary(node);
	result["parent_path"] = String(parent->get_path());
	result["note"] = "Scene modified. Call save_scene to persist it.";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 复制节点
// ============================================================
String NodeTools::duplicate_node(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Node *duplicate = node->duplicate();
	if (!duplicate) {
		return vformat(R"({"error": "Failed to duplicate node '%s'.'})", node_path);
	}

	Node *parent = node->get_parent();
	if (!parent) {
		memdelete(duplicate);
		return vformat(R"({"error": "Node '%s' has no parent."})", node_path);
	}

	parent->add_child(duplicate);

	// 设置新名称
	String new_name = String(p_args.get("new_name", "")).strip_edges();
	if (!new_name.is_empty()) {
		duplicate->set_name(new_name);
	}

	_assign_owner_recursive(duplicate, _get_edited_scene_root());

	if (bool(p_args.get("select_new_node", true))) {
		_select_node(duplicate);
	}

	Dictionary result;
	result["source"] = _node_to_summary(node);
	result["duplicate"] = _node_to_summary(duplicate);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 重命名节点
// ============================================================
String NodeTools::rename_node(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String new_name = String(p_args.get("new_name", "")).strip_edges();
	if (node_path.is_empty() || new_name.is_empty()) {
		return R"({"error": "'node_path' and 'new_name' are required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Dictionary changes;
	changes["name"] = new_name;
	_commit_undoable_properties(node, changes, "Rename Node", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["node"] = _node_to_summary(node);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 重新设置父节点
// ============================================================
String NodeTools::reparent_node(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String new_parent_path = String(p_args.get("new_parent_path", "")).strip_edges();
	if (node_path.is_empty() || new_parent_path.is_empty()) {
		return R"({"error": "'node_path' and 'new_parent_path' are required."})";
	}

	Node *node = _resolve_node_path(node_path);
	Node *new_parent = _resolve_node_path(new_parent_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}
	if (!new_parent) {
		return vformat(R"({"error": "New parent not found: %s"})", new_parent_path);
	}

	Node *scene_root = _get_edited_scene_root();
	if (node == scene_root) {
		return R"({"error": "Reparenting the edited scene root is not supported."})";
	}

	bool keep_global = p_args.get("keep_global_transform", false);

	// 保存全局变换（如需要）
	Dictionary stored_transform;
	if (keep_global) {
		Node2D *node_2d = Object::cast_to<Node2D>(node);
		Node3D *node_3d = Object::cast_to<Node3D>(node);
		Control *control = Object::cast_to<Control>(node);
		if (node_2d) {
			stored_transform["kind"] = "Node2D";
			// 存储全局位置/旋转/缩放
			stored_transform["global_position_x"] = node_2d->get_global_position().x;
			stored_transform["global_position_y"] = node_2d->get_global_position().y;
		} else if (node_3d) {
			stored_transform["kind"] = "Node3D";
			stored_transform["global_position_x"] = node_3d->get_global_position().x;
			stored_transform["global_position_y"] = node_3d->get_global_position().y;
			stored_transform["global_position_z"] = node_3d->get_global_position().z;
		} else if (control) {
			stored_transform["kind"] = "Control";
			stored_transform["global_position_x"] = control->get_global_position().x;
			stored_transform["global_position_y"] = control->get_global_position().y;
		}
	}

	// 移动节点
	Node *old_parent = node->get_parent();
	if (old_parent) {
		old_parent->remove_child(node);
	}
	new_parent->add_child(node);
	_assign_owner_recursive(node, scene_root);

	// 恢复全局变换
	if (keep_global && stored_transform.has("kind")) {
		String kind = stored_transform["kind"];
		if (kind == "Node2D") {
			Node2D *n2d = Object::cast_to<Node2D>(node);
			if (n2d) {
				n2d->set_global_position(Vector2(stored_transform["global_position_x"], stored_transform["global_position_y"]));
			}
		} else if (kind == "Node3D") {
			Node3D *n3d = Object::cast_to<Node3D>(node);
			if (n3d) {
				n3d->set_global_position(Vector3(stored_transform["global_position_x"], stored_transform["global_position_y"], stored_transform["global_position_z"]));
			}
		} else if (kind == "Control") {
			Control *ctrl = Object::cast_to<Control>(node);
			if (ctrl) {
				ctrl->set_global_position(Vector2(stored_transform["global_position_x"], stored_transform["global_position_y"]));
			}
		}
	}

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["new_parent_path"] = String(new_parent->get_path());
	return JSON::stringify(result, "\t");
}

// ============================================================
// 删除节点
// ============================================================
String NodeTools::remove_node(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Node *scene_root = _get_edited_scene_root();
	if (node == scene_root) {
		return R"({"error": "Removing the edited scene root is not supported."})";
	}

	Node *parent = node->get_parent();
	if (parent) {
		parent->remove_child(node);
	}
	memdelete(node);

	return vformat(R"({"removed": "%s"})", node_path);
}

// ============================================================
// 设置节点单个属性
// ============================================================
String NodeTools::set_node_property(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String property_name = String(p_args.get("property", "")).strip_edges();
	if (node_path.is_empty() || property_name.is_empty()) {
		return R"({"error": "'node_path' and 'property' are required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Dictionary changes;
	changes[property_name] = p_args.get("value");
	_commit_undoable_properties(node, changes, "Set Node Property", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["property"] = property_name;
	result["value"] = p_args.get("value");
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置节点多个属性
// ============================================================
String NodeTools::set_node_properties(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Variant props_var = p_args.get("properties");
	if (props_var.get_type() != Variant::DICTIONARY) {
		return R"({"error": "'properties' must be an object."})";
	}

	Dictionary properties = props_var;

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	// 合并所有属性变更
	Dictionary changes;
	Array keys = properties.keys();
	for (int i = 0; i < keys.size(); i++) {
		changes[keys[i]] = properties[keys[i]];
	}
	_commit_undoable_properties(node, changes, "Set Node Properties", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["properties"] = properties;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置2D变换
// ============================================================
String NodeTools::set_transform_2d(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Dictionary changes;

	Node2D *node_2d = Object::cast_to<Node2D>(node);
	Control *control = Object::cast_to<Control>(node);

	if (node_2d) {
		if (p_args.has("position")) {
			changes["position"] = _to_vector2(p_args["position"]);
		}
		if (p_args.has("rotation_degrees")) {
			changes["rotation_degrees"] = double(p_args["rotation_degrees"]);
		}
		if (p_args.has("scale")) {
			changes["scale"] = _to_vector2(p_args["scale"]);
		}
		_commit_undoable_properties(node_2d, changes, "Set 2D Transform", bool(p_args.get("undoable", true)));
	} else if (control) {
		if (p_args.has("position")) {
			changes["position"] = _to_vector2(p_args["position"]);
		}
		if (p_args.has("rotation_degrees")) {
			changes["rotation_degrees"] = double(p_args["rotation_degrees"]);
		}
		if (p_args.has("scale")) {
			changes["scale"] = _to_vector2(p_args["scale"]);
		}
		if (p_args.has("size")) {
			changes["size"] = _to_vector2(p_args["size"]);
		}
		_commit_undoable_properties(control, changes, "Set Control Transform", bool(p_args.get("undoable", true)));
	} else {
		return vformat(R"({"error": "Node '%s' is not a Node2D or Control."})", node_path);
	}

	Dictionary result;
	result["node"] = _build_node_info(node);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置3D变换
// ============================================================
String NodeTools::set_transform_3d(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Node3D *node_3d = Object::cast_to<Node3D>(node);
	if (!node_3d) {
		return vformat(R"({"error": "Node '%s' is not a Node3D."})", node_path);
	}

	Dictionary changes;
	if (p_args.has("position")) {
		changes["position"] = _to_vector3(p_args["position"]);
	}
	if (p_args.has("rotation_degrees")) {
		changes["rotation_degrees"] = _to_vector3(p_args["rotation_degrees"]);
	}
	if (p_args.has("scale")) {
		changes["scale"] = _to_vector3(p_args["scale"]);
	}
	_commit_undoable_properties(node_3d, changes, "Set 3D Transform", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["node"] = _build_node_info(node);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置节点脚本
// ============================================================
String NodeTools::set_node_script(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String script_path = _normalize_path(p_args.get("script_path", ""));
	if (node_path.is_empty() || script_path.is_empty()) {
		return R"({"error": "'node_path' and 'script_path' are required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Ref<Resource> script_res = ResourceLoader::load(script_path);
	if (script_res.is_null()) {
		return vformat(R"({"error": "Script not found or invalid: %s"})", script_path);
	}

	node->set_script(script_res);

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["script_path"] = script_path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出节点属性
// ============================================================
String NodeTools::list_node_properties(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	bool include_usage = p_args.get("include_usage", false);
	Array properties;

	TypedArray<Dictionary> prop_list = node->get_property_list();
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		Dictionary item;
		item["name"] = prop_info.get("name", "");
		item["type"] = prop_info.get("type", 0);
		item["class_name"] = prop_info.get("class_name", "");
		item["hint"] = prop_info.get("hint", 0);
		item["hint_string"] = prop_info.get("hint_string", "");
		if (include_usage) {
			item["usage"] = prop_info.get("usage", 0);
		}
		properties.push_back(item);
	}

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["count"] = properties.size();
	result["properties"] = properties;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出节点信号
// ============================================================
String NodeTools::list_node_signals(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	Array signals_list;
	TypedArray<Dictionary> sig_list = node->get_signal_list();
	for (int i = 0; i < sig_list.size(); i++) {
		Dictionary sig_info = sig_list[i];
		Dictionary item;
		item["name"] = sig_info.get("name", "");
		item["args"] = sig_info.get("args", Array());
		signals_list.push_back(item);
	}

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["count"] = signals_list.size();
	result["signals"] = signals_list;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出节点方法
// ============================================================
String NodeTools::list_node_methods(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"({"error": "'node_path' is required."})";
	}

	Node *node = _resolve_node_path(node_path);
	if (!node) {
		return vformat(R"({"error": "Node not found: %s"})", node_path);
	}

	bool include_private = p_args.get("include_private", false);
	Array methods_list;

	TypedArray<Dictionary> meth_list = node->get_method_list();
	for (int i = 0; i < meth_list.size(); i++) {
		Dictionary meth_info = meth_list[i];
		String method_name = meth_info.get("name", "");

		// 跳过私有方法（如果未指定包含）
		if (!include_private && method_name.begins_with("_")) {
			continue;
		}

		Dictionary item;
		item["name"] = method_name;
		item["args"] = meth_info.get("args", Array());
		item["return"] = meth_info.get("return", Dictionary());
		item["flags"] = meth_info.get("flags", 0);
		methods_list.push_back(item);
	}

	Dictionary result;
	result["node"] = _node_to_summary(node);
	result["count"] = methods_list.size();
	result["methods"] = methods_list;
	return JSON::stringify(result, "\t");
}

// ============================================================
// _bind_methods - 注册公开方法
// ============================================================
void NodeTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &NodeTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("get_node_info", "args"), &NodeTools::get_node_info);
	ClassDB::bind_method(D_METHOD("find_nodes", "args"), &NodeTools::find_nodes, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("select_node", "args"), &NodeTools::select_node);
	ClassDB::bind_method(D_METHOD("create_node", "args"), &NodeTools::create_node);
	ClassDB::bind_method(D_METHOD("duplicate_node", "args"), &NodeTools::duplicate_node);
	ClassDB::bind_method(D_METHOD("rename_node", "args"), &NodeTools::rename_node);
	ClassDB::bind_method(D_METHOD("reparent_node", "args"), &NodeTools::reparent_node);
	ClassDB::bind_method(D_METHOD("remove_node", "args"), &NodeTools::remove_node);
	ClassDB::bind_method(D_METHOD("set_node_property", "args"), &NodeTools::set_node_property);
	ClassDB::bind_method(D_METHOD("set_node_properties", "args"), &NodeTools::set_node_properties);
	ClassDB::bind_method(D_METHOD("set_transform_2d", "args"), &NodeTools::set_transform_2d);
	ClassDB::bind_method(D_METHOD("set_transform_3d", "args"), &NodeTools::set_transform_3d);
	ClassDB::bind_method(D_METHOD("set_node_script", "args"), &NodeTools::set_node_script);
	ClassDB::bind_method(D_METHOD("list_node_properties", "args"), &NodeTools::list_node_properties);
	ClassDB::bind_method(D_METHOD("list_node_signals", "args"), &NodeTools::list_node_signals);
	ClassDB::bind_method(D_METHOD("list_node_methods", "args"), &NodeTools::list_node_methods);
}

// ============================================================
// 内部辅助方法
// ============================================================

Node *NodeTools::_get_edited_scene_root() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return nullptr;
	}
	return editor->get_edited_scene_root();
}

Node *NodeTools::_resolve_node_path(const String &p_path) const {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return nullptr;
	}

	String identifier = p_path.strip_edges();
	if (identifier.is_empty() || identifier == ".") {
		return scene_root;
	}

	// 按实例 ID 查找
	String id_text = identifier.begins_with("id:") ? identifier.substr(3) : identifier;
	if (id_text.is_valid_int()) {
		ObjectID id = (ObjectID)id_text.to_int();
		Object *obj = ObjectDB::get_instance(id);
		if (obj && Object::cast_to<Node>(obj)) {
			return Object::cast_to<Node>(obj);
		}
	}

	// 按路径查找
	if (String(scene_root->get_path()) == identifier) {
		return scene_root;
	}
	if (identifier.begins_with("/")) {
		SceneTree *tree = scene_root->get_tree();
		if (tree && tree->get_root()) {
			return tree->get_root()->get_node_or_null(NodePath(identifier));
		}
	}
	return scene_root->get_node_or_null(NodePath(identifier));
}

String NodeTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

void NodeTools::_assign_owner_recursive(Node *p_node, Node *p_owner) const {
	p_node->set_owner(p_owner);
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_assign_owner_recursive(child, p_owner);
		}
	}
}

Dictionary NodeTools::_node_to_summary(Node *p_node) const {
	Dictionary summary;
	if (!p_node) {
		return summary;
	}
	summary["id"] = itos(p_node->get_instance_id());
	summary["instance_id"] = p_node->get_instance_id();
	summary["name"] = p_node->get_name();
	summary["type"] = p_node->get_class();
	summary["path"] = String(p_node->get_path());
	summary["scene_file_path"] = p_node->get_scene_file_path();
	return summary;
}

Dictionary NodeTools::_build_node_info(Node *p_node) const {
	Dictionary info = _node_to_summary(p_node);
	if (!p_node) {
		return info;
	}

	info["child_count"] = p_node->get_child_count();

	// 分组信息
	Array groups;
	HashSet<StringName> group_set = p_node->get_groups();
	for (const StringName &g : group_set) {
		groups.push_back(String(g));
	}
	info["groups"] = groups;

	// 脚本
	Ref<Script> script = p_node->get_script();
	info["script"] = script.is_valid() ? script->get_resource_path() : "";

	// Node2D 特定信息
	Node2D *node_2d = Object::cast_to<Node2D>(p_node);
	if (node_2d) {
		Dictionary pos;
		pos["x"] = node_2d->get_position().x;
		pos["y"] = node_2d->get_position().y;
		info["position"] = pos;
		info["rotation_degrees"] = node_2d->get_rotation_degrees();
		Dictionary scale;
		scale["x"] = node_2d->get_scale().x;
		scale["y"] = node_2d->get_scale().y;
		info["scale"] = scale;
	}

	// Control 特定信息
	Control *control = Object::cast_to<Control>(p_node);
	if (control) {
		Dictionary pos;
		pos["x"] = control->get_position().x;
		pos["y"] = control->get_position().y;
		info["position"] = pos;
		Dictionary size;
		size["x"] = control->get_size().x;
		size["y"] = control->get_size().y;
		info["size"] = size;
		info["rotation_degrees"] = control->get_rotation_degrees();
		Dictionary scale;
		scale["x"] = control->get_scale().x;
		scale["y"] = control->get_scale().y;
		info["scale"] = scale;
	}

	// Node3D 特定信息
	Node3D *node_3d = Object::cast_to<Node3D>(p_node);
	if (node_3d) {
		Dictionary pos;
		pos["x"] = node_3d->get_position().x;
		pos["y"] = node_3d->get_position().y;
		pos["z"] = node_3d->get_position().z;
		info["position"] = pos;
		Dictionary rot;
		rot["x"] = node_3d->get_rotation_degrees().x;
		rot["y"] = node_3d->get_rotation_degrees().y;
		rot["z"] = node_3d->get_rotation_degrees().z;
		info["rotation_degrees"] = rot;
		Dictionary scale;
		scale["x"] = node_3d->get_scale().x;
		scale["y"] = node_3d->get_scale().y;
		scale["z"] = node_3d->get_scale().z;
		info["scale"] = scale;
	}

	return info;
}

String NodeTools::_safe_name(const String &p_requested, const String &p_fallback) const {
	String trimmed = p_requested.strip_edges();
	return trimmed.is_empty() ? p_fallback : trimmed;
}

void NodeTools::_select_node(Node *p_node) const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor || !p_node) {
		return;
	}

	EditorSelection *selection = editor->get_selection();
	if (selection) {
		selection->clear();
		selection->add_node(p_node);
		editor->edit_node(p_node);
	}
}

void NodeTools::_commit_undoable_properties(Object *p_object, const Dictionary &p_changes, const String &p_action_name, bool p_undoable) const {
	if (!p_object || p_changes.is_empty()) {
		return;
	}

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		// 无编辑器，直接设置
		Array keys = p_changes.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			p_object->set(key, p_changes[key]);
		}
		return;
	}

	UndoRedo *undo_redo = nullptr;

	// 尝试获取编辑器的 UndoRedo
	if (_plugin) {
		undo_redo = _plugin->get_undo_redo();
	}

	if (p_undoable && undo_redo) {
		// 使用 UndoRedo 系统
		undo_redo->create_action(p_action_name);
		Array keys = p_changes.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			undo_redo->add_do_property(p_object, key, p_changes[key]);
			undo_redo->add_undo_property(p_object, key, p_object->get(key));
		}
		undo_redo->commit_action();
	} else {
		// 直接设置属性
		Array keys = p_changes.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			p_object->set(key, p_changes[key]);
		}
	}
}

bool NodeTools::_has_property(Object *p_object, const String &p_property) const {
	if (!p_object) {
		return false;
	}
	TypedArray<Dictionary> prop_list = p_object->get_property_list();
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		if (String(prop_info.get("name", "")) == p_property) {
			return true;
		}
	}
	return false;
}

Vector2 NodeTools::_to_vector2(const Variant &p_value) const {
	switch (p_value.get_type()) {
		case Variant::VECTOR2:
			return p_value;
		case Variant::ARRAY: {
			Array arr = p_value;
			if (arr.size() >= 2) {
				return Vector2(double(arr[0]), double(arr[1]));
			}
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return Vector2(double(d.get("x", 0.0)), double(d.get("y", 0.0)));
		} break;
		case Variant::STRING: {
			// 解析 "x,y" 格式
			String s = p_value;
			Vector<String> parts = s.split(",");
			if (parts.size() >= 2) {
				return Vector2(parts[0].to_float(), parts[1].to_float());
			}
		} break;
		default:
			break;
	}
	return Vector2();
}

Vector3 NodeTools::_to_vector3(const Variant &p_value) const {
	switch (p_value.get_type()) {
		case Variant::VECTOR3:
			return p_value;
		case Variant::ARRAY: {
			Array arr = p_value;
			if (arr.size() >= 3) {
				return Vector3(double(arr[0]), double(arr[1]), double(arr[2]));
			}
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return Vector3(double(d.get("x", 0.0)), double(d.get("y", 0.0)), double(d.get("z", 0.0)));
		} break;
		case Variant::STRING: {
			String s = p_value;
			Vector<String> parts = s.split(",");
			if (parts.size() >= 3) {
				return Vector3(parts[0].to_float(), parts[1].to_float(), parts[2].to_float());
			}
		} break;
		default:
			break;
	}
	return Vector3();
}

void NodeTools::_find_nodes_recursive(Node *p_node, const String &p_name_contains, const String &p_class_name, const String &p_script_path, int p_max_results, Array &p_results) const {
	if (p_results.size() >= p_max_results) {
		return;
	}

	bool name_ok = p_name_contains.is_empty() || String(p_node->get_name()).to_lower().contains(p_name_contains);
	bool class_ok = p_class_name.is_empty() || p_node->is_class(p_class_name);

	bool script_ok = true;
	if (!p_script_path.is_empty()) {
		Ref<Script> script = p_node->get_script();
		script_ok = script.is_valid() && script->get_resource_path() == p_script_path;
	}

	if (name_ok && class_ok && script_ok) {
		p_results.push_back(_node_to_summary(p_node));
	}

	for (int i = 0; i < p_node->get_child_count() && p_results.size() < p_max_results; i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_find_nodes_recursive(child, p_name_contains, p_class_name, p_script_path, p_max_results, p_results);
		}
	}
}
