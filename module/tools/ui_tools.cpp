/**
 * ui_tools.cpp - UI 创建工具实现
 *
 * Godot Control / CanvasLayer UI 控件的创建和配置。
 * 从 funplay_core_tools.gd 的 UI 相关方法迁移而来。
 */

#include "ui_tools.h"

#include "editor/editor_interface.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/editor_data.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/panel.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/box_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/margin_container.h"
#include "scene/main/canvas_layer.h"
#include "scene/resources/texture.h"
#include "scene/resources/style_box.h"
#include "scene/resources/font.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/object/undo_redo.h"
#include "core/config/project_settings.h"

// ============================================================
void UITools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建 UI 根节点
// ============================================================
String UITools::create_ui_root(const Dictionary &p_args) {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"json({"error": "No edited scene is open."})json";
	}

	String kind = String(p_args.get("kind", "canvas_layer")).to_lower();
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = _resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	Node *created_root = nullptr;
	Control *primary_control = nullptr;

	if (kind == "canvas_layer") {
		CanvasLayer *canvas_layer = memnew(CanvasLayer);
		canvas_layer->set_name(_safe_name(String(p_args.get("name", "UI")).strip_edges(), "UI"));
		parent->add_child(canvas_layer);
		_assign_owner_recursive(canvas_layer, scene_root);
		created_root = canvas_layer;

		Control *control_root = memnew(Control);
		control_root->set_name(_safe_name(String(p_args.get("control_name", "Root")).strip_edges(), "Root"));
		canvas_layer->add_child(control_root);
		_assign_owner_recursive(control_root, scene_root);
		_apply_layout_preset(control_root, "full_rect");
		primary_control = control_root;
	} else if (kind == "control") {
		Control *control = memnew(Control);
		control->set_name(_safe_name(String(p_args.get("name", "UIRoot")).strip_edges(), "UIRoot"));
		parent->add_child(control);
		_assign_owner_recursive(control, scene_root);
		_apply_layout_preset(control, String(p_args.get("layout_preset", "full_rect")));
		created_root = control;
		primary_control = control;
	} else {
		return vformat(R"json({"error": "Unsupported ui root kind '%s'.'})json", kind);
	}

	if (bool(p_args.get("select_new_node", true))) {
		_select_node(created_root);
	}

	Dictionary result;
	result["created_root"] = _node_to_summary(created_root);
	if (primary_control) {
		result["primary_control"] = _build_control_info(primary_control);
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建任意 Control 子类
// ============================================================
String UITools::create_control(const Dictionary &p_args) {
	String control_type = String(p_args.get("control_type", "")).strip_edges();
	if (control_type.is_empty()) {
		return R"json({"error": "'control_type' is required."})json";
	}

	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"json({"error": "No edited scene is open."})json";
	}

	if (!ClassDB::class_exists(control_type)) {
		return vformat(R"json({"error": "Unknown control type '%s'.'})json", control_type);
	}

	Object *obj = ClassDB::instantiate(control_type.utf8().get_data());
	if (!obj || !Object::cast_to<Control>(obj)) {
		if (obj) {
			memdelete(obj);
		}
		return vformat(R"json({"error": "'%s' is not instantiable as a Control."})json", control_type);
	}

	Control *control = Object::cast_to<Control>(obj);
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = _resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	control->set_name(_safe_name(String(p_args.get("name", control_type)).strip_edges(), control_type));
	parent->add_child(control);
	_assign_owner_recursive(control, scene_root);

	// 设置通用属性
	if (p_args.has("text") && _has_property(control, "text")) {
		control->set("text", String(p_args["text"]));
	}
	if (p_args.has("placeholder_text") && _has_property(control, "placeholder_text")) {
		control->set("placeholder_text", String(p_args["placeholder_text"]));
	}
	if (p_args.has("tooltip_text") && _has_property(control, "tooltip_text")) {
		control->set("tooltip_text", String(p_args["tooltip_text"]));
	}
	if (p_args.has("size")) {
		control->set_size(_to_vector2(p_args["size"]));
	}
	if (p_args.has("position")) {
		control->set_position(_to_vector2(p_args["position"]));
	}
	if (p_args.has("custom_minimum_size")) {
		control->set_custom_minimum_size(_to_vector2(p_args["custom_minimum_size"]));
	}
	if (p_args.has("layout_preset")) {
		_apply_layout_preset(control, String(p_args["layout_preset"]));
	}
	if (p_args.has("horizontal_size_flags")) {
		control->set_h_size_flags(_parse_size_flags(p_args["horizontal_size_flags"]));
	}
	if (p_args.has("vertical_size_flags")) {
		control->set_v_size_flags(_parse_size_flags(p_args["vertical_size_flags"]));
	}
	if (p_args.has("theme_type_variation") && _has_property(control, "theme_type_variation")) {
		control->set("theme_type_variation", String(p_args["theme_type_variation"]));
	}

	// TextureRect 特殊处理
	TextureRect *tex_rect = Object::cast_to<TextureRect>(control);
	if (tex_rect) {
		if (p_args.has("texture_path")) {
			String texture_path = String(p_args["texture_path"]).strip_edges();
			if (!texture_path.is_empty()) {
				Ref<Texture2D> texture = ResourceLoader::load(_normalize_path(texture_path));
				if (texture.is_valid()) {
					tex_rect->set_texture(texture);
				}
			}
		}
		if (p_args.has("stretch_mode")) {
			tex_rect->set_stretch_mode((TextureRect::StretchMode)int(p_args["stretch_mode"]));
		}
		if (p_args.has("expand_mode")) {
			tex_rect->set_expand_mode((TextureRect::ExpandMode)int(p_args["expand_mode"]));
		}
	}

	if (bool(p_args.get("select_new_node", true))) {
		_select_node(control);
	}

	Dictionary result;
	result["created"] = _build_control_info(control);
	if (parent) {
		result["parent_path"] = String(parent->get_path());
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建 Label
// ============================================================
String UITools::create_label(const Dictionary &p_args) {
	Dictionary merged = p_args.duplicate(true);
	merged["control_type"] = "Label";
	return create_control(merged);
}

// ============================================================
// 创建 Button
// ============================================================
String UITools::create_button(const Dictionary &p_args) {
	Dictionary merged = p_args.duplicate(true);
	merged["control_type"] = "Button";
	return create_control(merged);
}

// ============================================================
// 创建 Panel
// ============================================================
String UITools::create_panel(const Dictionary &p_args) {
	Dictionary merged = p_args.duplicate(true);
	merged["control_type"] = "Panel";
	return create_control(merged);
}

// ============================================================
// 创建 TextureRect
// ============================================================
String UITools::create_texture_rect(const Dictionary &p_args) {
	Dictionary merged = p_args.duplicate(true);
	merged["control_type"] = "TextureRect";
	return create_control(merged);
}

// ============================================================
// 创建 Container
// ============================================================
String UITools::create_container(const Dictionary &p_args) {
	String container_type = String(p_args.get("container_type", "")).strip_edges();
	if (container_type.is_empty()) {
		return R"json({"error": "'container_type' is required."})json";
	}
	Dictionary modified_args = p_args;
	modified_args["control_type"] = container_type;
	return create_control(modified_args);
}

// ============================================================
// 设置 Control 布局
// ============================================================
String UITools::set_control_layout(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	Control *control = _resolve_control(node_path);
	if (!control) {
		return R"json({"error": "Control not found."})json";
	}

	Dictionary changes;

	if (p_args.has("layout_preset")) {
		_apply_layout_preset(control, String(p_args["layout_preset"]));
	}

	if (p_args.has("anchors")) {
		Variant anchors_var = p_args["anchors"];
		if (anchors_var.get_type() == Variant::DICTIONARY) {
			Dictionary anchors = anchors_var;
			changes["anchor_left"] = double(anchors.get("left", control->get_anchor(SIDE_LEFT)));
			changes["anchor_top"] = double(anchors.get("top", control->get_anchor(SIDE_TOP)));
			changes["anchor_right"] = double(anchors.get("right", control->get_anchor(SIDE_RIGHT)));
			changes["anchor_bottom"] = double(anchors.get("bottom", control->get_anchor(SIDE_BOTTOM)));
		}
	}

	if (p_args.has("offsets")) {
		Variant offsets_var = p_args["offsets"];
		if (offsets_var.get_type() == Variant::DICTIONARY) {
			Dictionary offsets = offsets_var;
			changes["offset_left"] = double(offsets.get("left", control->get_offset(SIDE_LEFT)));
			changes["offset_top"] = double(offsets.get("top", control->get_offset(SIDE_TOP)));
			changes["offset_right"] = double(offsets.get("right", control->get_offset(SIDE_RIGHT)));
			changes["offset_bottom"] = double(offsets.get("bottom", control->get_offset(SIDE_BOTTOM)));
		}
	}

	if (p_args.has("size")) {
		changes["size"] = _to_vector2(p_args["size"]);
	}
	if (p_args.has("position")) {
		changes["position"] = _to_vector2(p_args["position"]);
	}
	if (p_args.has("grow_horizontal")) {
		changes["grow_horizontal"] = int(p_args["grow_horizontal"]);
	}
	if (p_args.has("grow_vertical")) {
		changes["grow_vertical"] = int(p_args["grow_vertical"]);
	}

	_commit_undoable_properties(control, changes, "Set Control Layout", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["control"] = _build_control_info(control);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 Control 大小标志
// ============================================================
String UITools::set_control_size_flags(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	Control *control = _resolve_control(node_path);
	if (!control) {
		return R"json({"error": "Control not found."})json";
	}

	Dictionary changes;
	if (p_args.has("horizontal")) {
		changes["size_flags_horizontal"] = _parse_size_flags(p_args["horizontal"]);
	}
	if (p_args.has("vertical")) {
		changes["size_flags_vertical"] = _parse_size_flags(p_args["vertical"]);
	}
	if (p_args.has("stretch_ratio")) {
		changes["size_flags_stretch_ratio"] = double(p_args["stretch_ratio"]);
	}

	_commit_undoable_properties(control, changes, "Set Control Size Flags", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["control"] = _build_control_info(control);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 Control 文本
// ============================================================
String UITools::set_control_text(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	Control *control = _resolve_control(node_path);
	if (!control) {
		return R"json({"error": "Control not found."})json";
	}

	String text = String(p_args.get("text", ""));
	String property_name = String(p_args.get("property", "text")).strip_edges();
	if (property_name.is_empty()) {
		property_name = "text";
	}

	if (!_has_property(control, property_name)) {
		return vformat(R"json({"error": "Control does not expose property '%s'.'})json", property_name);
	}

	Dictionary changes;
	changes[property_name] = text;
	_commit_undoable_properties(control, changes, "Set Control Text", bool(p_args.get("undoable", true)));

	Dictionary result;
	result["control"] = _build_control_info(control);
	result["property"] = property_name;
	result["text"] = text;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 Control 主题覆盖
// ============================================================
String UITools::set_control_theme_override(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	Control *control = _resolve_control(node_path);
	if (!control) {
		return R"json({"error": "Control not found."})json";
	}

	String override_type = String(p_args.get("override_type", "")).to_lower();
	String name = String(p_args.get("name", "")).strip_edges();
	if (override_type.is_empty() || name.is_empty()) {
		return R"json({"error": "'override_type' and 'name' are required."})json";
	}

	if (override_type == "color") {
		control->add_theme_color_override(name, _to_color(p_args.get("value", Variant())));
	} else if (override_type == "constant") {
		control->add_theme_constant_override(name, int(p_args.get("value", 0)));
	} else if (override_type == "font_size") {
		control->add_theme_font_size_override(name, int(p_args.get("value", 0)));
	} else if (override_type == "font") {
		String font_path = _normalize_path(p_args.get("resource_path", ""));
		Ref<Font> font = ResourceLoader::load(font_path);
		if (font.is_null()) {
			return vformat(R"json({"error": "Font resource not found: %s"})json", font_path);
		}
		control->add_theme_font_override(name, font);
	} else if (override_type == "stylebox") {
		String style_path = _normalize_path(p_args.get("resource_path", ""));
		Ref<StyleBox> stylebox = ResourceLoader::load(style_path);
		if (stylebox.is_null()) {
			return vformat(R"json({"error": "StyleBox resource not found: %s"})json", style_path);
		}
		control->call("add_theme_stylebox_override", name, stylebox);
	} else {
		return vformat(R"json({"error": "Unsupported override_type '%s'.'})json", override_type);
	}

	Dictionary result;
	result["control"] = _build_control_info(control);
	result["override_type"] = override_type;
	result["name"] = name;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 连接节点信号
// ============================================================
String UITools::connect_node_signal(const Dictionary &p_args) {
	String source_path = String(p_args.get("source_path", "")).strip_edges();
	String target_path = String(p_args.get("target_path", "")).strip_edges();
	String signal_name = String(p_args.get("signal_name", "")).strip_edges();
	String method_name = String(p_args.get("method_name", "")).strip_edges();

	if (source_path.is_empty() || target_path.is_empty()) {
		return R"json({"error": "Source or target node not found."})json";
	}
	if (signal_name.is_empty() || method_name.is_empty()) {
		return R"json({"error": "'signal_name' and 'method_name' are required."})json";
	}

	Node *source_node = _resolve_node_path(source_path);
	Node *target_node = _resolve_node_path(target_path);
	if (!source_node || !target_node) {
		return R"json({"error": "Source or target node not found."})json";
	}

	if (!source_node->has_signal(signal_name)) {
		return vformat(R"json({"error": "Source node does not have signal '%s'.'})json", signal_name);
	}

	Callable callable = Callable(target_node, method_name);
	if (source_node->is_connected(signal_name, callable)) {
		return R"json({"error": "Signal already connected."})json";
	}

	int flags = int(p_args.get("flags", 0));
	Error err = source_node->connect(signal_name, callable, flags);
	if (err != OK) {
		return vformat(R"json({"error": "Failed to connect signal '%s' (code %d)."})json", signal_name, (int)err);
	}

	Dictionary result;
	result["source"] = _node_to_summary(source_node);
	result["target"] = _node_to_summary(target_node);
	result["signal_name"] = signal_name;
	result["method_name"] = method_name;
	return JSON::stringify(result, "\t");
}

// ============================================================
void UITools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &UITools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("create_ui_root", "args"), &UITools::create_ui_root, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_control", "args"), &UITools::create_control);
	ClassDB::bind_method(D_METHOD("create_label", "args"), &UITools::create_label, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_button", "args"), &UITools::create_button, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_panel", "args"), &UITools::create_panel, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_texture_rect", "args"), &UITools::create_texture_rect, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_container", "args"), &UITools::create_container);
	ClassDB::bind_method(D_METHOD("set_control_layout", "args"), &UITools::set_control_layout);
	ClassDB::bind_method(D_METHOD("set_control_size_flags", "args"), &UITools::set_control_size_flags);
	ClassDB::bind_method(D_METHOD("set_control_text", "args"), &UITools::set_control_text);
	ClassDB::bind_method(D_METHOD("set_control_theme_override", "args"), &UITools::set_control_theme_override);
	ClassDB::bind_method(D_METHOD("connect_node_signal", "args"), &UITools::connect_node_signal);
}

// ============================================================
// 内部辅助方法
// ============================================================

Node *UITools::_get_edited_scene_root() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	return editor ? editor->get_edited_scene_root() : nullptr;
}

Node *UITools::_resolve_node_path(const String &p_path) const {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return nullptr;
	}
	String identifier = p_path.strip_edges();
	if (identifier.is_empty() || identifier == ".") {
		return scene_root;
	}
	if (String(scene_root->get_path()) == identifier) {
		return scene_root;
	}
	// Try relative path from scene root first
	String root_name = scene_root->get_name();
	String relative;
	if (identifier.begins_with("/" + root_name + "/")) {
		relative = identifier.substr(root_name.length() + 2);
	} else if (identifier.begins_with("/")) {
		relative = identifier.substr(1);
	} else {
		relative = identifier;
	}
	
	Node *found = scene_root->get_node_or_null(NodePath(relative));
	if (found) {
		return found;
	}
	if (identifier == "/" + root_name || identifier == root_name) {
		return scene_root;
	}
	return scene_root->get_node_or_null(NodePath(identifier));
}

Control *UITools::_resolve_control(const String &p_path) const {
	Node *node = _resolve_node_path(p_path);
	return node ? Object::cast_to<Control>(node) : nullptr;
}

void UITools::_assign_owner_recursive(Node *p_node, Node *p_owner) const {
	p_node->set_owner(p_owner);
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_assign_owner_recursive(child, p_owner);
		}
	}
}

Dictionary UITools::_node_to_summary(Node *p_node) const {
	Dictionary summary;
	if (!p_node) {
		return summary;
	}
	summary["id"] = itos(p_node->get_instance_id());
	summary["instance_id"] = p_node->get_instance_id();
	summary["name"] = p_node->get_name();
	summary["type"] = p_node->get_class();
	summary["path"] = String(p_node->get_path());
	return summary;
}

Dictionary UITools::_build_control_info(Control *p_control) const {
	Dictionary info;
	if (!p_control) {
		return info;
	}
	info["id"] = itos(p_control->get_instance_id());
	info["instance_id"] = p_control->get_instance_id();
	info["name"] = p_control->get_name();
	info["type"] = p_control->get_class();
	info["path"] = String(p_control->get_path());

	Dictionary pos;
	pos["x"] = p_control->get_position().x;
	pos["y"] = p_control->get_position().y;
	info["position"] = pos;

	Dictionary size;
	size["x"] = p_control->get_size().x;
	size["y"] = p_control->get_size().y;
	info["size"] = size;

	Dictionary anchors;
	anchors["left"] = p_control->get_anchor(SIDE_LEFT);
	anchors["top"] = p_control->get_anchor(SIDE_TOP);
	anchors["right"] = p_control->get_anchor(SIDE_RIGHT);
	anchors["bottom"] = p_control->get_anchor(SIDE_BOTTOM);
	info["anchors"] = anchors;

	Dictionary offsets;
	offsets["left"] = p_control->get_offset(SIDE_LEFT);
	offsets["top"] = p_control->get_offset(SIDE_TOP);
	offsets["right"] = p_control->get_offset(SIDE_RIGHT);
	offsets["bottom"] = p_control->get_offset(SIDE_BOTTOM);
	info["offsets"] = offsets;

	info["size_flags_horizontal"] = p_control->get_h_size_flags();
	info["size_flags_vertical"] = p_control->get_v_size_flags();

	return info;
}

String UITools::_safe_name(const String &p_requested, const String &p_fallback) const {
	String trimmed = p_requested.strip_edges();
	return trimmed.is_empty() ? p_fallback : trimmed;
}

void UITools::_select_node(Node *p_node) const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (!editor || !p_node) {
		return;
	}
	EditorSelection *selection = editor->get_selection();
	if (selection) {
		selection->clear();
		selection->add_node(p_node);
		editor->edit_node(p_node);
	}
}

void UITools::_apply_layout_preset(Control *p_control, const String &p_preset) const {
	if (!p_control) {
		return;
	}

	String preset = p_preset.to_lower();
	if (preset == "full_rect") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
		p_control->set_offset(SIDE_LEFT, 0.0);
		p_control->set_offset(SIDE_TOP, 0.0);
		p_control->set_offset(SIDE_RIGHT, 0.0);
		p_control->set_offset(SIDE_BOTTOM, 0.0);
	} else if (preset == "top_left") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 0.0);
		p_control->set_anchor(SIDE_BOTTOM, 0.0);
	} else if (preset == "top_right") {
		p_control->set_anchor(SIDE_LEFT, 1.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 0.0);
	} else if (preset == "bottom_left") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 1.0);
		p_control->set_anchor(SIDE_RIGHT, 0.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
	} else if (preset == "bottom_right") {
		p_control->set_anchor(SIDE_LEFT, 1.0);
		p_control->set_anchor(SIDE_TOP, 1.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
	} else if (preset == "center") {
		p_control->set_anchor(SIDE_LEFT, 0.5);
		p_control->set_anchor(SIDE_TOP, 0.5);
		p_control->set_anchor(SIDE_RIGHT, 0.5);
		p_control->set_anchor(SIDE_BOTTOM, 0.5);
	} else if (preset == "left_wide") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 0.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
	} else if (preset == "right_wide") {
		p_control->set_anchor(SIDE_LEFT, 1.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
	} else if (preset == "top_wide") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 0.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 0.0);
	} else if (preset == "bottom_wide") {
		p_control->set_anchor(SIDE_LEFT, 0.0);
		p_control->set_anchor(SIDE_TOP, 1.0);
		p_control->set_anchor(SIDE_RIGHT, 1.0);
		p_control->set_anchor(SIDE_BOTTOM, 1.0);
	}
}

