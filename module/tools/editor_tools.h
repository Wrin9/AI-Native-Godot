/**
 * editor_tools.h - 编辑器工具
 *
 * 编辑器撤销/重做、代码检查、状态获取和设置工具。
 */

#ifndef EDITOR_TOOLS_H
#define EDITOR_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class EditorTools : public RefCounted {
	GDCLASS(EditorTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 撤销
	String undo(const Dictionary &p_args);

	// 重做
	String redo(const Dictionary &p_args);

	// 运行代码（语法检查）
	String run_code(const Dictionary &p_args);

	// 获取编辑器状态
	String get_editor_state(const Dictionary &p_args);

	// 设置编辑器设置
	String set_editor_setting(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // EDITOR_TOOLS_H
