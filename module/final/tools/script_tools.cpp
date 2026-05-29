/**
 * script_tools.cpp - 脚本工具实现
 *
 * 脚本创建、编辑、验证等工具。
 * 从 funplay_core_tools.gd 的脚本相关方法迁移而来。
 */

#include "script_tools.h"

#include "editor/editor_node.h"
#include "editor/editor_interface.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/editor_file_system.h"
#include "scene/resources/script.h"
#include "scene/resources/gdscript.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"

// ============================================================
void ScriptTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建脚本
// ============================================================
String ScriptTools::create_script(const Dictionary &p_args) {
	String requested_path = p_args.get("path", "");
	String requested_language = String(p_args.get("language", "auto")).to_lower();

	// 确定脚本语言
	String resolved_language = requested_language;
	if (resolved_language == "auto" || resolved_language == "gd") {
		resolved_language = "gdscript";
	}
	if (resolved_language == "csharp" || resolved_language == "cs") {
		resolved_language = "dotnet";
	}

	String path = _normalize_script_path(requested_path, resolved_language);
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	// C# 脚本使用不同的模板
	if (resolved_language == "dotnet") {
		String csharp_class_name = String(p_args.get("class_name", "")).strip_edges();
		if (csharp_class_name.is_empty()) {
			// 从文件名推断类名
			String file_name = path.get_file();
			csharp_class_name = _pascal_case(file_name.get_basename());
		}

		String namespace_name = String(p_args.get("namespace", "")).strip_edges();
		String base_class = String(p_args.get("extends", "Node")).strip_edges();
		String body = String(p_args.get("body", "")).strip_edges();
		bool use_tool = p_args.get("tool", false);
		bool use_partial = p_args.get("partial", true);

		String script_content;
		script_content += "using Godot;\n";
		if (bool(p_args.get("include_system", false))) {
			script_content += "using System;\n";
		}
		script_content += "\n";
		if (!namespace_name.is_empty()) {
			script_content += vformat("namespace %s;\n\n", namespace_name);
		}
		if (use_tool) {
			script_content += "[Tool]\n";
		}
		String partial_text = use_partial ? " partial" : "";
		script_content += vformat("public%s class %s : %s\n{\n", partial_text, csharp_class_name, base_class);
		if (!body.is_empty()) {
			// 在每行前加制表符
			Vector<String> body_lines = body.split("\n");
			for (int i = 0; i < body_lines.size(); i++) {
				if (body_lines[i].strip_edges().is_empty()) {
					script_content += "\n";
				} else {
					script_content += "\t" + body_lines[i] + "\n";
				}
			}
		} else {
			script_content += "\tpublic override void _Ready()\n\t{\n\t}\n";
		}
		script_content += "}\n";

		// 写入文件
		Dictionary write_args;
		write_args["path"] = path;
		write_args["content"] = script_content;
		String result = edit_script(write_args);

		if (bool(p_args.get("open_in_editor", true))) {
			Dictionary open_args;
			open_args["path"] = path;
			open_script(open_args);
		}
		return result;
	}

	// GDScript 创建
	String extends_name = String(p_args.get("extends", "Node")).strip_edges();
	String script_class_name = String(p_args.get("class_name", "")).strip_edges();
	String body = String(p_args.get("body", "")).strip_edges();
	bool use_tool = p_args.get("tool", false);

	String script_content;
	if (use_tool) {
		script_content += "@tool\n";
	}
	script_content += vformat("extends %s\n", extends_name);
	if (!script_class_name.is_empty()) {
		script_content += vformat("class_name %s\n", script_class_name);
	}
	script_content += "\n";
	if (!body.is_empty()) {
		script_content += body + "\n";
	} else {
		script_content += "func _ready() -> void:\n\tpass\n";
	}

	Dictionary write_args;
	write_args["path"] = path;
	write_args["content"] = script_content + "\n";
	String result = edit_script(write_args);

	if (bool(p_args.get("open_in_editor", true))) {
		Dictionary open_args;
		open_args["path"] = path;
		open_script(open_args);
	}
	return result;
}

// ============================================================
// 编辑脚本（覆盖内容）
// ============================================================
String ScriptTools::edit_script(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	String content = p_args.get("content", "");

	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		return vformat(R"({"error": "Failed to create parent directory for %s"})", path);
	}

	Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
	if (file.is_null()) {
		return vformat(R"({"error": "Failed to open file for writing: %s"})", path);
	}

	file->store_string(content);

	_refresh_filesystem();

	Dictionary result;
	result["path"] = path;
	result["bytes_written"] = content.utf8().size();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 补丁脚本
