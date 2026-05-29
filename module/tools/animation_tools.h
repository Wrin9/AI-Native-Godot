/**
 * animation_tools.h - 动画工具
 *
 * AnimationPlayer、Animation 资源的创建和操作工具。
 * 从 funplay_core_tools.gd 的动画相关方法迁移而来。
 */

#ifndef ANIMATION_TOOLS_H
#define ANIMATION_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;
class Node;
class AnimationPlayer;
class AnimationLibrary;

class AnimationTools : public RefCounted {
	GDCLASS(AnimationTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建 AnimationPlayer
	String create_animation_player(const Dictionary &p_args);

	// 创建动画片段
	String create_animation_clip(const Dictionary &p_args);

	// 添加动画轨道
	String add_animation_track(const Dictionary &p_args);

	// 列出动画
	String list_animations(const Dictionary &p_args);

	// 播放动画
	String play_animation(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Node *_get_edited_scene_root() const;
	Node *_resolve_node_path(const String &p_path) const;
	AnimationPlayer *_resolve_animation_player(const String &p_path) const;
	void _assign_owner_recursive(Node *p_node, Node *p_owner) const;
	Dictionary _node_to_summary(Node *p_node) const;
	String _safe_name(const String &p_requested, const String &p_fallback) const;
	void _select_node(Node *p_node) const;
	AnimationLibrary *_get_or_create_animation_library(AnimationPlayer *p_player, const String &p_library_name) const;
	int _animation_track_type(const String &p_track_type) const;
};

#endif // ANIMATION_TOOLS_H
