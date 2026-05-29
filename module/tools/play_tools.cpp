/**
 * play_tools.cpp - 运行时工具实现
 *
 * 播放模式控制、输入模拟、时间缩放等工具。
 * 从 funplay_core_tools.gd 的运行时相关方法迁移而来。
 */

#include "play_tools.h"

#include "editor/editor_interface.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/image_texture.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/crypto/crypto_core.h"

// 按键名称映射
static HashMap<String, Key> _init_key_name_map() {
	HashMap<String, Key> map;
	map["enter"] = Key::ENTER;
	map["escape"] = Key::ESCAPE;
	map["esc"] = Key::ESCAPE;
	map["space"] = Key::SPACE;
	map["tab"] = Key::TAB;
	map["backspace"] = Key::BACKSPACE;
	map["up"] = Key::UP;
	map["down"] = Key::DOWN;
	map["left"] = Key::LEFT;
	map["right"] = Key::RIGHT;
	map["shift"] = Key::SHIFT;
	map["ctrl"] = Key::CTRL;
	map["control"] = Key::CTRL;
	map["alt"] = Key::ALT;
	return map;
}

static const HashMap<String, Key> KEY_NAME_MAP = _init_key_name_map();

// 鼠标按钮映射
static HashMap<String, MouseButton> _init_mouse_button_map() {
	HashMap<String, MouseButton> map;
	map["left"] = MouseButton::LEFT;
	map["right"] = MouseButton::RIGHT;
	map["middle"] = MouseButton::MIDDLE;
	map["wheel_up"] = MouseButton::WHEEL_UP;
	map["wheel_down"] = MouseButton::WHEEL_DOWN;
	return map;
}

static const HashMap<String, MouseButton> MOUSE_BUTTON_MAP = _init_mouse_button_map();

