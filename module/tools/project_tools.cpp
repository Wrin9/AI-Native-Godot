#include "core/version.h"
/**
 * project_tools.cpp - 项目工具实现
 *
 * 项目设置、Autoload、InputMap、Addon 管理工具。
 * 从 funplay_core_tools.gd 的项目相关方法迁移而来。
 */

#include "project_tools.h"

#include "editor/editor_interface.h"
#include "core/io/config_file.h"
#include "editor/plugins/editor_plugin.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"
#include "core/input/input_map.h"
#include "core/object/class_db.h"

void ProjectTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

String ProjectTools::_to_absolute(const String &p_path) const {
	if (p_path.begins_with("res://") || p_path.begins_with("user://")) {
		return ProjectSettings::get_singleton()->globalize_path(p_path);
	}
	return p_path;
}

String ProjectTools::get_project_info(const Dictionary &p_args) {
	Dictionary info;
	info["project_name"] = String(ProjectSettings::get_singleton()->get_setting("application/config/name", ""));
	info["project_root"] = ProjectSettings::get_singleton()->globalize_path("res://");
	info["main_scene"] = String(ProjectSettings::get_singleton()->get_setting("application/run/main_scene", ""));
	info["rendering_method"] = String(ProjectSettings::get_singleton()->get_setting("rendering/renderer/rendering_method", ""));

	Dictionary godot_version;
	godot_version["major"] = VERSION_MAJOR;
	godot_version["minor"] = VERSION_MINOR;
	godot_version["patch"] = VERSION_PATCH;
	info["godot_version"] = godot_version;

	Array input_actions;
	TypedArray<StringName> actions_arr = InputMap::get_singleton()->get_actions();
	for (int i = 0; i < actions_arr.size(); i++) {
		input_actions.push_back(String(actions_arr[i]));
	}
	info["input_actions"] = input_actions;
	info["autoloads"] = _list_autoloads_internal();

	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		info["is_playing_scene"] = editor->is_playing_scene();
		info["current_scene_path"] = editor->get_current_path();
	}
	return JSON::stringify(info, "\t");
}

String ProjectTools::list_project_settings(const Dictionary &p_args) {
	String prefix = String(p_args.get("prefix", "")).strip_edges();
	bool include_internal = p_args.get("include_internal", false);
	int max_results = CLAMP(int(p_args.get("max_results", 500)), 1, 5000);

	Array settings;
	TypedArray<Dictionary> prop_list = ProjectSettings::get_singleton()->call("get_property_list");
	for (int i = 0; i < prop_list.size() && settings.size() < (unsigned)max_results; i++) {
		Dictionary prop_info = prop_list[i];
		String name = String(prop_info.get("name", ""));

		if (!prefix.is_empty() && !name.begins_with(prefix)) {
			continue;
		}
		if (!include_internal && name.begins_with("_")) {
			continue;
		}

		Dictionary entry;
		entry["name"] = name;
		entry["value"] = ProjectSettings::get_singleton()->get_setting(name);
		entry["type"] = prop_info.get("type", 0);
		entry["hint"] = prop_info.get("hint", 0);
		entry["hint_string"] = prop_info.get("hint_string", "");
		settings.push_back(entry);
	}

	Dictionary result;
	result["prefix"] = prefix;
	result["count"] = settings.size();
	result["settings"] = settings;
	return JSON::stringify(result, "\t");
}

String ProjectTools::get_project_setting(const Dictionary &p_args) {
	String key = String(p_args.get("key", "")).strip_edges();
	if (key.is_empty()) {
		return R"json({"error": "'key' is required."})json";
	}
	if (!ProjectSettings::get_singleton()->has_setting(key)) {
		return vformat(R"json({"error": "Project setting not found: %s"})json", key);
	}
	Dictionary result;
	result["key"] = key;
	result["value"] = ProjectSettings::get_singleton()->get_setting(key);
	return JSON::stringify(result, "\t");
}

String ProjectTools::set_project_setting(const Dictionary &p_args) {
	String key = String(p_args.get("key", "")).strip_edges();
	if (key.is_empty()) {
		return R"json({"error": "'key' is required."})json";
	}
	if (!p_args.has("value")) {
		return R"json({"error": "'value' is required."})json";
	}

	ProjectSettings::get_singleton()->set_setting(key, p_args["value"]);
	bool save_changes = p_args.get("save", true);
	if (save_changes) {
		ProjectSettings::get_singleton()->save();
	}

	Dictionary result;
	result["key"] = key;
	result["value"] = ProjectSettings::get_singleton()->get_setting(key);
	result["saved"] = save_changes;
	return JSON::stringify(result, "\t");
}

