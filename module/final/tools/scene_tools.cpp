/**
 * scene_tools.cpp - 场景操作工具实现
 *
 * 场景文件管理、场景树操作、场景实例化等工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的场景相关方法迁移而来。
 */

#include "scene_tools.h"

#include "editor/editor_node.h"
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/editor_file_system.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/resource_saver.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/config/project_settings.h"

// ============================================================
// 设置编辑器插件引用
// ============================================================
void SceneTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 获取当前场景信息
// ============================================================
String SceneTools::get_scene_info(const Dictionary &p_args) {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No scene is currently open in the editor."})";
	}

	Dictionary info = _build_scene_info(scene_root);

	// 附加打开的场景列表和运行状态
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		info["open_scenes"] = editor->get_open_scenes();
		info["is_playing_scene"] = editor->is_playing_scene();
	}

	Dictionary engine_info;
	engine_info["time_scale"] = Engine::get_singleton()->get_time_scale();
	info["time_scale"] = Engine::get_singleton()->get_time_scale();

	return JSON::stringify(info, "\t");
}

// ============================================================
// 获取场景树结构
// ============================================================
String SceneTools::get_scene_tree(const Dictionary &p_args) {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No scene is currently open in the editor."})";
	}

	int max_depth = 4;
	if (p_args.has("max_depth")) {
		max_depth = CLAMP(int(p_args["max_depth"]), 1, 100);
	}

	Dictionary tree = _serialize_scene_tree(scene_root, max_depth);
	return JSON::stringify(tree, "\t");
}

// ============================================================
// 列出所有场景文件
// ============================================================
String SceneTools::list_scenes(const Dictionary &p_args) {
	String root_path = _normalize_path(p_args.get("path", "res://"));
	int max_entries = CLAMP(int(p_args.get("max_entries", 300)), 1, 3000);
	bool recursive = p_args.get("recursive", true);

	Vector<String> extensions;
	extensions.push_back(".tscn");
	extensions.push_back(".scn");

	Array scene_paths;
	_collect_matching_files(root_path, recursive, max_entries, scene_paths, extensions);

	Dictionary result;
	result["path"] = root_path;
	result["scene_count"] = scene_paths.size();
	result["scenes"] = scene_paths;

	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		result["open_scenes"] = editor->get_open_scenes();
	}

	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出打开的场景
// ============================================================
String SceneTools::list_open_scenes(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	Array open_scenes = editor->get_open_scenes();
	Dictionary result;
	result["open_scenes"] = open_scenes;
	result["count"] = open_scenes.size();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 打开场景
// ============================================================
String SceneTools::open_scene(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	if (!FileAccess::file_exists(path)) {
		return vformat(R"({"error": "Scene not found: %s"})", path);
	}

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	bool set_inherited = p_args.get("set_inherited", false);
	editor->open_scene_from_path(path, set_inherited);

	return vformat(R"({"opened": "%s"})", path);
}

// ============================================================
// 创建新场景
// ============================================================
String SceneTools::create_new_scene(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	String root_type = String(p_args.get("root_type", "Node2D")).strip_edges();
	if (!ClassDB::class_exists(root_type)) {
		return vformat(R"({"error": "Unknown Godot class '%s'.'})", root_type);
	}

	// 实例化根节点
	Object *obj = ClassDB::instantiate(root_type.utf8().get_data());
	if (!obj || !Object::cast_to<Node>(obj)) {
		if (obj) {
			memdelete(obj);
		}
		return vformat(R"({"error": "'%s' is not instantiable as a Node."})", root_type);
	}

	Node *root = Object::cast_to<Node>(obj);
	String root_name = _safe_name(String(p_args.get("root_name", root_type)).strip_edges(), root_type);
	root->set_name(root_name);

	// 附加脚本（如果指定）
	String script_path = _normalize_path(p_args.get("script_path", ""));
	if (!script_path.is_empty()) {
		Ref<Resource> script_res = ResourceLoader::load(script_path);
		if (script_res.is_null() || !script_ref->is_class("Script")) {
			memdelete(root);
			return vformat(R"({"error": "Script not found or invalid: %s"})", script_path);
		}
		root->set_script(script_res);
	}

	// 打包场景
	Ref<PackedScene> packed;
	packed.instantiate();
	Error pack_err = packed->pack(root);
	if (pack_err != OK) {
		memdelete(root);
		return vformat(R"({"error": "Failed to pack scene (code %d)."})", (int)pack_err);
	}

	// 确保父目录存在
	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		memdelete(root);
		return vformat(R"({"error": "Failed to create parent directory for %s"})", path);
	}

	// 保存场景
	Error save_err = ResourceSaver::save(packed, path, ResourceSaver::FLAG_CHANGE_PATH);
	memdelete(root);
	if (save_err != OK) {
		return vformat(R"({"error": "Failed to save scene to %s (code %d)."})", path, (int)save_err);
	}

	_refresh_filesystem();

	// 可选打开场景
	if (bool(p_args.get("open_after", true))) {
		EditorInterface *editor = EditorInterface::get_singleton();
		if (editor) {
			editor->open_scene_from_path(path);
		}
	}

	Dictionary result;
	result["created_scene"] = path;
	result["root_type"] = root_type;
	result["root_name"] = root_name;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 保存场景
// ============================================================
String SceneTools::save_scene(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	Node *scene_root = editor->get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No edited scene is open."})";
	}

	Error save_result = editor->save_scene();
	if (save_result == OK) {
		return R"({"saved": true})";
	}
	return vformat(R"({"error": "Failed to save scene (code %d)"})", (int)save_result);
}

// ============================================================
// 另存为
// ============================================================
String SceneTools::save_scene_as(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	Node *scene_root = editor->get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No edited scene is open."})";
	}

	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		return vformat(R"({"error": "Failed to create parent directory for %s"})", path);
	}

	bool with_preview = p_args.get("with_preview", true);
	editor->save_scene_as(path, with_preview);

	return vformat(R"({"saved_as": "%s"})", path);
}