void UITools::_commit_undoable_properties(Object *p_object, const Dictionary &p_changes, const String &p_action_name, bool p_undoable) const {
	if (!p_object || p_changes.is_empty()) {
		return;
	}

	EditorUndoRedoManager *undo_redo = _plugin ? EditorInterface::get_singleton()->get_editor_undo_redo() : nullptr;

	if (p_undoable && undo_redo) {
		undo_redo->create_action(p_action_name);
		Array keys = p_changes.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			undo_redo->add_do_property(p_object, key, p_changes[key]);
			undo_redo->add_undo_property(p_object, key, p_object->get(key));
		}
		undo_redo->commit_action();
	} else {
		Array keys = p_changes.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			p_object->set(key, p_changes[key]);
		}
	}
}

bool UITools::_has_property(Object *p_object, const String &p_property) const {
	if (!p_object) {
		return false;
	}
	TypedArray<Dictionary> prop_list = p_object->call("get_property_list");
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		if (String(prop_info.get("name", "")) == p_property) {
			return true;
		}
	}
	return false;
}

Vector2 UITools::_to_vector2(const Variant &p_value) const {
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
		default:
			break;
	}
	return Vector2();
}

Color UITools::_to_color(const Variant &p_value) const {
	if (p_value.get_type() == Variant::COLOR) {
		return p_value;
	}
	if (p_value.get_type() == Variant::DICTIONARY) {
		Dictionary d = p_value;
		return Color(
			double(d.get("r", 1.0)),
			double(d.get("g", 1.0)),
			double(d.get("b", 1.0)),
			double(d.get("a", 1.0))
		);
	}
	if (p_value.get_type() == Variant::ARRAY) {
		Array arr = p_value;
		if (arr.size() >= 3) {
			return Color(
				double(arr[0]),
				double(arr[1]),
				double(arr[2]),
				arr.size() >= 4 ? double(arr[3]) : 1.0
			);
		}
	}
	return Color();
}

int UITools::_parse_size_flags(const Variant &p_value) const {
	if (p_value.get_type() == Variant::INT) {
		return int(p_value);
	}
	if (p_value.get_type() == Variant::NIL) {
		return 0;
	}
	if (p_value.get_type() == Variant::ARRAY) {
		int combined = 0;
		Array arr = p_value;
		for (int i = 0; i < arr.size(); i++) {
			combined |= _parse_size_flags(arr[i]);
		}
		return combined;
	}
	String normalized = String(p_value).strip_edges().to_lower();
	if (normalized == "fill") {
		return Control::SIZE_FILL;
	}
	if (normalized == "expand") {
		return Control::SIZE_EXPAND;
	}
	if (normalized == "expand_fill") {
		return Control::SIZE_EXPAND_FILL;
	}
	if (normalized == "shrink_center") {
		return Control::SIZE_SHRINK_CENTER;
	}
	if (normalized == "shrink_end") {
		return Control::SIZE_SHRINK_END;
	}
	return 0;
}

String UITools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}
