/**
 * mcp_tool_helpers.h - 共享辅助方法
 * 
 * 所有工具类共用的路径解析和节点查找方法。
 * 通过宏内联，避免代码重复。
 */
#ifndef MCP_TOOL_HELPERS_H
#define MCP_TOOL_HELPERS_H

#include "editor/editor_interface.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "core/config/project_settings.h"

// 统一的节点路径解析：支持相对路径、绝对路径、实例ID
inline Node *mcp_resolve_node_path(const String &p_path) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) return nullptr;
	Node *scene_root = editor->get_edited_scene_root();
	if (!scene_root) return nullptr;

	String identifier = p_path.strip_edges();
	if (identifier.is_empty() || identifier == ".") return scene_root;

	// 按实例 ID 查找
	String id_text = identifier.begins_with("id:") ? identifier.substr(3) : identifier;
	if (id_text.is_valid_int()) {
		ObjectID id = (ObjectID)id_text.to_int();
		Object *obj = ObjectDB::get_instance(id);
		if (obj && Object::cast_to<Node>(obj)) return Object::cast_to<Node>(obj);
	}

	// 精确路径匹配
	if (String(scene_root->get_path()) == identifier) return scene_root;

	// 相对路径：去掉 /RootName/ 前缀
	String root_name = scene_root->get_name();
	String relative;
	if (identifier.begins_with("/" + root_name + "/")) {
		relative = identifier.substr(root_name.length() + 2);
	} else if (identifier.begins_with("/")) {
		relative = identifier.substr(1);
	} else {
		relative = identifier;
	}

	Node *found = scene_root->get_node_or_null(NodePath(relative));
	if (found) return found;

	if (identifier == "/" + root_name || identifier == root_name) return scene_root;
	return scene_root->get_node_or_null(NodePath(identifier));
}

inline Node *mcp_get_scene_root() {
	EditorInterface *editor = EditorInterface::get_singleton();
	return editor ? editor->get_edited_scene_root() : nullptr;
}

inline String mcp_to_absolute(const String &p_path) {
	if (p_path.begins_with("res://") || p_path.begins_with("user://")) {
		return ProjectSettings::get_singleton()->globalize_path(p_path);
	}
	return p_path;
}

inline String mcp_normalize_path(const String &p_path) {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) return "";
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) return trimmed.simplify_path();
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

inline Dictionary mcp_node_summary(Node *p_node) {
	Dictionary d;
	if (!p_node) return d;
	d["id"] = itos(p_node->get_instance_id());
	d["instance_id"] = p_node->get_instance_id();
	d["name"] = p_node->get_name();
	d["type"] = p_node->get_class();
	d["path"] = String(p_node->get_path());
	return d;
}

#endif // MCP_TOOL_HELPERS_H