// ============================================================
void PlayTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 获取播放状态
// ============================================================
String PlayTools::get_play_state(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	Dictionary result;
	result["is_playing_scene"] = editor->is_playing_scene();
	result["current_scene_path"] = editor->get_current_path();
	result["open_scenes"] = editor->get_open_scenes();
	result["time_scale"] = Engine::get_singleton()->get_time_scale();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 进入播放模式
// ============================================================
String PlayTools::enter_play_mode(const Dictionary &p_args) {
	String mode = String(p_args.get("mode", "current")).to_lower();

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	if (mode == "current") {
		editor->play_current_scene();
	} else if (mode == "main") {
		editor->play_main_scene();
	} else if (mode == "custom") {
		String scene_path = _normalize_path(p_args.get("scene_path", ""));
		if (scene_path.is_empty()) {
			return R"json({"error": "'scene_path' is required when mode is 'custom'."})json";
		}
		editor->play_custom_scene(scene_path);
	} else {
		return vformat(R"json({"error": "Unsupported play mode '%s'."})json", mode);
	}

	return vformat(R"json({"entered_play_mode": "%s"})json", mode);
}

// ============================================================
// 播放主场景
// ============================================================
String PlayTools::play_main_scene(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	editor->play_main_scene();
	return R"json({"started": "main_scene"})json";
}

// ============================================================
// 退出播放模式
// ============================================================
String PlayTools::exit_play_mode(const Dictionary &p_args) {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	editor->stop_playing_scene();
	return R"json({"stopped": true})json";
}

// ============================================================
// 模拟输入动作
// ============================================================
String PlayTools::simulate_action(const Dictionary &p_args) {
	String action_name = String(p_args.get("action", "")).strip_edges();
	if (action_name.is_empty()) {
		return R"json({"error": "'action' is required."})json";
	}

	String mode = String(p_args.get("mode", "tap")).to_lower();
	float strength = float(p_args.get("strength", 1.0));

	// 按下事件
	if (mode == "press" || mode == "tap") {
		Ref<InputEventAction> press_event;
		press_event.instantiate();
		press_event->set_action(action_name);
		press_event->set_pressed(true);
		press_event->set_strength(strength);
		Input::get_singleton()->parse_input_event(press_event);
	}

	// 释放事件
	if (mode == "release" || mode == "tap") {
		Ref<InputEventAction> release_event;
		release_event.instantiate();
		release_event->set_action(action_name);
		release_event->set_pressed(false);
		release_event->set_strength(0.0);
		Input::get_singleton()->parse_input_event(release_event);
	}

	Dictionary result;
	result["action"] = action_name;
	result["mode"] = mode;
	result["strength"] = strength;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 模拟键盘事件
// ============================================================
String PlayTools::simulate_key_event(const Dictionary &p_args) {
	String mode = String(p_args.get("mode", "tap")).to_lower();

	Key keycode = (Key)_to_keycode(p_args.get("key", Variant()));
	Key physical_keycode = (Key)_to_keycode(p_args.get("physical_key", Variant()));

	if (keycode == Key::NONE && physical_keycode == Key::NONE) {
		return R"json({"error": "'key' or 'physical_key' is required."})json";
	}

	// 按下事件
	if (mode == "press" || mode == "tap") {
		Ref<InputEventKey> press_event;
		press_event.instantiate();
		press_event->set_pressed(true);
		if (keycode != Key::NONE) {
			press_event->set_keycode(keycode);
		}
		if (physical_keycode != Key::NONE) {
			press_event->set_physical_keycode(physical_keycode);
		}
		Input::get_singleton()->parse_input_event(press_event);
	}

	// 释放事件
	if (mode == "release" || mode == "tap") {
		Ref<InputEventKey> release_event;
		release_event.instantiate();
		release_event->set_pressed(false);
		if (keycode != Key::NONE) {
			release_event->set_keycode(keycode);
		}
		if (physical_keycode != Key::NONE) {
			release_event->set_physical_keycode(physical_keycode);
		}
		Input::get_singleton()->parse_input_event(release_event);
	}

	Dictionary result;
	result["mode"] = mode;
	result["keycode"] = (int)keycode;
	result["physical_keycode"] = (int)physical_keycode;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 模拟鼠标按钮
// ============================================================
String PlayTools::simulate_mouse_button(const Dictionary &p_args) {
	String mode = String(p_args.get("mode", "tap")).to_lower();
	int button_index = _to_mouse_button(p_args.get("button", "left"));
	Vector2 position = _to_vector2(p_args.get("position", Vector2()));

	// 按下事件
	if (mode == "press" || mode == "tap") {
		Ref<InputEventMouseButton> press_event;
		press_event.instantiate();
		press_event->set_button_index((MouseButton)button_index);
		press_event->set_position(position);
		press_event->set_pressed(true);
		Input::get_singleton()->parse_input_event(press_event);
	}

	// 释放事件
	if (mode == "release" || mode == "tap") {
		Ref<InputEventMouseButton> release_event;
		release_event.instantiate();
		release_event->set_button_index((MouseButton)button_index);
		release_event->set_position(position);
		release_event->set_pressed(false);
		Input::get_singleton()->parse_input_event(release_event);
	}

	Dictionary result;
	result["mode"] = mode;
	result["button_index"] = button_index;
	Dictionary pos;
	pos["x"] = position.x;
	pos["y"] = position.y;
	result["position"] = pos;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 模拟鼠标拖拽
// ============================================================
String PlayTools::simulate_mouse_drag(const Dictionary &p_args) {
	Vector2 from_position = _to_vector2(p_args.get("from_position", Vector2()));
	Vector2 to_position = _to_vector2(p_args.get("to_position", Vector2()));
	int steps = CLAMP(int(p_args.get("steps", 8)), 1, 240);
	int button_index = _to_mouse_button(p_args.get("button", "left"));

	// 按下
	Ref<InputEventMouseButton> press_event;
	press_event.instantiate();
	press_event->set_button_index((MouseButton)button_index);
	press_event->set_position(from_position);
	press_event->set_global_position(from_position);
	press_event->set_pressed(true);
	Input::get_singleton()->parse_input_event(press_event);

	// 拖拽步骤
	Vector2 previous = from_position;
	for (int step = 1; step <= steps; step++) {
		float weight = (float)step / (float)steps;
		Vector2 current = from_position.lerp(to_position, weight);

		Ref<InputEventMouseMotion> motion_event;
		motion_event.instantiate();
		motion_event->set_position(current);
		motion_event->set_global_position(current);
		motion_event->set_relative(current - previous);
		motion_event->set_button_mask((MouseButtonMask)(1 << (button_index - 1)));
		Input::get_singleton()->parse_input_event(motion_event);

		previous = current;
	}

	// 释放
	Ref<InputEventMouseButton> release_event;
	release_event.instantiate();
	release_event->set_button_index((MouseButton)button_index);
	release_event->set_position(to_position);
	release_event->set_global_position(to_position);
	release_event->set_pressed(false);
	Input::get_singleton()->parse_input_event(release_event);

	Dictionary result;
	Dictionary from_pos;
	from_pos["x"] = from_position.x;
	from_pos["y"] = from_position.y;
	result["from_position"] = from_pos;
	Dictionary to_pos;
	to_pos["x"] = to_position.x;
	to_pos["y"] = to_position.y;
	result["to_position"] = to_pos;
	result["steps"] = steps;
	result["button_index"] = button_index;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 模拟输入序列
// ============================================================
String PlayTools::simulate_input_sequence(const Dictionary &p_args) {
	Variant events_var = p_args.get("events", Variant());
	if (events_var.get_type() != Variant::ARRAY) {
		return R"json({"error": "'events' must be an array."})json";
	}

	Array events = events_var;
	Array results;

	for (int i = 0; i < events.size(); i++) {
		Dictionary item;
		if (events[i].get_type() != Variant::DICTIONARY) {
			item["type"] = "unknown";
			item["result"] = "Error: Sequence item must be an object.";
			results.push_back(item);
			continue;
		}

		Dictionary event_data = events[i];
		String event_type = String(event_data.get("type", "")).strip_edges();
		String result_text;

		if (event_type == "action") {
			result_text = simulate_action(event_data);
		} else if (event_type == "key") {
			result_text = simulate_key_event(event_data);
		} else if (event_type == "mouse_button") {
			result_text = simulate_mouse_button(event_data);
		} else if (event_type == "mouse_drag") {
			result_text = simulate_mouse_drag(event_data);
		} else {
			result_text = vformat(R"json({"error": "Unsupported sequence event type '%s'.'})json", event_type);
		}

		item["type"] = event_type;
		item["result"] = result_text;
		results.push_back(item);
	}

	Dictionary result;
	result["count"] = events.size();
	result["results"] = results;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 获取时间缩放
// ============================================================
String PlayTools::get_time_scale(const Dictionary &p_args) {
	Dictionary result;
	result["time_scale"] = Engine::get_singleton()->get_time_scale();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置时间缩放
// ============================================================
String PlayTools::set_time_scale(const Dictionary &p_args) {
	if (!p_args.has("value")) {
		return R"json({"error": "'value' is required."})json";
	}

	double value = double(p_args["value"]);
	Engine::get_singleton()->set_time_scale(value);

	Dictionary result;
	result["time_scale"] = Engine::get_singleton()->get_time_scale();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 捕获编辑器视图
// ============================================================
String PlayTools::capture_editor_view(const Dictionary &p_args) {
	String view = String(p_args.get("view", "2d")).to_lower();

	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor) {
		return R"json({"error": "Editor interface not available."})json";
	}

	// 获取视口纹理
	Ref<Texture2D> texture;
	if (view == "3d") {
		int index = int(p_args.get("index", 0));
		SubViewport *viewport = editor->get_editor_viewport_3d(index);
		if (viewport) {
			texture = viewport->get_texture();
		}
	} else {
		SubViewport *viewport = editor->get_editor_viewport_2d();
		if (viewport) {
			texture = viewport->get_texture();
		}
	}

	if (texture.is_null()) {
		return vformat(R"json({"error": "Editor viewport '%s' is not available."})json", view);
	}

	Ref<Image> image = texture->get_image();
	if (image.is_null()) {
		return R"json({"error": "Failed to capture viewport image."})json";
	}

	Dictionary result;

	// 可选保存到文件
	if (bool(p_args.get("save_to_file", false))) {
		String save_path = _normalize_path(p_args.get("save_path", vformat("user://funplay_mcp_capture_%s.png", view)));
		String ensure_err = _ensure_parent_dir(save_path);
		if (!ensure_err.is_empty()) {
			return vformat(R"json({"error": "Failed to create parent directory for %s"})json", save_path);
		}
		Error save_err = image->save_png(save_path);
		if (save_err != OK) {
			return vformat(R"json({"error": "Failed to save screenshot to %s (code %d)."})json", save_path, (int)save_err);
		}
		result["saved_path"] = save_path;
	}

	// 可选返回 data URI
	if (bool(p_args.get("return_data_uri", true))) {
		PackedByteArray png_bytes = image->save_png_to_buffer();
		String base64 = CryptoCore::b64_encode_str(png_bytes.ptr(), png_bytes.size());
		return "data:image/png;base64," + base64;
	}

	result["captured_view"] = view;
	Dictionary size;
	size["x"] = image->get_width();
	size["y"] = image->get_height();
	result["size"] = size;
	return JSON::stringify(result, "\t");
}

// ============================================================
void PlayTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &PlayTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("get_play_state", "args"), &PlayTools::get_play_state, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("enter_play_mode", "args"), &PlayTools::enter_play_mode, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("play_main_scene", "args"), &PlayTools::play_main_scene, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("exit_play_mode", "args"), &PlayTools::exit_play_mode, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("simulate_action", "args"), &PlayTools::simulate_action);
	ClassDB::bind_method(D_METHOD("simulate_key_event", "args"), &PlayTools::simulate_key_event);
	ClassDB::bind_method(D_METHOD("simulate_mouse_button", "args"), &PlayTools::simulate_mouse_button);
	ClassDB::bind_method(D_METHOD("simulate_mouse_drag", "args"), &PlayTools::simulate_mouse_drag);
	ClassDB::bind_method(D_METHOD("simulate_input_sequence", "args"), &PlayTools::simulate_input_sequence);
	ClassDB::bind_method(D_METHOD("get_time_scale", "args"), &PlayTools::get_time_scale, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_time_scale", "args"), &PlayTools::set_time_scale);
	ClassDB::bind_method(D_METHOD("capture_editor_view", "args"), &PlayTools::capture_editor_view, DEFVAL(Dictionary()));
}

// ============================================================
// 内部辅助方法
// ============================================================

String PlayTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

String PlayTools::_ensure_parent_dir(const String &p_path) const {
	String parent_dir = p_path.get_base_dir();
	if (parent_dir.is_empty() || parent_dir == "res://" || parent_dir == "user://") {
		return "";
	}
	Error err = DirAccess::make_dir_recursive_absolute(parent_dir);
	return (err != OK) ? vformat("Error: Failed to create directory %s", parent_dir) : "";
}

Vector2 PlayTools::_to_vector2(const Variant &p_value) const {
	switch (p_value.get_type()) {
		case Variant::VECTOR2:
			return p_value;
		case Variant::ARRAY: {
			Array arr = p_value;
			if (arr.size() >= 2) {
				return Vector2(double(arr[0]), double(arr[1]));
			}
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return Vector2(double(d.get("x", 0.0)), double(d.get("y", 0.0)));
		} break;
		case Variant::STRING: {
			String s = p_value;
			Vector<String> parts = s.split(",");
			if (parts.size() >= 2) {
				return Vector2(parts[0].to_float(), parts[1].to_float());
			}
		} break;
		default:
			break;
	}
	return Vector2();
}

int PlayTools::_to_keycode(const Variant &p_value) const {
	if (p_value.get_type() == Variant::INT) {
		return int(p_value);
	}
	if (p_value.get_type() == Variant::NIL) {
		return 0;
	}

	String text = String(p_value).strip_edges();
	if (text.is_empty()) {
		return 0;
	}

	String normalized = text.to_lower();
	if (KEY_NAME_MAP.has(normalized)) {
		return (int)KEY_NAME_MAP[normalized];
	}

	// 单字符映射为 Unicode
	if (text.length() == 1) {
		return (int)text[0];
	}
	return 0;
}

int PlayTools::_to_mouse_button(const Variant &p_value) const {
	if (p_value.get_type() == Variant::INT) {
		return int(p_value);
	}
	if (p_value.get_type() == Variant::NIL) {
		return (int)MouseButton::LEFT;
	}

	String normalized = String(p_value).strip_edges().to_lower();
	if (MOUSE_BUTTON_MAP.has(normalized)) {
		return (int)MOUSE_BUTTON_MAP[normalized];
	}
	return (int)MouseButton::LEFT;
}