// ============================================================
String ScriptTools::patch_script(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}
	if (!FileAccess::file_exists(path)) {
		return vformat(R"({"error": "File not found: %s"})", path);
	}

	// 读取现有内容
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
	if (file.is_null()) {
		return vformat(R"({"error": "Failed to open file: %s"})", path);
	}
	String content = file->get_as_utf8_string();

	String find_text = p_args.get("find", "");
	String replace_text = p_args.get("replace", "");
	String prepend_text = p_args.get("prepend", "");
	String append_text = p_args.get("append", "");

	// 查找替换
	if (!find_text.is_empty()) {
		if (!content.contains(find_text)) {
			return vformat(R"({"error": "Patch text was not found in %s."})", path);
		}
		content = content.replace(find_text, replace_text);
	}

	// 头部追加
	if (!prepend_text.is_empty()) {
		content = prepend_text + content;
	}

	// 尾部追加
	if (!append_text.is_empty()) {
		content += append_text;
	}

	// 写回
	Dictionary write_args;
	write_args["path"] = path;
	write_args["content"] = content;
	return edit_script(write_args);
}

// ============================================================
// 列出项目脚本
// ============================================================
String ScriptTools::list_scripts(const Dictionary &p_args) {
	String root_path = _normalize_path(p_args.get("path", "res://"));
	int max_entries = CLAMP(int(p_args.get("max_entries", 300)), 1, 5000);
	bool recursive = p_args.get("recursive", true);
	String requested_language = String(p_args.get("language", "auto")).to_lower();

	// 确定语言
	String resolved_language = requested_language;
	if (resolved_language == "gd" || resolved_language == "auto") {
		resolved_language = "gdscript";
	}

	Array scripts;

	if (resolved_language == "gdscript" || resolved_language == "mixed") {
		Vector<String> gd_extensions;
		gd_extensions.push_back(".gd");
		Array gd_paths;
		_collect_matching_files(root_path, recursive, max_entries, gd_paths, gd_extensions);
		for (int i = 0; i < gd_paths.size(); i++) {
			Dictionary entry;
			entry["path"] = gd_paths[i];
			entry["language"] = "gdscript";
			scripts.push_back(entry);
		}
	}

	if (resolved_language == "dotnet" || resolved_language == "mixed") {
		Vector<String> cs_extensions;
		cs_extensions.push_back(".cs");
		Array cs_paths;
		_collect_matching_files(root_path, recursive, max_entries, cs_paths, cs_extensions);
		for (int i = 0; i < cs_paths.size(); i++) {
			Dictionary entry;
			entry["path"] = cs_paths[i];
			entry["language"] = "dotnet";
			scripts.push_back(entry);
		}
	}

	Dictionary result;
	result["path"] = root_path;
	result["language"] = resolved_language;
	result["count"] = scripts.size();
	result["scripts"] = scripts;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 在编辑器中打开脚本
// ============================================================
String ScriptTools::open_script(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	Ref<Resource> script_res = ResourceLoader::load(path);
	if (script_res.is_null()) {
		return vformat(R"({"error": "Script not found or invalid: %s"})", path);
	}

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"({"error": "Editor interface not available."})";
	}

	int line = int(p_args.get("line", -1));
	int column = int(p_args.get("column", 0));
	editor->edit_script(script_res, line, column, true);

	return vformat(R"({"opened": "%s"})", path);
}

// ============================================================
// 获取脚本错误
// ============================================================
String ScriptTools::get_script_errors(const Dictionary &p_args) {
	String root_path = _normalize_path(p_args.get("path", "res://"));
	int max_files = CLAMP(int(p_args.get("max_files", 200)), 1, 3000);

	Vector<String> gd_extensions;
	gd_extensions.push_back(".gd");
	Array gd_paths;
	_collect_matching_files(root_path, true, max_files, gd_paths, gd_extensions);

	Array results;
	int checked = 0;

	for (int i = 0; i < gd_paths.size(); i++) {
		String file_path = gd_paths[i];
		checked++;

		// 读取文件内容进行编译检查
		Ref<GDScript> script;
		script.instantiate();
		script->set_resource_path(file_path);

		Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::READ);
		if (file.is_null()) {
			continue;
		}
		script->set_source_code(file->get_as_utf8_string());

		Error err = script->reload();
		if (err != OK) {
			Dictionary error_entry;
			error_entry["path"] = file_path;
			error_entry["ok"] = false;
			error_entry["error_code"] = (int)err;
			error_entry["language"] = "gdscript";
			results.push_back(error_entry);
		}
	}

	Dictionary result;
	result["path"] = root_path;
	result["checked"] = checked;
	result["error_count"] = results.size();
	result["errors"] = results;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 验证脚本
