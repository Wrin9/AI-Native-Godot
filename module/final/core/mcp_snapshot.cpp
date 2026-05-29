/**************************************************************************/
/*  mcp_snapshot.cpp                                                      */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 编辑器状态快照实现                                                       */
/**************************************************************************/

#include "mcp_snapshot.h"

#include "core/os/os.h"
#include "core/crypto/crypto_core.h"
#include "core/templates/hash_set.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/editor_file_system.h"

// ============================================================
// 构造 / 析构
// ============================================================

MCPSnapshot::MCPSnapshot() {
}

MCPSnapshot::~MCPSnapshot() {
}

// ============================================================
// 公开 API
// ============================================================

Dictionary MCPSnapshot::capture_full() {
	Dictionary snapshot;

	uint64_t now = OS::get_singleton()->get_ticks_msec();
	snapshot["timestamp_msec"] = (int64_t)now;

	// 场景树快照
	snapshot["scene_tree"] = get_scene_tree_snapshot(_max_depth);

	// 项目文件快照
	snapshot["project_files"] = get_project_files_snapshot();

	// 编辑器选择快照
	snapshot["selection"] = get_selection_snapshot();

	// 脚本列表快照
	snapshot["scripts"] = get_scripts_snapshot();

	// 项目设置快照
	snapshot["project_settings"] = get_project_settings_snapshot();

	// 计算哈希
	_last_snapshot_hash = _compute_hash(snapshot);
	snapshot["hash"] = _last_snapshot_hash;

	// 缓存
	_last_full_snapshot = snapshot;

	return snapshot;
}

Dictionary MCPSnapshot::capture_diff() {
	Dictionary diff;

	if (_last_full_snapshot.is_empty()) {
		// 无缓存，返回完整快照
		Dictionary full = capture_full();
		diff["type"] = "full";
		diff["data"] = full;
		return diff;
	}

	// 捕获当前状态
	Dictionary current = capture_full();

	// 比较差异
	Dictionary changes;

	// 场景树差异
	Dictionary old_tree = _last_full_snapshot.get("scene_tree", Dictionary());
	Dictionary new_tree = current.get("scene_tree", Dictionary());
	Dictionary tree_diff = _diff_scene_trees(old_tree, new_tree);
	if (!tree_diff.is_empty()) {
		changes["scene_tree"] = tree_diff;
	}

	// 选择差异
	Dictionary old_sel = _last_full_snapshot.get("selection", Dictionary());
	Dictionary new_sel = current.get("selection", Dictionary());
	if (old_sel != new_sel) {
		changes["selection"] = new_sel;
	}

	diff["type"] = changes.is_empty() ? "none" : "incremental";
	diff["changes"] = changes;
	diff["from_hash"] = _last_full_snapshot.get("hash", "");
	diff["to_hash"] = current.get("hash", "");
	diff["timestamp_msec"] = (int64_t)OS::get_singleton()->get_ticks_msec();

	return diff;
}

Dictionary MCPSnapshot::get_scene_tree_snapshot(int p_max_depth) {
	Dictionary result;

	SceneTree *tree = SceneTree::get_singleton();
	if (!tree) {
		result["error"] = "SceneTree not available";
		return result;
	}

	// 检查缓存
	uint64_t now = OS::get_singleton()->get_ticks_msec();
	if (!_cached_scene_tree.is_empty() && (now - _cache_timestamp_msec) < _cache_ttl_msec) {
		return _cached_scene_tree;
	}

	Node *root = tree->get_root();
	if (!root) {
		result["error"] = "No root node";
		return result;
	}

	// 遍历场景树
	_walk_scene_tree(root, result, 0, p_max_depth);

	// 更新缓存
	_cached_scene_tree = result;
	_cache_timestamp_msec = now;

	return result;
}