// ============================================================
// 实例化子场景
// ============================================================
String SceneTools::instantiate_scene(const Dictionary &p_args) {
	String scene_path = _normalize_path(p_args.get("scene_path", ""));
	if (scene_path.is_empty()) {
		return R"({"error": "'scene_path' is required."})";
	}

	// 加载打包场景
	Ref<PackedScene> packed = ResourceLoader::load(scene_path);
	if (packed.is_null()) {
		return vformat(R"({"error": "Scene not found or invalid: %s"})", scene_path);
	}

	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No edited scene is open."})";
	}

	// 确定父节点
	String parent_path_str = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = _resolve_node_path(parent_path_str);
	if (!parent) {
		parent = scene_root;
	}

	// 实例化
	Node *instance = packed->instantiate();
	if (!instance) {
		return vformat(R"({"error": "Failed to instantiate scene: %s"})", scene_path);
	}

	// 设置名称
	String custom_name = String(p_args.get("name", "")).strip_edges();
	if (!custom_name.is_empty()) {
		instance->set_name(custom_name);
	}

	parent->add_child(instance);
	_assign_owner_recursive(instance, scene_root);

	// 可选选中新节点
	if (bool(p_args.get("select_new_node", true))) {
		_select_node(instance);
	}

	Dictionary result;
	result["instantiated_scene"] = scene_path;
	result["instance"] = _node_to_summary(instance);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 从节点创建打包场景
// ============================================================
String SceneTools::create_packed_scene_from_node(const Dictionary &p_args) {
	String node_path_str = String(p_args.get("node_path", "")).strip_edges();
	String path = _normalize_path(p_args.get("path", ""));

	Node *node = _resolve_node_path(node_path_str);
	if (!node) {
		return R"({"error": "Node not found."})";
	}
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	Ref<PackedScene> packed;
	packed.instantiate();
	Error pack_err = packed->pack(node);
	if (pack_err != OK) {
		return vformat(R"({"error": "Failed to pack node (code %d)."})", (int)pack_err);
	}

	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		return vformat(R"({"error": "Failed to create parent directory for %s"})", path);
	}

	Error save_err = ResourceSaver::save(packed, path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"({"error": "Failed to save PackedScene to %s (code %d)."})", path, (int)save_err);
	}

	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor && bool(p_args.get("select_file", true))) {
		editor->select_file(path);
	}
	_refresh_filesystem();

	Dictionary result;
	result["source_node"] = _node_to_summary(node);
	result["packed_scene_path"] = path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 获取打包场景信息
// ============================================================
String SceneTools::get_packed_scene_info(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	Ref<PackedScene> packed = ResourceLoader::load(path);
	if (packed.is_null()) {
		return vformat(R"({"error": "PackedScene not found or invalid: %s"})", path);
	}

	Node *instance = packed->instantiate();
	if (!instance) {
		return vformat(R"({"error": "Failed to instantiate PackedScene for inspection: %s"})", path);
	}

	int max_depth = CLAMP(int(p_args.get("max_depth", 3)), 1, 100);

	Dictionary info;
	info["path"] = path;
	info["root"] = _node_to_summary(instance);
	info["node_count"] = _count_nodes(instance);
	info["tree"] = _serialize_scene_tree(instance, max_depth);

	memdelete(instance);
	return JSON::stringify(info, "\t");
}

// ============================================================
// 获取当前选择
// ============================================================
String SceneTools::get_selection(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	Array selected_nodes;
	EditorSelection *selection = editor->get_selection();
	if (selection) {
		TypedArray<Node> nodes = selection->get_selected_nodes();
		for (int i = 0; i < nodes.size(); i++) {
			Node *node = Object::cast_to<Node>(nodes[i]);
			if (node) {
				selected_nodes.push_back(_node_to_summary(node));
			}
		}
	}

	return JSON::stringify(selected_nodes, "\t");
}