String ProjectTools::list_project_features(const Dictionary &p_args) {
	Dictionary result;
	result["project_name"] = String(ProjectSettings::get_singleton()->get_setting("application/config/name", ""));
	result["main_scene"] = String(ProjectSettings::get_singleton()->get_setting("application/run/main_scene", ""));
	result["rendering_method"] = String(ProjectSettings::get_singleton()->get_setting("rendering/renderer/rendering_method", ""));

	Array input_actions;
	TypedArray<StringName> actions_arr = InputMap::get_singleton()->get_actions();
	for (int i = 0; i < actions_arr.size(); i++) {
		input_actions.push_back(String(actions_arr[i]));
	}
	result["input_actions"] = input_actions;
	result["autoloads"] = _list_autoloads_internal();
	return JSON::stringify(result, "\t");
}

String ProjectTools::list_addons(const Dictionary &p_args) {
	String addons_dir = "res://addons";
	Array addons;

	Ref<DirAccess> dir = DirAccess::open(_to_absolute(addons_dir));
	if (dir.is_null()) {
		Dictionary result;
		result["addons"] = addons;
		return JSON::stringify(result, "\t");
	}

	PackedStringArray directories = dir->get_directories();
	for (int i = 0; i < directories.size(); i++) {
		String addon_name = directories[i];
		String plugin_cfg_path = addons_dir.path_join(addon_name).path_join("plugin.cfg");

		Dictionary info;
		info["name"] = addon_name;
		info["path"] = addons_dir.path_join(addon_name);
		info["has_plugin_cfg"] = FileAccess::exists(plugin_cfg_path);

		// 检查是否启用
		bool is_enabled = false;
		PackedStringArray enabled_plugins = ProjectSettings::get_singleton()->get_setting("editor_plugins/enabled", PackedStringArray());
		for (int j = 0; j < enabled_plugins.size(); j++) {
			if (String(enabled_plugins[j]) == addon_name) {
				is_enabled = true;
				break;
			}
		}
		info["enabled"] = is_enabled;

		// 读取 plugin.cfg 元数据
		if (FileAccess::exists(plugin_cfg_path)) {
			Ref<ConfigFile> config;
			config.instantiate();
			Error cfg_err = config->load(plugin_cfg_path);
			if (cfg_err == OK) {
				info["display_name"] = String(config->get_value("plugin", "name", ""));
				info["description"] = String(config->get_value("plugin", "description", ""));
				info["author"] = String(config->get_value("plugin", "author", ""));
				info["version"] = String(config->get_value("plugin", "version", ""));
				info["script"] = String(config->get_value("plugin", "script", ""));
			}
		}
		addons.push_back(info);
	}

	Dictionary result;
	result["count"] = addons.size();
	result["addons"] = addons;
	return JSON::stringify(result, "\t");
}

String ProjectTools::set_addon_enabled(const Dictionary &p_args) {
	String addon_name = String(p_args.get("addon", "")).strip_edges();
	if (addon_name.is_empty()) {
		return R"json({"error": "'addon' is required."})json";
	}
	if (!p_args.has("enabled")) {
		return R"json({"error": "'enabled' is required."})json";
	}

	bool enabled = p_args["enabled"];
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor && editor->has_method("set_plugin_enabled")) {
		editor->set_plugin_enabled(addon_name, enabled);
	} else {
		PackedStringArray enabled_plugins = ProjectSettings::get_singleton()->get_setting("editor_plugins/enabled", PackedStringArray());
		if (enabled) {
			bool found = false;
			for (int i = 0; i < enabled_plugins.size(); i++) {
				if (String(enabled_plugins[i]) == addon_name) {
					found = true;
					break;
				}
			}
			if (!found) {
				enabled_plugins.push_back(addon_name);
			}
		} else {
			PackedStringArray filtered;
			for (int i = 0; i < enabled_plugins.size(); i++) {
				if (String(enabled_plugins[i]) != addon_name) {
					filtered.push_back(enabled_plugins[i]);
				}
			}
			enabled_plugins = filtered;
		}
		ProjectSettings::get_singleton()->set_setting("editor_plugins/enabled", enabled_plugins);
		ProjectSettings::get_singleton()->save();
	}

	Dictionary result;
	result["addon"] = addon_name;
	result["enabled"] = enabled;
	return JSON::stringify(result, "\t");
}