// ============================================================
String ScriptTools::validate_script(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"({"error": "'path' is required."})";
	}

	if (!FileAccess::file_exists(path)) {
		return vformat(R"({"error": "File not found: %s"})", path);
	}

	String language = _guess_script_language(path);
	Dictionary result;

	if (language == "gdscript") {
		Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
		if (file.is_null()) {
			return vformat(R"({"error": "Failed to open file: %s"})", path);
		}

		Ref<GDScript> script;
		script.instantiate();
		script->set_resource_path(path);
		script->set_source_code(file->get_as_utf8_string());

		Error err = script->reload();
		result["path"] = path;
		result["ok"] = err == OK;
		result["error_code"] = (int)err;
		result["language"] = "gdscript";
	} else {
		result["path"] = path;
		result["ok"] = true;
		result["language"] = language;
		result["note"] = "C# validation requires dotnet build, not yet implemented in C++ module.";
	}

	return JSON::stringify(result, "\t");
}

// ============================================================
// 请求脚本重载
// ============================================================
String ScriptTools::request_script_reload(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));

	if (!path.is_empty()) {
		Ref<Resource> script_res = ResourceLoader::load(path);
		if (script_res.is_valid()) {
			Ref<Script> script = script_res;
			if (script.is_valid()) {
				Error err = script->reload();
				_refresh_filesystem();
				Dictionary result;
				result["path"] = path;
				result["reload_error"] = (int)err;
				return JSON::stringify(result, "\t");
			}
		}
	}

	_refresh_filesystem();
	return R"({"message": "Requested Godot resource filesystem rescan."})";
}

// ============================================================
void ScriptTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &ScriptTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("create_script", "args"), &ScriptTools::create_script);
	ClassDB::bind_method(D_METHOD("edit_script", "args"), &ScriptTools::edit_script);
	ClassDB::bind_method(D_METHOD("patch_script", "args"), &ScriptTools::patch_script);
	ClassDB::bind_method(D_METHOD("list_scripts", "args"), &ScriptTools::list_scripts, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("open_script", "args"), &ScriptTools::open_script);
	ClassDB::bind_method(D_METHOD("get_script_errors", "args"), &ScriptTools::get_script_errors, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("validate_script", "args"), &ScriptTools::validate_script);
	ClassDB::bind_method(D_METHOD("request_script_reload", "args"), &ScriptTools::request_script_reload, DEFVAL(Dictionary()));
}

// ============================================================
// 内部辅助方法
// ============================================================

String ScriptTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

String ScriptTools::_normalize_script_path(const String &p_path, const String &p_language) const {
	String normalized = _normalize_path(p_path);
	if (normalized.is_empty()) {
		return "";
	}
	if (normalized.to_lower().ends_with(".gd") || normalized.to_lower().ends_with(".cs")) {
		return normalized;
	}
	if (p_language == "dotnet") {
		return normalized + ".cs";
	}
	return normalized + ".gd";
}

String ScriptTools::_ensure_parent_dir(const String &p_path) const {
	String parent_dir = p_path.get_base_dir();
	if (parent_dir.is_empty() || parent_dir == "res://" || parent_dir == "user://") {
		return "";
	}
	Error err = DirAccess::make_dir_recursive_absolute(parent_dir);
	return (err != OK) ? vformat("Error: Failed to create directory %s", parent_dir) : "";
}

void ScriptTools::_refresh_filesystem() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}
}

void ScriptTools::_collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const {
	if (p_results.size() >= p_max_entries) {
		return;
	}

	Ref<DirAccess> dir = DirAccess::open(p_path);
	if (dir.is_null()) {
		return;
	}

	dir->list_dir_begin();
	String item = dir->get_next();
	while (!item.is_empty()) {
		if (p_results.size() >= p_max_entries) {
			break;
		}

		String child_path = p_path.path_join(item);
		if (dir->current_is_dir()) {
			if (p_recursive) {
				_collect_matching_files(child_path, p_recursive, p_max_entries, p_results, p_extensions);
			}
		} else {
			String lower = child_path.to_lower();
			for (int i = 0; i < p_extensions.size(); i++) {
				if (lower.ends_with(p_extensions[i].to_lower())) {
					p_results.push_back(child_path);
					break;
				}
			}
		}
		item = dir->get_next();
	}
	dir->list_dir_end();
}

String ScriptTools::_pascal_case(const String &p_value) const {
	Vector<String> parts = p_value.replace("-", "_").replace(" ", "_").split("_");
	String result;
	for (int i = 0; i < parts.size(); i++) {
		String text = parts[i].strip_edges();
		if (text.is_empty()) {
			continue;
		}
		result += text.left(1).to_upper() + text.substr(1);
	}
	return result.is_empty() ? "NewScript" : result;
}

String ScriptTools::_guess_script_language(const String &p_path) const {
	String lower = p_path.to_lower();
	if (lower.ends_with(".cs")) {
		return "dotnet";
	}
	if (lower.ends_with(".gdshader") || lower.ends_with(".shader")) {
		return "shader";
	}
	return "gdscript";
}
