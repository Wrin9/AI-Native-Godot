/**
 * node_tools.h - 节点操作工具
 *
 * 节点的创建、删除、属性设置、变换等操作工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的节点相关方法迁移而来。
 */

#ifndef NODE_TOOLS_H
#define NODE_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

class EditorPlugin;
class Node;
class UndoRedo;

class NodeTools : public RefCounted {
	GDCLASS(NodeTools, RefCounted);

public:
	// 设置编辑器插件引用
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 获取节点详细信息
	String get_node_info(const Dictionary &p_args);

	// 查找节点
	String find_nodes(const Dictionary &p_args);

	// 选择节点
	String select_node(const Dictionary &p_args);

	// 创建节点
	String create_node(const Dictionary &p_args);

	// 复制节点
	String duplicate_node(const Dictionary &p_args);

	// 重命名节点
	String rename_node(const Dictionary &p_args);

	// 重新设置父节点
	String reparent_node(const Dictionary &p_args);

	// 删除节点
	String remove_node(const Dictionary &p_args);

	// 设置节点单个属性
	String set_node_property(const Dictionary &p_args);

	// 设置节点多个属性
	String set_node_properties(const Dictionary &p_args);

	// 设置2D变换
	String set_transform_2d(const Dictionary &p_args);

	// 设置3D变换
	String set_transform_3d(const Dictionary &p_args);

	// 设置节点脚本
	String set_node_script(const Dictionary &p_args);

	// 列出节点属性
	String list_node_properties(const Dictionary &p_args);

	// 列出节点信号
	String list_node_signals(const Dictionary &p_args);

	// 列出节点方法
	String list_node_methods(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Node *_get_edited_scene_root() const;
	Node *_resolve_node_path(const String &p_path) const;
	String _normalize_path(const String &p_path) const;
	void _assign_owner_recursive(Node *p_node, Node *p_owner) const;
	Dictionary _node_to_summary(Node *p_node) const;
	Dictionary _build_node_info(Node *p_node) const;
	String _safe_name(const String &p_requested, const String &p_fallback) const;
	void _select_node(Node *p_node) const;
	void _commit_undoable_properties(Object *p_object, const Dictionary &p_changes, const String &p_action_name, bool p_undoable) const;
	bool _has_property(Object *p_object, const String &p_property) const;
	Vector2 _to_vector2(const Variant &p_value) const;
	Vector3 _to_vector3(const Variant &p_value) const;
	void _find_nodes_recursive(Node *p_node, const String &p_name_contains, const String &p_class_name, const String &p_script_path, int p_max_results, Array &p_results) const;
};

#endif // NODE_TOOLS_H