String ProjectTools::list_autoloads(const Dictionary &p_args) {
	Array autoloads = _list_autoloads_internal();
	Dictionary result;
	result["count"] = autoloads.size();
	result["autoloads"] = autoloads;
	return JSON::stringify(result, "\t");
}

String ProjectTools::set_autoload(const Dictionary &p_args) {
	String name = String(p_args.get("name", "")).strip_edges();
	String path = _normalize_path(p_args.get("path", ""));
	if (name.is_empty() || path.is_empty()) {
		return R"json({"error": "'name' and 'path' are required."})json";
	}

	String key = vformat("autoload/%s", name);
	String value = String(p_args.get("value", path));
	if (value.is_empty()) {
		value = path;
	}

	ProjectSettings::get_singleton()->set_setting(key, value);
	bool save_changes = p_args.get("save", true);
	if (save_changes) {
		ProjectSettings::get_singleton()->save();
	}

	Dictionary result;
	result["name"] = name;
	result["path"] = value;
	result["saved"] = save_changes;
	return JSON::stringify(result, "\t");
}

String ProjectTools::remove_autoload(const Dictionary &p_args) {
	String name = String(p_args.get("name", "")).strip_edges();
	if (name.is_empty()) {
		return R"json({"error": "'name' is required."})json";
	}

	String key = vformat("autoload/%s", name);
	if (!ProjectSettings::get_singleton()->has_setting(key)) {
		return vformat(R"json({"error": "Autoload not found: %s"})json", name);
	}

	ProjectSettings::get_singleton()->set_setting(key, Variant());
	bool save_changes = p_args.get("save", true);
	if (save_changes) {
		ProjectSettings::get_singleton()->save();
	}
	return vformat(R"json({"removed": "%s"})json", name);
}

String ProjectTools::list_input_actions(const Dictionary &p_args) {
	Array actions_list;
	TypedArray<StringName> actions_arr = InputMap::get_singleton()->get_actions();
	for (int i = 0; i < actions_arr.size(); i++) {
		actions_list.push_back(_build_input_action_info(String(actions_arr[i])));
	}

	Dictionary result;
	result["count"] = actions_list.size();
	result["actions"] = actions_list;
	return JSON::stringify(result, "\t");
}

String ProjectTools::get_input_action(const Dictionary &p_args) {
	String action_name = String(p_args.get("action", "")).strip_edges();
	if (action_name.is_empty()) {
		return R"json({"error": "'action' is required."})json";
	}
	if (!InputMap::get_singleton()->has_action(action_name)) {
		return vformat(R"json({"error": "Input action not found: %s"})json", action_name);
	}
	return JSON::stringify(_build_input_action_info(action_name), "\t");
}

String ProjectTools::add_input_action(const Dictionary &p_args) {
	String action_name = String(p_args.get("action", "")).strip_edges();
	if (action_name.is_empty()) {
		return R"json({"error": "'action' is required."})json";
	}

	if (!InputMap::get_singleton()->has_action(action_name)) {
		double deadzone = double(p_args.get("deadzone", 0.2));
		InputMap::get_singleton()->add_action(action_name, deadzone);
	}

	// 添加事件
	Variant events_var = p_args.get("events", Variant());
	if (events_var.get_type() == Variant::ARRAY) {
		Array events = events_var;
		for (int i = 0; i < events.size(); i++) {
			if (events[i].get_type() != Variant::DICTIONARY) {
				continue;
			}
			Dictionary event_data = events[i];
			String event_type = String(event_data.get("type", "")).strip_edges().to_lower();

			if (event_type == "key" || event_type == "inputeventkey") {
				Ref<InputEventKey> key_event;
				key_event.instantiate();
				key_event->set_pressed(bool(event_data.get("pressed", true)));

				int keycode = 0;
				Variant key_var = event_data.get("key", event_data.get("keycode", Variant()));
				if (key_var.get_type() == Variant::INT) {
					keycode = int(key_var);
				} else if (key_var.get_type() == Variant::STRING) {
					String s = String(key_var);
					if (s.length() == 1) {
						keycode = (int)s[0];
					}
				}
				key_event->set_keycode((Key)keycode);

				if (event_data.has("physical_key")) {
					Variant pk = event_data["physical_key"];
					if (pk.get_type() == Variant::INT) {
						key_event->set_physical_keycode((Key)int(pk));
					}
				}
				InputMap::get_singleton()->action_add_event(action_name, key_event);
			}
		}
	}

	bool save_changes = p_args.get("save", true);
	if (save_changes) {
		ProjectSettings::get_singleton()->save();
	}
	return JSON::stringify(_build_input_action_info(action_name), "\t");
}

