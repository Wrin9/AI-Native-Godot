/**
 * file_tools.h - 文件操作工具
 *
 * 项目文件的读取、写入、搜索等操作工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的文件相关方法迁移而来。
 */

#ifndef FILE_TOOLS_H
#define FILE_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class FileTools : public RefCounted {
	GDCLASS(FileTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 列出文件
	String list_files(const Dictionary &p_args);

	// 搜索文件
	String search_files(const Dictionary &p_args);

	// 检查文件/目录是否存在
	String file_exists(const Dictionary &p_args);

	// 读取文件
	String read_file(const Dictionary &p_args);

	// 写入文件
	String write_file(const Dictionary &p_args);

	// 删除文件
	String delete_file(const Dictionary &p_args);

	// 移动/重命名文件
	String move_file(const Dictionary &p_args);

	// 复制文件
	String copy_file(const Dictionary &p_args);

	// 查找符号引用
	String find_usages(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	String _to_absolute(const String &p_path) const;
	String _normalize_path(const String &p_path) const;
	String _ensure_parent_dir(const String &p_path) const;
	void _refresh_filesystem() const;
	void _collect_files(const String &p_path, bool p_recursive, bool p_include_hidden, int p_max_entries, Array &p_results) const;
	void _collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const;
	bool _matches_extension(const String &p_path, const Vector<String> &p_extensions) const;
	void _search_files_recursive(const String &p_path, const String &p_pattern, const String &p_mode, bool p_recursive, int p_max_results, Array &p_matches) const;
};

#endif // FILE_TOOLS_H