Dictionary MCPSnapshot::get_project_files_snapshot() const {
	Dictionary result;

	EditorFileSystem *efs = EditorFileSystem::get_singleton();
	if (!efs) {
		result["error"] = "EditorFileSystem not available";
		return result;
	}

	Array file_list;

	// 遍历编辑器文件系统
	// 注意：这里用简化的实现，实际可以通过 EditorFileSystem 的 API 获取完整文件列表
	_collect_project_files("res://", file_list, 0);

	result["file_count"] = file_list.size();
	result["files"] = file_list;
	result["timestamp_msec"] = (int64_t)OS::get_singleton()->get_ticks_msec();

	return result;
}

Dictionary MCPSnapshot::get_selection_snapshot() const {
	Dictionary result;

	SceneTree *tree = SceneTree::get_singleton();
	if (!tree) {
		return result;
	}

	// 获取编辑器选择 - 通过 EditorPlugin 的 get_editor_interface
	// 这里提供基础框架，具体实现依赖 EditorInterface
	Node *root = tree->get_root();
	if (!root) return result;

	// 尝试获取编辑器选择
	// EditorInterface *ei = EditorInterface::get_singleton();
	// 在 Module 中可以直接使用 EditorNode
	Array selected_nodes;

	result["selected_nodes"] = selected_nodes;
	result["selected_count"] = selected_nodes.size();

	return result;
}

Dictionary MCPSnapshot::get_scripts_snapshot() const {
	Dictionary result;
	Array script_list;

	// 收集已加载的脚本
	// 通过 ResourceCache 获取所有 GDScript 资源
	// 这是一个简化实现

	result["scripts"] = script_list;
	result["script_count"] = script_list.size();

	return result;
}

Dictionary MCPSnapshot::get_project_settings_snapshot() const {
	Dictionary result;

	// 收集常用项目设置
	static const char *tracked_settings[] = {
		"application/config/name",
		"application/config/description",
		"application/run/main_scene",
		"display/window/size/viewport_width",
		"display/window/size/viewport_height",
		"rendering/renderer/rendering_method",
		"physics/2d/engine",
		"physics/3d/engine",
		nullptr
	};

	Dictionary settings;
	for (int i = 0; tracked_settings[i] != nullptr; i++) {
		String key = tracked_settings[i];
		Variant value = ProjectSettings::get_singleton()->get_setting(key);
		settings[key] = value;
	}

	result["settings"] = settings;
	result["timestamp_msec"] = (int64_t)OS::get_singleton()->get_ticks_msec();

	return result;
}

void MCPSnapshot::set_max_depth(int p_depth) {
	_max_depth = p_depth;
	// 清除缓存，因为深度改变了
	_cached_scene_tree = Dictionary();
}

int MCPSnapshot::get_max_depth() const {
	return _max_depth;
}

void MCPSnapshot::set_include_resource_paths(bool p_include) {
	_include_resource_paths = p_include;
}

bool MCPSnapshot::get_include_resource_paths() const {
	return _include_resource_paths;
}

String MCPSnapshot::get_last_snapshot_hash() const {
	return _last_snapshot_hash;
}

void MCPSnapshot::clear_cache() {
	_cached_scene_tree = Dictionary();
	_cache_timestamp_msec = 0;
	_last_full_snapshot = Dictionary();
	_last_snapshot_hash = "";
}

// ============================================================
// 内部方法
// ============================================================

void MCPSnapshot::_walk_scene_tree(Node *p_node, Dictionary &p_out, int p_depth, int p_max_depth) {
	if (!p_node) return;

	// 检查深度限制
	if (p_max_depth >= 0 && p_depth > p_max_depth) return;

	Dictionary node_info = _node_to_dict(p_node);

	String node_name = p_node->get_name();
	if (node_name.is_empty()) {
		node_name = vformat("_unnamed_%d", p_node->get_instance_id());
	}

	p_out[node_name] = node_info;

	// 递归遍历子节点
	Dictionary children_dict;
	int child_count = p_node->get_child_count();
	for (int i = 0; i < child_count; i++) {
		Node *child = p_node->get_child(i);
		if (child && !child->is_internal()) {
			_walk_scene_tree(child, children_dict, p_depth + 1, p_max_depth);
		}
	}

	if (!children_dict.is_empty()) {
		node_info["children"] = children_dict;
	}
}

