/**
 * editor_tools.cpp - 编辑器工具实现
 */
#include "editor_tools.h"

#include "editor/editor_interface.h"
#include "editor/settings/editor_settings.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/editor_data.h"
#include "core/object/undo_redo.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/config/project_settings.h"
#include "mcp_tool_helpers.h"

void EditorTools::set_editor_plugin(EditorPlugin *p_plugin) { _plugin = p_plugin; }

// ============================================================
String EditorTools::undo(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) return R"json({"error": "Editor not available"})json";
	// 通过编辑器快捷键模拟 Ctrl+Z
	Dictionary result;
	result["action"] = "undo";
	// EditorUndoRedoManager 的 undo 需要通过 get_history_undo_redo 获取 UndoRedo
	EditorUndoRedoManager *urm = editor->get_editor_undo_redo();
	if (urm) {
		// 获取当前场景的 history 并执行 undo
		UndoRedo *ur = urm->get_history_undo_redo(EditorUndoRedoManager::GLOBAL_HISTORY);
		if (ur && ur->undo()) {
			result["status"] = "ok";
		} else {
			result["status"] = "nothing_to_undo";
		}
	} else {
		result["status"] = "no_undo_manager";
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
String EditorTools::redo(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) return R"json({"error": "Editor not available"})json";
	Dictionary result;
	result["action"] = "redo";
	EditorUndoRedoManager *urm = editor->get_editor_undo_redo();
	if (urm) {
		UndoRedo *ur = urm->get_history_undo_redo(EditorUndoRedoManager::GLOBAL_HISTORY);
		if (ur && ur->redo()) {
			result["status"] = "ok";
		} else {
			result["status"] = "nothing_to_redo";
		}
	} else {
		result["status"] = "no_undo_manager";
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
String EditorTools::run_code(const Dictionary &p_args) {
	String code = String(p_args.get("code", "")).strip_edges();
	if (code.is_empty()) {
		return R"json({"error": "'code' is required."})json";
	}
	String temp_path = "res://.mcp_temp_script.gd";
	String abs_path = mcp_to_absolute(temp_path);
	Ref<FileAccess> f = FileAccess::open(abs_path, FileAccess::WRITE);
	if (f.is_null()) {
		return R"json({"error": "Failed to create temp script file."})json";
	}
	f->store_string(code);
	f->close();

	Dictionary result;
	result["action"] = "run_code";
	result["temp_path"] = temp_path;

	// 语法检查
	ScriptLanguage *gdscript_lang = nullptr;
	int lang_count = ScriptServer::get_language_count();
	for (int i = 0; i < lang_count; i++) {
		ScriptLanguage *lang = ScriptServer::get_language(i);
		if (lang->get_name() == "GDScript") {
			gdscript_lang = lang;
			break;
		}
	}

	if (gdscript_lang) {
		List<String> errors;
		gdscript_lang->validate(code, temp_path, &errors);
		if (errors.size() > 0) {
			Array err_list;
			for (const String &e : errors) {
				err_list.append(e);
			}
			result["status"] = "syntax_error";
			result["errors"] = err_list;
		} else {
			result["status"] = "syntax_ok";
		}
	} else {
		result["status"] = "saved";
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
String EditorTools::get_editor_state(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) return R"json({"error": "Editor not available"})json";

	Dictionary result;
	result["editor_version"] = "Godot 4.6.3";

	Node *scene_root = editor->get_edited_scene_root();
	if (scene_root) {
		result["edited_scene"] = String(scene_root->get_name());
		result["edited_scene_path"] = scene_root->get_scene_file_path();
	}

	Array selected = editor->get_selection()->get_selected_nodes();
	Array sel_names;
	for (int i = 0; i < selected.size(); i++) {
		Node *n = Object::cast_to<Node>(selected[i]);
		if (n) sel_names.append(String(n->get_path()));
	}
	result["selected_nodes"] = sel_names;

	PackedStringArray open_scenes = editor->get_open_scenes();
	Array scenes_arr;
	for (int i = 0; i < open_scenes.size(); i++) scenes_arr.append(open_scenes[i]);
	result["open_scenes"] = scenes_arr;
	result["is_playing"] = editor->is_playing_scene();

	return JSON::stringify(result, "\t");
}

// ============================================================
String EditorTools::set_editor_setting(const Dictionary &p_args) {
	String key = String(p_args.get("key", "")).strip_edges();
	Variant value = p_args.get("value", Variant());
	if (key.is_empty()) {
		return R"json({"error": "'key' and 'value' are required."})json";
	}
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) return R"json({"error": "Editor not available"})json";
	Ref<EditorSettings> settings = editor->get_editor_settings();
	if (settings.is_null()) return R"json({"error": "Editor settings not available"})json";
	settings->set_setting(key, value);
	Dictionary result;
	result["key"] = key;
	result["value"] = value;
	result["status"] = "set";
	return JSON::stringify(result, "\t");
}

// ============================================================
void EditorTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("undo", "args"), &EditorTools::undo, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("redo", "args"), &EditorTools::redo, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("run_code", "args"), &EditorTools::run_code, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_editor_state", "args"), &EditorTools::get_editor_state, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_editor_setting", "args"), &EditorTools::set_editor_setting, DEFVAL(Dictionary()));
}