// ============================================================
// _bind_methods - 注册公开方法
// ============================================================
void SceneTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &SceneTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("get_scene_info", "args"), &SceneTools::get_scene_info, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_scene_tree", "args"), &SceneTools::get_scene_tree, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_scenes", "args"), &SceneTools::list_scenes, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_open_scenes", "args"), &SceneTools::list_open_scenes, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("open_scene", "args"), &SceneTools::open_scene);
	ClassDB::bind_method(D_METHOD("create_new_scene", "args"), &SceneTools::create_new_scene);
	ClassDB::bind_method(D_METHOD("save_scene", "args"), &SceneTools::save_scene, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("save_scene_as", "args"), &SceneTools::save_scene_as);
	ClassDB::bind_method(D_METHOD("instantiate_scene", "args"), &SceneTools::instantiate_scene);
	ClassDB::bind_method(D_METHOD("create_packed_scene_from_node", "args"), &SceneTools::create_packed_scene_from_node);
	ClassDB::bind_method(D_METHOD("get_packed_scene_info", "args"), &SceneTools::get_packed_scene_info);
	ClassDB::bind_method(D_METHOD("get_selection", "args"), &SceneTools::get_selection, DEFVAL(Dictionary()));
}

// ============================================================
// 内部辅助方法
// ============================================================

Node *SceneTools::_get_edited_scene_root() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return nullptr;
	}
	return editor->get_edited_scene_root();
}

Node *SceneTools::_resolve_node_path(const String &p_path) const {
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
		// 绝对路径
		SceneTree *tree = scene_root->get_tree();
		if (tree && tree->get_root()) {
			return tree->get_root()->get_node_or_null(NodePath(identifier));
		}
	}
	return scene_root->get_node_or_null(NodePath(identifier));
}

String SceneTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

void SceneTools::_assign_owner_recursive(Node *p_node, Node *p_owner) const {
	p_node->set_owner(p_owner);
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_assign_owner_recursive(child, p_owner);
		}
	}
}

String SceneTools::_ensure_parent_dir(const String &p_path) const {
	String parent_dir = p_path.get_base_dir();
	if (parent_dir.is_empty() || parent_dir == "res://" || parent_dir == "user://") {
		return "";
	}
	Error err = DirAccess::make_dir_recursive_absolute(parent_dir);
	return (err != OK) ? vformat("Error: Failed to create directory %s", parent_dir) : "";
}

void SceneTools::_refresh_filesystem() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}
}

void SceneTools::_collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const {
	if (p_results.size() >= p_max_entries) {
		return;
	}

	Ref<DirAccess> dir = DirAccess::open(p_path);
	if (dir.is_null()) {
		return;
	}

	dir->list_dir_begin();
	String item = dir->get_next();
	while (!item.is_empty()) {
		if (p_results.size() >= p_max_entries) {
			break;
		}

		String child_path = p_path.path_join(item);
		if (dir->current_is_dir()) {
			if (p_recursive) {
				_collect_matching_files(child_path, p_recursive, p_max_entries, p_results, p_extensions);
			}
		} else {
			// 检查扩展名匹配
			String lower = child_path.to_lower();
			for (int i = 0; i < p_extensions.size(); i++) {
				if (lower.ends_with(p_extensions[i].to_lower())) {
					p_results.push_back(child_path);
					break;
				}
			}
		}
		item = dir->get_next();
	}
	dir->list_dir_end();
}

Dictionary SceneTools::_node_to_summary(Node *p_node) const {
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

Dictionary SceneTools::_serialize_scene_tree(Node *p_node, int p_max_depth, int p_depth) const {
	Dictionary summary = _node_to_summary(p_node);
	Array children;

	if (p_depth >= p_max_depth) {
		summary["truncated"] = p_node->get_child_count() > 0;
		summary["children"] = children;
		return summary;
	}

	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			children.push_back(_serialize_scene_tree(child, p_max_depth, p_depth + 1));
		}
	}
	summary["children"] = children;
	return summary;
}

int SceneTools::_count_nodes(Node *p_node) const {
	if (!p_node) {
		return 0;
	}
	int total = 1;
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			total += _count_nodes(child);
		}
	}
	return total;
}

Dictionary SceneTools::_build_scene_info(Node *p_scene_root) const {
	Dictionary info;
	info["scene_path"] = p_scene_root->get_scene_file_path();
	info["scene_root"] = _node_to_summary(p_scene_root);
	info["node_count"] = _count_nodes(p_scene_root);
	info["child_count"] = p_scene_root->get_child_count();

	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor && editor->get_selection()) {
		info["selected_nodes"] = editor->get_selection()->get_selected_nodes().size();
	}
	return info;
}

String SceneTools::_safe_name(const String &p_requested, const String &p_fallback) const {
	String trimmed = p_requested.strip_edges();
	return trimmed.is_empty() ? p_fallback : trimmed;
}

void SceneTools::_select_node(Node *p_node) const {
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
