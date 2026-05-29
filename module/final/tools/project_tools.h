/**
 * project_tools.h - 项目工具
 *
 * 项目设置、Autoload、InputMap、Addon 管理工具。
 * 从 funplay_core_tools.gd 的项目相关方法迁移而来。
 */

#ifndef PROJECT_TOOLS_H
#define PROJECT_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class ProjectTools : public RefCounted {
	GDCLASS(ProjectTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 获取项目信息
	String get_project_info(const Dictionary &p_args);

	// 列出项目设置
	String list_project_settings(const Dictionary &p_args);

	// 获取项目设置
	String get_project_setting(const Dictionary &p_args);

	// 设置项目设置
	String set_project_setting(const Dictionary &p_args);

	// 列出项目特性
	String list_project_features(const Dictionary &p_args);

	// 列出 Addon 插件
	String list_addons(const Dictionary &p_args);

	// 启用/禁用 Addon
	String set_addon_enabled(const Dictionary &p_args);

	// 列出 Autoload
	String list_autoloads(const Dictionary &p_args);

	// 设置 Autoload
	String set_autoload(const Dictionary &p_args);

	// 移除 Autoload
	String remove_autoload(const Dictionary &p_args);

	// 列出输入动作
	String list_input_actions(const Dictionary &p_args);

	// 获取输入动作
	String get_input_action(const Dictionary &p_args);

	// 添加输入动作
	String add_input_action(const Dictionary &p_args);

	// 移除输入动作
	String remove_input_action(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Array _list_autoloads_internal() const;
	Dictionary _build_input_action_info(const String &p_action_name) const;
	Dictionary _serialize_input_event(const Ref<InputEvent> &p_event) const;
	String _normalize_path(const String &p_path) const;
};

#endif // PROJECT_TOOLS_H
