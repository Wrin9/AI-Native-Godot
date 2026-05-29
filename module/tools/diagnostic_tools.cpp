/**
 * diagnostic_tools.cpp - 诊断工具实现
 *
 * 控制台日志、性能快照、场景复杂度分析等工具。
 * 从 funplay_core_tools.gd 的诊断相关方法迁移而来。
 */

#include "diagnostic_tools.h"

#include "editor/editor_interface.h"
#include "editor/file_system/editor_file_system.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/gui/control.h"
#include "core/io/dir_access.h"
#include "core/version.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"

// 场景文件扩展名
static const char *SCENE_EXTENSIONS[] = { ".tscn", ".scn", nullptr };
// 脚本文件扩展名
static const char *SCRIPT_EXTENSIONS[] = { ".gd", ".cs", ".gdshader", ".shader", nullptr };
// 文本文件扩展名
static const char *TEXT_EXTENSIONS[] = {
	".gd", ".gdshader", ".tres", ".tscn", ".json", ".txt", ".md",
	".cfg", ".ini", ".toml", ".yaml", ".yml", ".shader", ".cs",
	nullptr
};

// ============================================================
void DiagnosticTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 获取控制台日志
// ============================================================
String DiagnosticTools::_to_absolute(const String &p_path) const {
	if (p_path.begins_with("res://") || p_path.begins_with("user://")) {
		return ProjectSettings::get_singleton()->globalize_path(p_path);
	}
	return p_path;
}