Dictionary MCPSnapshot::_node_to_dict(Node *p_node) const {
	Dictionary info;
	if (!p_node) return info;

	info["type"] = p_node->get_class();
	info["path"] = String(p_node->get_path());
	info["instance_id"] = p_node->get_instance_id();

	// 包含常用属性
	if (p_node->is_class("CanvasItem")) {
		info["visible"] = p_node->call("is_visible");
	}

	if (p_node->is_class("Control")) {
		Dictionary rect;
		rect["x"] = p_node->get("position").operator Vector2().x;
		rect["y"] = p_node->get("position").operator Vector2().y;
		rect["width"] = p_node->get("size").operator Vector2().x;
		rect["height"] = p_node->get("size").operator Vector2().y;
		info["rect"] = rect;
	}

	if (p_node->is_class("Node2D")) {
		Dictionary transform;
		transform["x"] = p_node->get("position").operator Vector2().x;
		transform["y"] = p_node->get("position").operator Vector2().y;
		transform["rotation"] = p_node->get("rotation");
		transform["scale_x"] = p_node->get("scale").operator Vector2().x;
		transform["scale_y"] = p_node->get("scale").operator Vector2().y;
		info["transform"] = transform;
	}

	if (p_node->is_class("Node3D")) {
		Dictionary transform;
		Vector3 pos = p_node->get("position");
		transform["x"] = pos.x;
		transform["y"] = pos.y;
		transform["z"] = pos.z;
		info["transform"] = transform;
	}

	// 包含脚本信息
	Ref<Script> script = p_node->get_script();
	if (script.is_valid()) {
		info["has_script"] = true;
		if (_include_resource_paths) {
			info["script_path"] = script->get_path();
		}
	} else {
		info["has_script"] = false;
	}

	// 子节点数量
	info["child_count"] = p_node->get_child_count(false);

	return info;
}

String MCPSnapshot::_compute_hash(const Dictionary &p_snapshot) const {
	// 使用 JSON 序列化后计算哈希
	String json = JSON::stringify(p_snapshot, "", true);
	String hash = CryptoCore::md5_text(json.utf8().ptr(), json.utf8_length());
	return hash;
}

void MCPSnapshot::_collect_project_files(const String &p_path, Array &p_files, int p_depth) const {
	// 递归收集项目文件（简化实现）
	// 实际项目中应使用 EditorFileSystem API
	if (p_depth > 10) return; // 防止无限递归

	Ref<DirAccess> dir = DirAccess::open(p_path);
	if (dir.is_null()) return;

	dir->list_dir_begin();
	String file_name = dir->get_next();

	while (!file_name.is_empty()) {
		if (file_name == "." || file_name == ".." || file_name.begins_with(".")) {
			file_name = dir->get_next();
			continue;
		}

		String full_path = p_path.path_join(file_name);

		if (dir->current_is_dir()) {
			_collect_project_files(full_path, p_files, p_depth + 1);
		} else {
			// 过滤常见的可识别文件类型
			String ext_with_dot = "." + file_name.get_extension();
			bool is_tracked = ext_with_dot.is_empty()
					|| ext_with_dot == ".tscn" || ext_with_dot == ".scn"
					|| ext_with_dot == ".gd" || ext_with_dot == ".gdshader" || ext_with_dot == ".cs"
					|| ext_with_dot == ".tres" || ext_with_dot == ".json"
					|| ext_with_dot == ".cfg" || ext_with_dot == ".toml" || ext_with_dot == ".yaml"
					|| ext_with_dot == ".png" || ext_with_dot == ".jpg" || ext_with_dot == ".svg"
					|| ext_with_dot == ".wav" || ext_with_dot == ".ogg" || ext_with_dot == ".mp3"
					|| ext_with_dot == ".fbx" || ext_with_dot == ".glb" || ext_with_dot == ".gltf" || ext_with_dot == ".obj";

			if (is_tracked) {
				Dictionary file_info;
				file_info["path"] = full_path;
				file_info["name"] = file_name;
				file_info["extension"] = file_name.get_extension();
				p_files.push_back(file_info);
			}
		}

		file_name = dir->get_next();
	}

	dir->list_dir_end();
}

