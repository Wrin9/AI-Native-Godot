/**
 * ui_tools.h - UI 创建工具
 *
 * Godot Control / CanvasLayer UI 控件的创建和配置工具。
 * 从 funplay_core_tools.gd 的 UI 相关方法迁移而来。
 */

#ifndef UI_TOOLS_H
#define UI_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/math/vector2.h"
#include "core/math/color.h"

class EditorPlugin;
class Node;
class Control;

class UITools : public RefCounted {
	GDCLASS(UITools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建 UI 根节点
	String create_ui_root(const Dictionary &p_args);

	// 创建任意 Control 子类
	String create_control(const Dictionary &p_args);

	// 创建 Label
	String create_label(const Dictionary &p_args);

	// 创建 Button
	String create_button(const Dictionary &p_args);

	// 创建 Panel
	String create_panel(const Dictionary &p_args);

	// 创建 TextureRect
	String create_texture_rect(const Dictionary &p_args);

	// 创建 Container
	String create_container(const Dictionary &p_args);

	// 设置 Control 布局
	String set_control_layout(const Dictionary &p_args);

	// 设置 Control 大小标志
	String set_control_size_flags(const Dictionary &p_args);

	// 设置 Control 文本
	String set_control_text(const Dictionary &p_args);

	// 设置 Control 主题覆盖
	String set_control_theme_override(const Dictionary &p_args);

	// 连接节点信号
	String connect_node_signal(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Node *_get_edited_scene_root() const;
	Node *_resolve_node_path(const String &p_path) const;
	Control *_resolve_control(const String &p_path) const;
	void _assign_owner_recursive(Node *p_node, Node *p_owner) const;
	Dictionary _node_to_summary(Node *p_node) const;
	Dictionary _build_control_info(Control *p_control) const;
	String _safe_name(const String &p_requested, const String &p_fallback) const;
	void _select_node(Node *p_node) const;
	void _apply_layout_preset(Control *p_control, const String &p_preset) const;
	void _commit_undoable_properties(Object *p_object, const Dictionary &p_changes, const String &p_action_name, bool p_undoable) const;
	bool _has_property(Object *p_object, const String &p_property) const;
	Vector2 _to_vector2(const Variant &p_value) const;
	Color _to_color(const Variant &p_value) const;
	int _parse_size_flags(const Variant &p_value) const;
	String _normalize_path(const String &p_path) const;
};

#endif // UI_TOOLS_H
