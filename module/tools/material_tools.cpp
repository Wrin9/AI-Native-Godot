/**
 * material_tools.cpp - 材质工具实现
 *
 * 材质资源的创建和分配。
 * 从 funplay_core_tools.gd 的材质相关方法迁移而来。
 */

#include "material_tools.h"

#include "editor/editor_interface.h"
#include "editor/file_system/editor_file_system.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/material.h"
#include "core/io/resource_saver.h"
#include "scene/main/canvas_item.h"
#include "scene/3d/visual_instance_3d.h"
#include "core/io/dir_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/config/project_settings.h"

// ============================================================
void MaterialTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建材质
// ============================================================
String MaterialTools::create_material(const Dictionary &p_args) {
	String path = _normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	String material_type = String(p_args.get("material_type", "StandardMaterial3D")).strip_edges();
	if (!ClassDB::class_exists(material_type)) {
		return vformat(R"json({"error": "Unknown material type '%s'.'})json", material_type);
	}

	Object *obj = ClassDB::instantiate(material_type.utf8().get_data());
	if (!obj || !Object::cast_to<Resource>(obj)) {
		if (obj) {
			memdelete(obj);
		}
		return vformat(R"json({"error": "'%s' is not instantiable as a Resource."})json", material_type);
	}

	Resource *material = Object::cast_to<Resource>(obj);

	// 设置属性
	Variant props_var = p_args.get("properties", Variant());
	if (props_var.get_type() == Variant::DICTIONARY) {
		Dictionary properties = props_var;
		Array keys = properties.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			material->set(key, properties[key]);
		}
	}

	// 保存材质
	String ensure_err = _ensure_parent_dir(path);
	if (!ensure_err.is_empty()) {
		memdelete(material);
		return vformat(R"json({"error": "Failed to create parent directory for %s"})json", path);
	}

	Error save_err = ResourceSaver::save(Ref<Resource>(material), path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		memdelete(material);
		return vformat(R"json({"error": "Failed to save material to %s (code %d)."})json", path, (int)save_err);
	}

	_refresh_filesystem();

	Dictionary result;
	result["material_type"] = material_type;
	result["path"] = path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 分配材质
// ============================================================
String MaterialTools::assign_material(const Dictionary &p_args) {
	String target_path = String(p_args.get("target_path", "")).strip_edges();
	String material_path = _normalize_path(p_args.get("material_path", ""));
	if (target_path.is_empty() || material_path.is_empty()) {
		return R"json({"error": "'target_path' and 'material_path' are required."})json";
	}

	Node *node = _resolve_node_path(target_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", target_path);
	}

	Ref<Material> material = ResourceLoader::load(material_path);
	if (material.is_null()) {
		return vformat(R"json({"error": "Material not found or invalid: %s"})json", material_path);
	}

	int surface_index = int(p_args.get("surface_index", -1));

	// 根据节点类型分配材质
	CanvasItem *canvas_item = Object::cast_to<CanvasItem>(node);
	GeometryInstance3D *geo_3d = Object::cast_to<GeometryInstance3D>(node);

	if (canvas_item) {
		canvas_item->set_material(material);
	} else if (geo_3d) {
		if (surface_index >= 0 && geo_3d->has_method("set_surface_override_material")) {
			geo_3d->call("set_surface_override_material", surface_index, material);
		} else {
			geo_3d->set_material_override(material);
		}
	} else if (_has_property(node, "material")) {
		node->set("material", material);
	} else {
		return vformat(R"json({"error": "Node '%s' does not expose a supported material slot."})json", target_path);
	}

	Dictionary result;
	result["target"] = _node_to_summary(node);
	result["material_path"] = material_path;
	result["surface_index"] = surface_index;
	return JSON::stringify(result, "\t");
}

// ============================================================
void MaterialTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_editor_plugin", "plugin"), &MaterialTools::set_editor_plugin);
	ClassDB::bind_method(D_METHOD("create_material", "args"), &MaterialTools::create_material);
	ClassDB::bind_method(D_METHOD("assign_material", "args"), &MaterialTools::assign_material);
}

// ============================================================
// 内部辅助方法
// ============================================================

Node *MaterialTools::_get_edited_scene_root() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	return editor ? editor->get_edited_scene_root() : nullptr;
}

Node *MaterialTools::_resolve_node_path(const String &p_path) const {
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

String MaterialTools::_normalize_path(const String &p_path) const {
	String trimmed = p_path.strip_edges();
	if (trimmed.is_empty()) {
		return "";
	}
	if (trimmed.begins_with("res://") || trimmed.begins_with("user://")) {
		return trimmed.simplify_path();
	}
	return ("res://" + trimmed.lstrip("/")).simplify_path();
}

String MaterialTools::_ensure_parent_dir(const String &p_path) const {
	String parent_dir = p_path.get_base_dir();
	if (parent_dir.is_empty() || parent_dir == "res://" || parent_dir == "user://") {
		return "";
	}
	Error err = DirAccess::make_dir_recursive_absolute(parent_dir);
	return (err != OK) ? vformat("Error: Failed to create directory %s", parent_dir) : "";
}

Dictionary MaterialTools::_node_to_summary(Node *p_node) const {
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

bool MaterialTools::_has_property(Object *p_object, const String &p_property) const {
	if (!p_object) {
		return false;
	}
	Array prop_list = p_object->call("get_property_list");
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		if (String(prop_info.get("name", "")) == p_property) {
			return true;
		}
	}
	return false;
}

void MaterialTools::_refresh_filesystem() const {
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}
}