Dictionary MCPSnapshot::_diff_dictionaries(const Dictionary &p_old, const Dictionary &p_new, const String &p_prefix) const {
	Dictionary diff;

	// 查找新增的键
	Array new_keys = p_new.keys();
	for (int i = 0; i < new_keys.size(); i++) {
		String key = new_keys[i];
		String full_key = p_prefix.is_empty() ? key : p_prefix + "." + key;

		if (!p_old.has(key)) {
			Dictionary added;
			added["action"] = "added";
			added["value"] = p_new[key];
			diff[full_key] = added;
			continue;
		}

		// 值变化
		Variant old_val = p_old[key];
		Variant new_val = p_new[key];

		if (old_val.get_type() == Variant::DICTIONARY && new_val.get_type() == Variant::DICTIONARY) {
			// 递归比较
			Dictionary sub_diff = _diff_dictionaries(old_val, new_val, full_key);
			diff.merge(sub_diff);
		} else if (old_val != new_val) {
			Dictionary changed;
			changed["action"] = "changed";
			changed["old_value"] = old_val;
			changed["new_value"] = new_val;
			diff[full_key] = changed;
		}
	}

	// 查找删除的键
	Array old_keys = p_old.keys();
	for (int i = 0; i < old_keys.size(); i++) {
		String key = old_keys[i];
		String full_key = p_prefix.is_empty() ? key : p_prefix + "." + key;

		if (!p_new.has(key)) {
			Dictionary removed;
			removed["action"] = "removed";
			removed["old_value"] = p_old[key];
			diff[full_key] = removed;
		}
	}

	return diff;
}

Dictionary MCPSnapshot::_diff_scene_trees(const Dictionary &p_old_tree, const Dictionary &p_new_tree) const {
	return _diff_dictionaries(p_old_tree, p_new_tree, "scene");
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPSnapshot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("capture_full"), &MCPSnapshot::capture_full);
	ClassDB::bind_method(D_METHOD("capture_diff"), &MCPSnapshot::capture_diff);
	ClassDB::bind_method(D_METHOD("get_scene_tree_snapshot", "max_depth"), &MCPSnapshot::get_scene_tree_snapshot, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_project_files_snapshot"), &MCPSnapshot::get_project_files_snapshot);
	ClassDB::bind_method(D_METHOD("get_selection_snapshot"), &MCPSnapshot::get_selection_snapshot);
	ClassDB::bind_method(D_METHOD("get_scripts_snapshot"), &MCPSnapshot::get_scripts_snapshot);
	ClassDB::bind_method(D_METHOD("get_project_settings_snapshot"), &MCPSnapshot::get_project_settings_snapshot);
	ClassDB::bind_method(D_METHOD("set_max_depth", "depth"), &MCPSnapshot::set_max_depth);
	ClassDB::bind_method(D_METHOD("get_max_depth"), &MCPSnapshot::get_max_depth);
	ClassDB::bind_method(D_METHOD("set_include_resource_paths", "include"), &MCPSnapshot::set_include_resource_paths);
	ClassDB::bind_method(D_METHOD("get_include_resource_paths"), &MCPSnapshot::get_include_resource_paths);
	ClassDB::bind_method(D_METHOD("get_last_snapshot_hash"), &MCPSnapshot::get_last_snapshot_hash);
	ClassDB::bind_method(D_METHOD("clear_cache"), &MCPSnapshot::clear_cache);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_depth", PROPERTY_HINT_RANGE, "-1,20,1"), "set_max_depth", "get_max_depth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "include_resource_paths"), "set_include_resource_paths", "get_include_resource_paths");

	ADD_SIGNAL(MethodInfo("snapshot_captured",
			PropertyInfo(Variant::STRING, "type"),
			PropertyInfo(Variant::DICTIONARY, "data")));

	ADD_SIGNAL(MethodInfo("scene_tree_changed",
			PropertyInfo(Variant::DICTIONARY, "diff")));
}
