/**
 * script_tools.h - 脚本工具
 *
 * 脚本创建、编辑、验证等工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的脚本相关方法迁移而来。
 */

#ifndef SCRIPT_TOOLS_H
#define SCRIPT_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class ScriptTools : public RefCounted {
	GDCLASS(ScriptTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建脚本
	String create_script(const Dictionary &p_args);

	// 编辑脚本（覆盖内容）
	String edit_script(const Dictionary &p_args);

	// 补丁脚本（查找替换、追加等）
	String patch_script(const Dictionary &p_args);

	// 列出项目脚本
	String list_scripts(const Dictionary &p_args);

	// 在编辑器中打开脚本
	String open_script(const Dictionary &p_args);

	// 获取脚本错误
	String get_script_errors(const Dictionary &p_args);

	// 验证脚本
	String validate_script(const Dictionary &p_args);

	// 请求脚本重载
	String request_script_reload(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	String _to_absolute(const String &p_path) const;
	String _normalize_path(const String &p_path) const;
	String _normalize_script_path(const String &p_path, const String &p_language) const;
	String _ensure_parent_dir(const String &p_path) const;
	void _refresh_filesystem() const;
	void _collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const;
	String _pascal_case(const String &p_value) const;
	String _guess_script_language(const String &p_path) const;
};

#endif // SCRIPT_TOOLS_H