String DiagnosticTools::get_console_logs(const Dictionary &p_args) {
	int max_lines = CLAMP(int(p_args.get("max_lines", 200)), 10, 4000);
	bool include_rotated = p_args.get("include_rotated", true);
	String filter_text = String(p_args.get("filter", "")).strip_edges();
	String severity = String(p_args.get("severity", "all")).to_lower();

	// 获取日志文件路径
	String configured_path = String(ProjectSettings::get_singleton()->get_setting("debug/file_logging/log_path", "user://logs/godot.log"));
	String current_log_path = ProjectSettings::get_singleton()->globalize_path(configured_path);
	String log_dir = current_log_path.get_base_dir();

	Array log_files;
	if (DirAccess::dir_exists_absolute(log_dir)) {
		PackedStringArray file_list = DirAccess::get_files_at(log_dir);
		String base_name = current_log_path.get_file();
		for (int i = 0; i < file_list.size(); i++) {
			if (!file_list[i].begins_with(base_name)) {
				continue;
			}
			if (!include_rotated && file_list[i] != base_name) {
				continue;
			}
			log_files.push_back(log_dir.path_join(file_list[i]));
		}
	}

	if (log_files.is_empty()) {
		return R"json({"error": "No log files found. File logging may be disabled."})json";
	}

	// 读取最新的日志文件
	String selected_file = log_files[log_files.size() - 1];
	String file_text = FileAccess::get_file_as_string(_to_absolute(selected_file));
	Vector<String> lines = file_text.split("\n");

	// 过滤日志行
	Vector<String> filtered_lines;
	for (int i = 0; i < lines.size(); i++) {
		String line = lines[i];
		String normalized = line.to_lower();

		// 严重程度过滤
		if (severity == "error") {
			if (!normalized.contains("error") && !normalized.contains("err:")) {
				continue;
			}
		} else if (severity == "warning") {
			if (!normalized.contains("warning") && !normalized.contains("warn:")) {
				continue;
			}
		} else if (severity == "info") {
			if (normalized.contains("error") || normalized.contains("warning")) {
				continue;
			}
		}

		// 文本过滤
		if (!filter_text.is_empty() && !normalized.contains(filter_text.to_lower())) {
			continue;
		}

		filtered_lines.push_back(line);
	}

	// 取末尾 max_lines 行
	int start_index = MAX(filtered_lines.size() - max_lines, 0);
	Array tail;
	for (int i = start_index; i < filtered_lines.size(); i++) {
		tail.push_back(filtered_lines[i]);
	}

	Dictionary result;
	result["log_path"] = selected_file;
	result["available_logs"] = log_files;
	result["line_count"] = tail.size();
	result["lines"] = tail;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 获取性能快照
// ============================================================
String DiagnosticTools::get_performance_snapshot(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	Node *scene_root = editor->get_edited_scene_root();

	Dictionary result;
	result["is_playing_scene"] = editor->is_playing_scene();
	result["frames_per_second"] = Engine::get_singleton()->get_frames_per_second();
	result["time_scale"] = Engine::get_singleton()->get_time_scale();
	result["open_scene_count"] = editor->get_open_scenes().size();
	result["scene_node_count"] = _count_nodes(scene_root);

	// 视口尺寸
	SubViewport *viewport_2d = editor->get_editor_viewport_2d();
	if (viewport_2d) {
		Dictionary size_2d;
		size_2d["x"] = viewport_2d->get_visible_rect().size.x;
		size_2d["y"] = viewport_2d->get_visible_rect().size.y;
		result["viewport_2d_size"] = size_2d;
	}

	SubViewport *viewport_3d = editor->get_editor_viewport_3d(0);
	if (viewport_3d) {
		Dictionary size_3d;
		size_3d["x"] = viewport_3d->get_visible_rect().size.x;
		size_3d["y"] = viewport_3d->get_visible_rect().size.y;
		result["viewport_3d_size"] = size_3d;
	}

	return JSON::stringify(result, "\t");
}

// ============================================================
// 分析场景复杂度
// ============================================================
String DiagnosticTools::analyze_scene_complexity(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	Node *scene_root = editor->get_edited_scene_root();
	if (!scene_root) {
		return R"json({"error": "No scene is currently open in the editor."})json";
	}

	Dictionary stats;
	stats["total_nodes"] = 0;
	stats["max_depth"] = 0;
	stats["node_2d_count"] = 0;
	stats["node_3d_count"] = 0;
	stats["control_count"] = 0;
	stats["scripted_nodes"] = 0;
	stats["light_count"] = 0;
	stats["camera_count"] = 0;
	stats["collision_count"] = 0;
	stats["audio_count"] = 0;
	stats["particles_count"] = 0;

	Dictionary unique_classes;
	stats["unique_classes"] = unique_classes;

	_analyze_node_recursive(scene_root, 0, stats);

	int unique_class_count = 0;
	Dictionary uc = stats["unique_classes"];
	Array uc_keys = uc.keys();
	unique_class_count = uc_keys.size();
	stats.erase("unique_classes");
	stats["unique_class_count"] = unique_class_count;

	// 计算复杂度评分
	int complexity_score = (int)stats["total_nodes"]
			+ (int)stats["scripted_nodes"] * 2
			+ (int)stats["light_count"] * 4
			+ (int)stats["camera_count"] * 2
			+ (int)stats["particles_count"] * 5;
	stats["complexity_score"] = complexity_score;

	return JSON::stringify(stats, "\t");
}

// ============================================================
// 获取项目信息
// ============================================================
String DiagnosticTools::get_project_info(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();

	Dictionary info;
	info["project_name"] = String(ProjectSettings::get_singleton()->get_setting("application/config/name", ""));
	info["project_root"] = ProjectSettings::get_singleton()->globalize_path("res://");

	Dictionary godot_version;
	godot_version["major"] = VERSION_MAJOR;
	godot_version["minor"] = VERSION_MINOR;
	godot_version["patch"] = VERSION_PATCH;
	info["godot_version"] = godot_version;

	if (editor) {
		Node *root = editor->get_edited_scene_root();
		if (root) {
			Dictionary root_info;
			root_info["name"] = root->get_name();
			root_info["type"] = root->get_class();
			root_info["path"] = String(root->get_path());
			info["current_scene_root"] = root_info;
		} else {
			info["current_scene_root"] = Variant();
		}
		info["current_scene_path"] = editor->get_current_path();
		info["open_scenes"] = editor->get_open_scenes();
		info["open_scene_count"] = editor->get_open_scenes().size();
		info["is_playing_scene"] = editor->is_playing_scene();
	}

	info["time_scale"] = Engine::get_singleton()->get_time_scale();

	return JSON::stringify(info, "\t");
}

// ============================================================
// 映射项目结构
// ============================================================
String DiagnosticTools::map_project(const Dictionary &p_args) {
	String output_format = String(p_args.get("format", "json")).strip_edges().to_lower();
	if (output_format != "json" && output_format != "html") {
		return R"json({"error": "'format' must be 'json' or 'html'."})json";
	}

	bool include_scripts = p_args.get("include_scripts", true);
	int max_files = CLAMP(int(p_args.get("max_files", 300)), 10, 2000);

	EditorInterface *editor = EditorInterface::get_singleton();

	// 收集场景文件
	Vector<String> scene_exts;
	scene_exts.push_back(".tscn");
	scene_exts.push_back(".scn");
	Array scene_paths;
	_collect_matching_files(_to_absolute("res://"), true, max_files, scene_paths, scene_exts);

	Array scenes;
	for (int i = 0; i < scene_paths.size(); i++) {
		Dictionary scene_entry;
		scene_entry["path"] = scene_paths[i];
		scenes.push_back(scene_entry);
	}

	// 收集脚本
	Array scripts;
	if (include_scripts) {
		Vector<String> script_exts;
		script_exts.push_back(".gd");
		script_exts.push_back(".cs");
		script_exts.push_back(".gdshader");
		script_exts.push_back(".shader");
		Array script_paths;
		_collect_matching_files(_to_absolute("res://"), true, max_files, script_paths, script_exts);
		for (int i = 0; i < script_paths.size(); i++) {
			Dictionary script_entry;
			script_entry["path"] = script_paths[i];
			scripts.push_back(script_entry);
		}
	}

	Dictionary project_map;
	Dictionary project_info;
	project_info["name"] = String(ProjectSettings::get_singleton()->get_setting("application/config/name", ""));
	project_info["root"] = ProjectSettings::get_singleton()->globalize_path("res://");
	project_info["main_scene"] = String(ProjectSettings::get_singleton()->get_setting("application/run/main_scene", ""));
	if (editor) {
		project_info["current_scene_path"] = editor->get_current_path();
		project_info["open_scenes"] = editor->get_open_scenes();
	}
	project_map["project"] = project_info;

	Dictionary counts;
	counts["scenes"] = scenes.size();
	counts["scripts"] = scripts.size();
	project_map["counts"] = counts;

	project_map["scenes"] = scenes;
	project_map["scripts"] = scripts;

	return JSON::stringify(project_map, "\t");
}

// ============================================================
void DiagnosticTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &DiagnosticTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("get_console_logs", "args"), &DiagnosticTools::get_console_logs, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_performance_snapshot", "args"), &DiagnosticTools::get_performance_snapshot, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("analyze_scene_complexity", "args"), &DiagnosticTools::analyze_scene_complexity, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_project_info", "args"), &DiagnosticTools::get_project_info, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("map_project", "args"), &DiagnosticTools::map_project, DEFVAL(Dictionary()));
}

