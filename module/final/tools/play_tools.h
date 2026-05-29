/**
 * play_tools.h - 运行时工具
 *
 * 播放模式控制、输入模拟、时间缩放等工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的运行时相关方法迁移而来。
 */

#ifndef PLAY_TOOLS_H
#define PLAY_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/math/vector2.h"

class EditorPlugin;

class PlayTools : public RefCounted {
	GDCLASS(PlayTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 获取播放状态
	String get_play_state(const Dictionary &p_args);

	// 进入播放模式
	String enter_play_mode(const Dictionary &p_args);

	// 播放主场景
	String play_main_scene(const Dictionary &p_args);

	// 退出播放模式
	String exit_play_mode(const Dictionary &p_args);

	// 模拟输入动作
	String simulate_action(const Dictionary &p_args);

	// 模拟键盘事件
	String simulate_key_event(const Dictionary &p_args);

	// 模拟鼠标按钮
	String simulate_mouse_button(const Dictionary &p_args);

	// 模拟鼠标拖拽
	String simulate_mouse_drag(const Dictionary &p_args);

	// 模拟输入序列
	String simulate_input_sequence(const Dictionary &p_args);

	// 获取时间缩放
	String get_time_scale(const Dictionary &p_args);

	// 设置时间缩放
	String set_time_scale(const Dictionary &p_args);

	// 捕获编辑器视图
	String capture_editor_view(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	String _normalize_path(const String &p_path) const;
	String _ensure_parent_dir(const String &p_path) const;
	Vector2 _to_vector2(const Variant &p_value) const;
	int _to_keycode(const Variant &p_value) const;
	int _to_mouse_button(const Variant &p_value) const;
};

#endif // PLAY_TOOLS_H
