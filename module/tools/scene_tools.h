/**
 * scene_tools.h - 场景操作工具
 * 
 * 场景文件管理、场景树操作、场景实例化等工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的场景相关方法迁移而来。
 */

#ifndef SCENE_TOOLS_H
#define SCENE_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"

class EditorPlugin;
class Node;

class SceneTools : public RefCounted {
	GDCLASS(SceneTools, RefCounted);

public:
	// 设置编辑器插件引用
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 获取当前场景信息
	String get_scene_info(const Dictionary &p_args);

	// 获取场景树结构
	String get_scene_tree(const Dictionary &p_args);

	// 列出所有场景文件
	String list_scenes(const Dictionary &p_args);

	// 列出打开的场景
	String list_open_scenes(const Dictionary &p_args);

	// 打开场景
	String open_scene(const Dictionary &p_args);

	// 创建新场景
	String create_new_scene(const Dictionary &p_args);

	// 保存场景
	String save_scene(const Dictionary &p_args);

	// 另存为
	String save_scene_as(const Dictionary &p_args);

	// 实例化子场景
	String instantiate_scene(const Dictionary &p_args);

	// 从节点创建打包场景
	String create_packed_scene_from_node(const Dictionary &p_args);

	// 获取打包场景信息
	String get_packed_scene_info(const Dictionary &p_args);

	// 获取当前选择
	String get_selection(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Node *_get_edited_scene_root() const;
	Node *_resolve_node_path(const String &p_path) const;
	String _normalize_path(const String &p_path) const;
	void _assign_owner_recursive(Node *p_node, Node *p_owner) const;
	String _ensure_parent_dir(const String &p_path) const;
	void _refresh_filesystem() const;
	void _collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const;
	Dictionary _node_to_summary(Node *p_node) const;
	Dictionary _serialize_scene_tree(Node *p_node, int p_max_depth, int p_depth = 0) const;
	int _count_nodes(Node *p_node) const;
	Dictionary _build_scene_info(Node *p_scene_root) const;
	String _safe_name(const String &p_requested, const String &p_fallback) const;
	void _select_node(Node *p_node) const;
};

#endif // SCENE_TOOLS_H
