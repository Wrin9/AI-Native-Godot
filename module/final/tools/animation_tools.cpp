/**
 * animation_tools.cpp - 动画工具实现
 *
 * AnimationPlayer、Animation 资源的创建和操作。
 * 从 funplay_core_tools.gd 的动画相关方法迁移而来。
 */

#include "animation_tools.h"

#include "editor/editor_interface.h"
#include "editor/editor_selection.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_library.h"
#include "scene/resources/animation.h"
#include "core/io/json.h"
#include "core/object/class_db.h"

// ============================================================
void AnimationTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建 AnimationPlayer
// ============================================================
String AnimationTools::create_animation_player(const Dictionary &p_args) {
	Node *scene_root = _get_edited_scene_root();
	if (!scene_root) {
		return R"({"error": "No edited scene is open."})";
	}

	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = _resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	AnimationPlayer *player = memnew(AnimationPlayer);
	player->set_name(_safe_name(String(p_args.get("name", "AnimationPlayer")).strip_edges(), "AnimationPlayer"));
	parent->add_child(player);
	_assign_owner_recursive(player, scene_root);

	if (p_args.has("root_node")) {
		player->set_root_node(NodePath(String(p_args["root_node"])));
	}

	if (bool(p_args.get("select_new_node", true))) {
		_select_node(player);
	}

	Dictionary result;
	result["created"] = _node_to_summary(player);
	result["parent_path"] = String(parent->get_path());
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建动画片段
// ============================================================
String AnimationTools::create_animation_clip(const Dictionary &p_args) {
	AnimationPlayer *player = _resolve_animation_player(String(p_args.get("animation_player_path", "")).strip_edges());
	if (!player) {
		return R"({"error": "AnimationPlayer not found."})";
	}

	String animation_name = String(p_args.get("animation_name", "")).strip_edges();
	if (animation_name.is_empty()) {
		return R"({"error": "'animation_name' is required."})";
	}

	String library_name = String(p_args.get("library_name", "")).strip_edges();

	Ref<Animation> animation;
	animation.instantiate();
	animation->set_length(double(p_args.get("length", 1.0)));
	animation->set_loop_mode((Animation::LoopMode)int(p_args.get("loop_mode", Animation::LOOP_NONE)));
	animation->set_step(double(p_args.get("step", 0.1)));

	AnimationLibrary *library = _get_or_create_animation_library(player, library_name);
	if (library->has_animation(animation_name)) {
		library->remove_animation(animation_name);
	}
	library->add_animation(animation_name, animation);

	if (bool(p_args.get("set_current", true))) {
		player->set_current_animation(animation_name);
	}

	Dictionary result;
	result["animation_player"] = _node_to_summary(player);
	result["library_name"] = library_name;
	result["animation_name"] = animation_name;
	result["length"] = animation->get_length();
	result["loop_mode"] = (int)animation->get_loop_mode();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 添加动画轨道
// ============================================================
String AnimationTools::add_animation_track(const Dictionary &p_args) {
	AnimationPlayer *player = _resolve_animation_player(String(p_args.get("animation_player_path", "")).strip_edges());
	if (!player) {
		return R"({"error": "AnimationPlayer not found."})";
	}

	String animation_name = String(p_args.get("animation_name", "")).strip_edges();
	if (animation_name.is_empty()) {
		return R"({"error": "'animation_name' is required."})";
	}

	String library_name = String(p_args.get("library_name", "")).strip_edges();
	AnimationLibrary *library = _get_or_create_animation_library(player, library_name);

	if (!library->has_animation(animation_name)) {
		return vformat(R"({"error": "Animation '%s' not found."})", animation_name);
	}

	Ref<Animation> animation = library->get_animation(animation_name);

	int track_type = _animation_track_type(String(p_args.get("track_type", "value")));
	int track_index = animation->add_track((Animation::TrackType)track_type);
	animation->track_set_path(track_index, NodePath(String(p_args.get("path", ""))));

	if (p_args.has("interpolation_type")) {
		animation->track_set_interpolation_type(track_index, (Animation::InterpolationType)int(p_args["interpolation_type"]));
	}
	if (p_args.has("update_mode") && track_type == Animation::TYPE_VALUE) {
		animation->value_track_set_update_mode(track_index, (Animation::UpdateMode)int(p_args["update_mode"]));
	}

	// 添加关键帧
	Variant keys_var = p_args.get("keys", Array());
	if (keys_var.get_type() == Variant::ARRAY) {
		Array keys = keys_var;
		for (int i = 0; i < keys.size(); i++) {
			if (keys[i].get_type() != Variant::DICTIONARY) {
				continue;
			}
			Dictionary key_data = keys[i];
			double time = double(key_data.get("time", 0.0));
			Variant value = key_data.get("value");
			float transition = float(key_data.get("transition", 1.0));
			animation->track_insert_key(track_index, time, value, transition);
		}
	}

	Dictionary result;
	result["animation_player"] = _node_to_summary(player);
	result["animation_name"] = animation_name;
	result["track_index"] = track_index;
	result["track_type"] = track_type;
	result["path"] = String(p_args.get("path", ""));
	result["key_count"] = animation->track_get_key_count(track_index);
	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出动画
// ============================================================
String AnimationTools::list_animations(const Dictionary &p_args) {
	AnimationPlayer *player = _resolve_animation_player(String(p_args.get("animation_player_path", "")).strip_edges());
	if (!player) {
		return R"({"error": "AnimationPlayer not found."})";
	}

	Array libraries;
	TypedArray<StringName> library_list = player->get_animation_library_list();
	for (int i = 0; i < library_list.size(); i++) {
		String library_name = String(library_list[i]);
		AnimationLibrary *library = player->get_animation_library(library_name);
		if (!library) {
			continue;
		}

		Array animations;
		PackedStringArray anim_list = library->get_animation_list();
		for (int j = 0; j < anim_list.size(); j++) {
			String anim_name = anim_list[j];
			Ref<Animation> anim = library->get_animation(anim_name);
			if (anim.is_null()) {
				continue;
			}
			Dictionary anim_entry;
			anim_entry["name"] = anim_name;
			anim_entry["length"] = anim->get_length();
			anim_entry["loop_mode"] = (int)anim->get_loop_mode();
			anim_entry["track_count"] = anim->get_track_count();
			animations.push_back(anim_entry);
		}

		Dictionary lib_entry;
		lib_entry["name"] = library_name;
		lib_entry["animations"] = animations;
		libraries.push_back(lib_entry);
	}

	Dictionary result;
	result["animation_player"] = _node_to_summary(player);
	result["libraries"] = libraries;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 播放动画
// ============================================================
String AnimationTools::play_animation(const Dictionary &p_args) {
	AnimationPlayer *player = _resolve_animation_player(String(p_args.get("animation_player_path", "")).strip_edges());
	if (!player) {
		return R"({"error": "AnimationPlayer not found."})";
	}

	String animation_name = String(p_args.get("animation_name", "")).strip_edges();
	if (animation_name.is_empty()) {
		return R"({"error": "'animation_name' is required."})";
	}

	double custom_blend = double(p_args.get("custom_blend", -1.0));
	double custom_speed = double(p_args.get("custom_speed", 1.0));
	bool from_end = p_args.get("from_end", false);

	player->play(animation_name, custom_blend, custom_speed, from_end);

	return vformat(R"({"playing": "%s"})", animation_name);
}

// ============================================================
void AnimationTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &AnimationTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("create_animation_player", "args"), &AnimationTools::create_animation_player, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_animation_clip", "args"), &AnimationTools::create_animation_clip);
	ClassDB::bind_method(D_METHOD("add_animation_track", "args"), &AnimationTools::add_animation_track);
	ClassDB::bind_method(D_METHOD("list_animations", "args"), &AnimationTools::list_animations);
	ClassDB::bind_method(D_METHOD("play_animation", "args"), &AnimationTools::play_animation);
}

// ============================================================
// 内部辅助方法
// ============================================================

Node *AnimationTools::_get_edited_scene_root() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	return editor ? editor->get_edited_scene_root() : nullptr;
}

Node *AnimationTools::_resolve_node_path(const String &p_path) const {
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
	if (identifier.begins_with("/")) {
		SceneTree *tree = scene_root->get_tree();
		if (tree && tree->get_root()) {
			return tree->get_root()->get_node_or_null(NodePath(identifier));
		}
	}
	return scene_root->get_node_or_null(NodePath(identifier));
}

AnimationPlayer *AnimationTools::_resolve_animation_player(const String &p_path) const {
	Node *node = _resolve_node_path(p_path);
	return node ? Object::cast_to<AnimationPlayer>(node) : nullptr;
}

void AnimationTools::_assign_owner_recursive(Node *p_node, Node *p_owner) const {
	p_node->set_owner(p_owner);
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = Object::cast_to<Node>(p_node->get_child(i));
		if (child) {
			_assign_owner_recursive(child, p_owner);
		}
	}
}

Dictionary AnimationTools::_node_to_summary(Node *p_node) const {
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

String AnimationTools::_safe_name(const String &p_requested, const String &p_fallback) const {
	String trimmed = p_requested.strip_edges();
	return trimmed.is_empty() ? p_fallback : trimmed;
}

void AnimationTools::_select_node(Node *p_node) const {
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

AnimationLibrary *AnimationTools::_get_or_create_animation_library(AnimationPlayer *p_player, const String &p_library_name) const {
	if (p_player->has_animation_library(p_library_name)) {
		return p_player->get_animation_library(p_library_name);
	}
	AnimationLibrary *library = memnew(AnimationLibrary);
	p_player->add_animation_library(p_library_name, library);
	return library;
}

int AnimationTools::_animation_track_type(const String &p_track_type) const {
	String lower = p_track_type.to_lower();
	if (lower == "position_3d") {
		return Animation::TYPE_POSITION_3D;
	}
	if (lower == "rotation_3d") {
		return Animation::TYPE_ROTATION_3D;
	}
	if (lower == "scale_3d") {
		return Animation::TYPE_SCALE_3D;
	}
	if (lower == "blend_shape") {
		return Animation::TYPE_BLEND_SHAPE;
	}
	if (lower == "method") {
		return Animation::TYPE_METHOD;
	}
	if (lower == "bezier") {
		return Animation::TYPE_BEZIER;
	}
	if (lower == "audio") {
		return Animation::TYPE_AUDIO;
	}
	if (lower == "animation") {
		return Animation::TYPE_ANIMATION;
	}
	// 默认：值轨道
	return Animation::TYPE_VALUE;
}