// ============================================================
// 内部辅助方法
// ============================================================

int DiagnosticTools::_count_nodes(Node *p_node) const {
	if (!p_node) {
		return 0;
	}
	int total = 1;
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			total += _count_nodes(child);
		}
	}
	return total;
}

void DiagnosticTools::_analyze_node_recursive(Node *p_node, int p_depth, Dictionary &p_stats) const {
	int total = p_stats["total_nodes"];
	p_stats["total_nodes"] = total + 1;

	int max_d = p_stats["max_depth"];
	if (p_depth > max_d) {
		p_stats["max_depth"] = p_depth;
	}

	String class_name = p_node->get_class();
	Dictionary unique_classes = p_stats["unique_classes"];
	unique_classes[class_name] = true;
	p_stats["unique_classes"] = unique_classes;

	// 类型统计
	if (Object::cast_to<Node2D>(p_node)) {
		p_stats["node_2d_count"] = (int)p_stats["node_2d_count"] + 1;
	}
	if (Object::cast_to<Node3D>(p_node)) {
		p_stats["node_3d_count"] = (int)p_stats["node_3d_count"] + 1;
	}
	if (Object::cast_to<Control>(p_node)) {
		p_stats["control_count"] = (int)p_stats["control_count"] + 1;
	}
	if (p_node->get_script().get_type() != Variant::NIL) {
		p_stats["scripted_nodes"] = (int)p_stats["scripted_nodes"] + 1;
	}

	// 按类名关键词统计
	if (class_name.contains("Light")) {
		p_stats["light_count"] = (int)p_stats["light_count"] + 1;
	}
	if (class_name.contains("Camera")) {
		p_stats["camera_count"] = (int)p_stats["camera_count"] + 1;
	}
	if (class_name.contains("Collision")) {
		p_stats["collision_count"] = (int)p_stats["collision_count"] + 1;
	}
	if (class_name.contains("Audio")) {
		p_stats["audio_count"] = (int)p_stats["audio_count"] + 1;
	}
	if (class_name.contains("Particles")) {
		p_stats["particles_count"] = (int)p_stats["particles_count"] + 1;
	}

	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_analyze_node_recursive(child, p_depth + 1, p_stats);
		}
	}
}

void DiagnosticTools::_collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const {
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
