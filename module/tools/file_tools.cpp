/**
 * file_tools.cpp - 文件操作工具实现
 *
 * 项目文件的读取、写入、搜索等操作工具。
 * 从 funplay_core_tools.gd 的文件相关方法迁移而来。
 */

#include "file_tools.h"

#include "editor/editor_interface.h"
#include "editor/file_system/editor_file_system.h"
#include "editor/plugins/editor_plugin.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"

// 文本文件扩展名列表
static const char *TEXT_EXTENSIONS[] = {
	".gd", ".gdshader", ".tres", ".tscn", ".json", ".txt", ".md",
	".cfg", ".ini", ".toml", ".yaml", ".yml", ".shader", ".cs",
	".xml", ".html", ".css", ".js", ".ts", nullptr
};

// ============================================================
void FileTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 列出文件
// ============================================================
String FileTools::_to_absolute(const String &p_path) const {
	if (p_path.begins_with("res://") || p_path.begins_with("user://")) {
		return ProjectSettings::get_singleton()->globalize_path(p_path);
	}
	return p_path;
}

String FileTools::list_files(const Dictionary &p_args) {
	String root_path = _normalize_path(p_args.get("path", "res://"));
	bool recursive = p_args.get("recursive", true);
	bool include_hidden = p_args.get("include_hidden", false);
	int max_entries = CLAMP(int(p_args.get("max_entries", 200)), 1, 4000);

	Ref<DirAccess> dir = DirAccess::open(_to_absolute(root_path));
	if (dir.is_null()) {
		return vformat(R"json({"error": "Directory not found: %s"})json", root_path);
	}

	Array results;
	_collect_files(_to_absolute(root_path), recursive, include_hidden, max_entries, results);

	Dictionary result;
	result["path"] = root_path;
	result["count"] = results.size();
	result["entries"] = results;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 搜索文件
// ============================================================
String FileTools::search_files(const Dictionary &p_args) {
	String root_path = _normalize_path(p_args.get("path", "res://"));
	String pattern = String(p_args.get("pattern", "")).strip_edges();
	if (pattern.is_empty()) {
		return R"json({"error": "'pattern' is required."})json";
	}

	String mode = String(p_args.get("mode", "path")).to_lower();
	bool recursive = p_args.get("recursive", true);
	int max_results = CLAMP(int(p_args.get("max_results", 100)), 1, 2000);

	Array matches;
	_search_files_recursive(_to_absolute(root_path), pattern, mode, recursive, max_results, matches);

	Dictionary result;
	result["path"] = root_path;
	result["pattern"] = pattern;
	result["mode"] = mode;
	result["count"] = matches.size();
	result["matches"] = matches;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 检查文件/目录是否存在
// ============================================================
String FileTools::file_exists(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	bool exists = FileAccess::exists(path) ||
			DirAccess::dir_exists_absolute(path) ||
			ResourceLoader::exists(path);

	Dictionary result;
	result["path"] = path;
	result["exists"] = exists;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 读取文件
// ============================================================
String FileTools::read_file(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}
	if (!FileAccess::exists(path)) {
		return vformat(R"json({"error": "File not found: %s"})json", path);
	}

	int max_chars = CLAMP(int(p_args.get("max_chars", 12000)), 200, 500000);

	Ref<FileAccess> file = FileAccess::open(_to_absolute(path), FileAccess::READ);
	if (file.is_null()) {
		return vformat(R"json({"error": "Failed to open file: %s"})json", path);
	}

	String text = file->get_as_utf8_string();
	if (text.length() > max_chars) {
		text = text.substr(0, max_chars) + "\n...[truncated]";
	}

	Dictionary result;
	result["path"] = path;
	result["content"] = text;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 写入文件
// ============================================================
String FileTools::write_file(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		return vformat(R"json({"error": "Failed to create parent directory for %s"})json", path);
	}

	String content = p_args.get("content", "");

	Ref<FileAccess> file = FileAccess::open(_to_absolute(path), FileAccess::WRITE);
	if (file.is_null()) {
		return vformat(R"json({"error": "Failed to open file for writing: %s"})json", path);
	}

	file->store_string(content);
	_refresh_filesystem();

	Dictionary result;
	result["path"] = path;
	result["bytes_written"] = content.utf8().size();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 删除文件
// ============================================================
String FileTools::delete_file(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	Error err = DirAccess::remove_absolute(path);
	if (err != OK) {
		return vformat(R"json({"error": "Failed to delete '%s' (code %d)."})json", path, (int)err);
	}

	_refresh_filesystem();
	return vformat(R"json({"deleted": "%s"})json", path);
}

// ============================================================
// 移动/重命名文件
// ============================================================
String FileTools::move_file(const Dictionary &p_args) {
	String from_path = _normalize_path(p_args.get("from_path", ""));
	String to_path = _normalize_path(p_args.get("to_path", ""));
	if (from_path.is_empty() || to_path.is_empty()) {
		return R"json({"error": "'from_path' and 'to_path' are required."})json";
	}

	String ensure_err = _ensure_parent_dir(to_path);
	if (!ensure_err.is_empty()) {
		return vformat(R"json({"error": "Failed to create parent directory for %s"})json", to_path);
	}

	Error err = DirAccess::rename_absolute(from_path, to_path);
	if (err != OK) {
		return vformat(R"json({"error": "Failed to move '%s' to '%s' (code %d)."})json", from_path, to_path, (int)err);
	}

	_refresh_filesystem();
	Dictionary result;
	result["from"] = from_path;
	result["to"] = to_path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 复制文件
// ============================================================
String FileTools::copy_file(const Dictionary &p_args) {
	String from_path = _normalize_path(p_args.get("from_path", ""));
	String to_path = _normalize_path(p_args.get("to_path", ""));
	if (from_path.is_empty() || to_path.is_empty()) {
		return R"json({"error": "'from_path' and 'to_path' are required."})json";
	}

	String ensure_err = _ensure_parent_dir(to_path);
	if (!ensure_err.is_empty()) {
		return vformat(R"json({"error": "Failed to create parent directory for %s"})json", to_path);
	}

	Error err = DirAccess::copy_absolute(from_path, to_path);
	if (err != OK) {
		return vformat(R"json({"error": "Failed to copy '%s' to '%s' (code %d)."})json", from_path, to_path, (int)err);
	}

	_refresh_filesystem();
	Dictionary result;
	result["from"] = from_path;
	result["to"] = to_path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 查找符号引用
// ============================================================
String FileTools::find_usages(const Dictionary &p_args) {
	String symbol = String(p_args.get("symbol", "")).strip_edges();
	if (symbol.is_empty()) {
		return R"json({"error": "'symbol' is required."})json";
	}

	String root_path = _normalize_path(p_args.get("path", "res://"));
	bool case_sensitive = p_args.get("case_sensitive", true);
	int max_results = CLAMP(int(p_args.get("max_results", 200)), 1, 2000);

	// 收集文本文件
	Vector<String> text_exts;
	for (int i = 0; TEXT_EXTENSIONS[i] != nullptr; i++) {
		text_exts.push_back(String(TEXT_EXTENSIONS[i]));
	}

	Array files;
	if (FileAccess::exists(root_path)) {
		if (_matches_extension(root_path, text_exts)) {
			files.push_back(root_path);
		}
	} else {
		_collect_matching_files(_to_absolute(root_path), true, 5000, files, text_exts);
	}

	String needle = case_sensitive ? symbol : symbol.to_lower();
	Array matches;

	for (int f = 0; f < files.size() && matches.size() < (size_t)max_results; f++) {
		String file_path = files[f];
		Ref<FileAccess> file = FileAccess::open(_to_absolute(file_path), FileAccess::READ);
		if (file.is_null()) {
			continue;
		}

		String content = file->get_as_utf8_string();
		Vector<String> lines = content.split("\n");

		for (int line_idx = 0; line_idx < lines.size() && matches.size() < (size_t)max_results; line_idx++) {
			String line = lines[line_idx];
			String haystack = case_sensitive ? line : line.to_lower();
			int start_index = 0;

			while (start_index < haystack.length()) {
				int column_index = haystack.find(needle, start_index);
				if (column_index == -1) {
					break;
				}

				Dictionary match_entry;
				match_entry["path"] = file_path;
				match_entry["line"] = line_idx + 1;
				match_entry["column"] = column_index + 1;
				String snippet = line.strip_edges();
				match_entry["snippet"] = snippet.substr(0, MIN(snippet.length(), 240));
				matches.push_back(match_entry);

				start_index = column_index + MAX(needle.length(), 1);
			}
		}
	}

	Dictionary result;
	result["symbol"] = symbol;
	result["path"] = root_path;
	result["case_sensitive"] = case_sensitive;
	result["count"] = matches.size();
	result["truncated"] = matches.size() >= (size_t)max_results;
	result["matches"] = matches;
	return JSON::stringify(result, "\t");
}

// ============================================================
void FileTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &FileTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("list_files", "args"), &FileTools::list_files, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("search_files", "args"), &FileTools::search_files);
	ClassDB::bind_method(D_METHOD("file_exists", "args"), &FileTools::file_exists);
	ClassDB::bind_method(D_METHOD("read_file", "args"), &FileTools::read_file);
	ClassDB::bind_method(D_METHOD("write_file", "args"), &FileTools::write_file);
	ClassDB::bind_method(D_METHOD("delete_file", "args"), &FileTools::delete_file);
	ClassDB::bind_method(D_METHOD("move_file", "args"), &FileTools::move_file);
	ClassDB::bind_method(D_METHOD("copy_file", "args"), &FileTools::copy_file);
	ClassDB::bind_method(D_METHOD("find_usages", "args"), &FileTools::find_usages);
}

// ============================================================
// 内部辅助方法
// ============================================================

String FileTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

String FileTools::_ensure_parent_dir(const String &p_path) const {
	String parent_dir = p_path.get_base_dir();
	if (parent_dir.is_empty() || parent_dir == "res://" || parent_dir == "user://") {
		return "";
	}
	Error err = DirAccess::make_dir_recursive_absolute(parent_dir);
	return (err != OK) ? vformat("Error: Failed to create directory %s", parent_dir) : "";
}

void FileTools::_refresh_filesystem() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}
}

void FileTools::_collect_files(const String &p_path, bool p_recursive, bool p_include_hidden, int p_max_entries, Array &p_results) const {
	if (p_results.size() >= p_max_entries) {
		return;
	}

	Ref<DirAccess> dir = DirAccess::open(_to_absolute(p_path));
	if (dir.is_null()) {
		return;
	}

	dir->list_dir_begin();
	String item = dir->get_next();
	while (!item.is_empty()) {
		if (item == "." || item == "..") {
			item = dir->get_next();
			continue;
		}
		if (p_results.size() >= p_max_entries) {
			break;
		}
		if (!p_include_hidden && item.begins_with(".")) {
			item = dir->get_next();
			continue;
		}

		String child_path = p_path.path_join(item);
		if (dir->current_is_dir()) {
			Dictionary entry;
			entry["path"] = child_path;
			entry["type"] = "dir";
			p_results.push_back(entry);
			if (p_recursive) {
				_collect_files(child_path, p_recursive, p_include_hidden, p_max_entries, p_results);
			}
		} else {
			Dictionary entry;
			entry["path"] = child_path;
			entry["type"] = "file";
			p_results.push_back(entry);
		}
		item = dir->get_next();
	}
	dir->list_dir_end();
}

void FileTools::_collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const {
	if (p_results.size() >= p_max_entries) {
		return;
	}

	Ref<DirAccess> dir = DirAccess::open(_to_absolute(p_path));
	if (dir.is_null()) {
		return;
	}

	dir->list_dir_begin();
	String item = dir->get_next();
	while (!item.is_empty()) {
		if (item == "." || item == "..") {
			item = dir->get_next();
			continue;
		}
		if (p_results.size() >= p_max_entries) {
			break;
		}
		String child_path = p_path.path_join(item);
		if (dir->current_is_dir()) {
			if (p_recursive) {
				_collect_matching_files(child_path, p_recursive, p_max_entries, p_results, p_extensions);
			}
		} else if (_matches_extension(child_path, p_extensions)) {
			p_results.push_back(child_path);
		}
		item = dir->get_next();
	}
	dir->list_dir_end();
}

bool FileTools::_matches_extension(const String &p_path, const Vector<String> &p_extensions) const {
	String lower = p_path.to_lower();
	for (int i = 0; i < p_extensions.size(); i++) {
		if (lower.ends_with(p_extensions[i].to_lower())) {
			return true;
		}
	}
	return false;
}

void FileTools::_search_files_recursive(const String &p_path, const String &p_pattern, const String &p_mode, bool p_recursive, int p_max_results, Array &p_matches) const {
	if (p_matches.size() >= (size_t)p_max_results) {
		return;
	}

	Ref<DirAccess> dir = DirAccess::open(_to_absolute(p_path));
	if (dir.is_null()) {
		return;
	}

	Vector<String> text_exts;
	for (int i = 0; TEXT_EXTENSIONS[i] != nullptr; i++) {
		text_exts.push_back(String(TEXT_EXTENSIONS[i]));
	}

	String pattern_lower = p_pattern.to_lower();

	dir->list_dir_begin();
	String item = dir->get_next();
	while (!item.is_empty()) {
		if (item == "." || item == "..") {
			item = dir->get_next();
			continue;
		}
		if (p_matches.size() >= (size_t)p_max_results) {
			break;
		}

		String child_path = p_path.path_join(item);
		if (dir->current_is_dir()) {
			if (p_recursive) {
				_search_files_recursive(child_path, p_pattern, p_mode, p_recursive, p_max_results, p_matches);
			}
		} else {
			bool path_match = child_path.to_lower().contains(pattern_lower);
			bool content_match = false;

			if ((p_mode == "content" || p_mode == "both") && _matches_extension(child_path, text_exts)) {
				Ref<FileAccess> file = FileAccess::open(child_path, FileAccess::READ);
				if (file.is_valid()) {
					content_match = file->get_as_utf8_string().contains(p_pattern);
				}
			}

			bool is_match = false;
			if (p_mode == "path" && path_match) {
				is_match = true;
			} else if (p_mode == "content" && content_match) {
				is_match = true;
			} else if (p_mode == "both" && (path_match || content_match)) {
				is_match = true;
			}

			if (is_match) {
				Dictionary match_entry;
				match_entry["path"] = child_path;
				match_entry["path_match"] = path_match;
				match_entry["content_match"] = content_match;
				p_matches.push_back(match_entry);
			}
		}
		item = dir->get_next();
	}
	dir->list_dir_end();
}