String ProjectTools::remove_input_action(const Dictionary &p_args) {
	String action_name = String(p_args.get("action", "")).strip_edges();
	if (action_name.is_empty()) {
		return R"json({"error": "'action' is required."})json";
	}
	if (!InputMap::get_singleton()->has_action(action_name)) {
		return vformat(R"json({"error": "Input action not found: %s"})json", action_name);
	}

	InputMap::get_singleton()->erase_action(action_name);
	bool save_changes = p_args.get("save", true);
	if (save_changes) {
		ProjectSettings::get_singleton()->save();
	}
	return vformat(R"json({"removed": "%s"})json", action_name);
}

void ProjectTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &ProjectTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("get_project_info", "args"), &ProjectTools::get_project_info, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_project_settings", "args"), &ProjectTools::list_project_settings, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_project_setting", "args"), &ProjectTools::get_project_setting);
	ClassDB::bind_method(D_METHOD("set_project_setting", "args"), &ProjectTools::set_project_setting);
	ClassDB::bind_method(D_METHOD("list_project_features", "args"), &ProjectTools::list_project_features, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_addons", "args"), &ProjectTools::list_addons, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_addon_enabled", "args"), &ProjectTools::set_addon_enabled);
	ClassDB::bind_method(D_METHOD("list_autoloads", "args"), &ProjectTools::list_autoloads, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_autoload", "args"), &ProjectTools::set_autoload);
	ClassDB::bind_method(D_METHOD("remove_autoload", "args"), &ProjectTools::remove_autoload);
	ClassDB::bind_method(D_METHOD("list_input_actions", "args"), &ProjectTools::list_input_actions, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_input_action", "args"), &ProjectTools::get_input_action);
	ClassDB::bind_method(D_METHOD("add_input_action", "args"), &ProjectTools::add_input_action);
	ClassDB::bind_method(D_METHOD("remove_input_action", "args"), &ProjectTools::remove_input_action);
}

// ============================================================
// 内部辅助方法
// ============================================================

Array ProjectTools::_list_autoloads_internal() const {
	Array autoloads;
	TypedArray<Dictionary> prop_list = ProjectSettings::get_singleton()->call("get_property_list");
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		String name = String(prop_info.get("name", ""));
		if (!name.begins_with("autoload/")) {
			continue;
		}
		Dictionary entry;
		entry["name"] = name.substr(9);
		entry["path"] = String(ProjectSettings::get_singleton()->get_setting(name, ""));
		autoloads.push_back(entry);
	}
	return autoloads;
}

Dictionary ProjectTools::_build_input_action_info(const String &p_action_name) const {
	Dictionary info;
	info["name"] = p_action_name;
	info["deadzone"] = InputMap::get_singleton()->action_get_deadzone(p_action_name);

	Array events;
	Array action_events = InputMap::get_singleton()->call("action_get_events", p_action_name);
	for (int i = 0; i < action_events.size(); i++) {
		Ref<InputEvent> event = action_events[i];
		if (event.is_valid()) {
			events.push_back(_serialize_input_event(event));
		}
	}
	info["event_count"] = events.size();
	info["events"] = events;
	return info;
}

Dictionary ProjectTools::_serialize_input_event(const Ref<InputEvent> &p_event) const {
	Dictionary data;
	data["type"] = p_event->get_class();
	data["as_text"] = p_event->as_text();

	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid()) {
		data["keycode"] = (int)key_event->get_keycode();
		data["physical_keycode"] = (int)key_event->get_physical_keycode();
		data["unicode"] = (int)key_event->get_unicode();
		data["pressed"] = key_event->is_pressed();
		data["echo"] = key_event->is_echo();
	}

	Ref<InputEventMouseButton> mouse_event = p_event;
	if (mouse_event.is_valid()) {
		data["button_index"] = (int)mouse_event->get_button_index();
		data["pressed"] = mouse_event->is_pressed();
		Dictionary pos;
		pos["x"] = mouse_event->get_position().x;
		pos["y"] = mouse_event->get_position().y;
		data["position"] = pos;
	}

	Ref<InputEventAction> action_event = p_event;
	if (action_event.is_valid()) {
		data["action"] = action_event->get_action();
		data["pressed"] = action_event->is_pressed();
		data["strength"] = action_event->get_strength();
	}

	return data;
}

String ProjectTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}
