/**
 * extra_tools.cpp - 额外工具实现（Viewport, 信号, 组, 导出）
 */
#include "extra_tools.h"

#include "editor/editor_interface.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/export/editor_export.h"
#include "editor/export/editor_export_preset.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "scene/main/viewport.h"
#include "scene/gui/subviewport_container.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/templates/hash_set.h"
#include "mcp_tool_helpers.h"

void ExtraTools::set_editor_plugin(EditorPlugin *p_plugin) { _plugin = p_plugin; }

// ============================================================
String ExtraTools::create_viewport(const Dictionary &p_args) {
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	String name = String(p_args.get("name", "SubViewport")).strip_edges();
	int size_w = int(p_args.get("size_w", 256));
	int size_h = int(p_args.get("size_h", 256));

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) return vformat(R"json({"error": "Parent not found: %s"})json", parent_path);
	Node *scene_root = mcp_get_scene_root();

	SubViewportContainer *container = memnew(SubViewportContainer);
	container->set_name(name + "Container");
	container->set_stretch(true);
	parent->add_child(container);
	container->set_owner(scene_root);

	SubViewport *vp = memnew(SubViewport);
	vp->set_name(name);
	vp->set_size(Size2i(size_w, size_h));
	vp->set_update_mode(SubViewport::UPDATE_ALWAYS);
	container->add_child(vp);
	vp->set_owner(scene_root);

	Dictionary result;
	result["container"] = mcp_node_summary(container);
	result["viewport"] = mcp_node_summary(vp);
	Array size_arr;
	size_arr.append(size_w);
	size_arr.append(size_h);
	result["size"] = size_arr;
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::disconnect_node_signal(const Dictionary &p_args) {
	String source_path = String(p_args.get("source_path", "")).strip_edges();
	String signal_name = String(p_args.get("signal_name", "")).strip_edges();
	String target_path = String(p_args.get("target_path", "")).strip_edges();
	String method_name = String(p_args.get("method_name", "")).strip_edges();

	if (source_path.is_empty() || signal_name.is_empty()) {
		return R"json({"error": "'source_path', 'signal_name', 'target_path', and 'method_name' are required."})json";
	}

	Node *source = mcp_resolve_node_path(source_path);
	if (!source) return vformat(R"json({"error": "Source node not found: %s"})json", source_path);
	Node *target = mcp_resolve_node_path(target_path);
	if (!target) return vformat(R"json({"error": "Target node not found: %s"})json", target_path);

	source->disconnect(signal_name, Callable(target, method_name));

	Dictionary result;
	result["source"] = String(source->get_path());
	result["signal"] = signal_name;
	result["target"] = String(target->get_path());
	result["method"] = method_name;
	result["status"] = "disconnected";
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::get_node_connections(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) return R"json({"error": "'node_path' is required."})json";

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) return vformat(R"json({"error": "Node not found: %s"})json", node_path);

	Array conn_list;
	// 使用 _get_signal_list() (返回 TypedArray<Dictionary>)
	// 使用公开 API get_signal_list + get_signal_connection_list
	List<MethodInfo> signals;
	node->get_signal_list(&signals);
	for (const MethodInfo &si : signals) {
		String sig_name = si.name;
		List<Object::Connection> conns;
		node->get_signal_connection_list(sig_name, &conns);
		for (const Object::Connection &conn : conns) {
			Dictionary entry;
			entry["signal"] = sig_name;
			Object *target_obj = conn.callable.get_object();
			if (target_obj) {
				Node *target_node = Object::cast_to<Node>(target_obj);
				entry["target"] = target_node ? vformat("%s (%s)", target_node->get_name(), String(target_node->get_path())) : target_obj->get_class();
			}
			entry["method"] = conn.callable.get_method();
			entry["flags"] = (int)conn.flags;
			conn_list.append(entry);
		}
	}

	Dictionary result;
	result["node"] = String(node->get_path());
	result["connections"] = conn_list;
	result["count"] = conn_list.size();
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::add_node_to_group(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String group = String(p_args.get("group", "")).strip_edges();
	if (node_path.is_empty() || group.is_empty()) {
		return R"json({"error": "'node_path' and 'group' are required."})json";
	}
	Node *node = mcp_resolve_node_path(node_path);
	if (!node) return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	node->add_to_group(group);
	Dictionary result;
	result["node"] = String(node->get_path());
	result["group"] = group;
	result["status"] = "added";
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::remove_node_from_group(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	String group = String(p_args.get("group", "")).strip_edges();
	if (node_path.is_empty() || group.is_empty()) {
		return R"json({"error": "'node_path' and 'group' are required."})json";
	}
	Node *node = mcp_resolve_node_path(node_path);
	if (!node) return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	node->remove_from_group(group);
	Dictionary result;
	result["node"] = String(node->get_path());
	result["group"] = group;
	result["status"] = "removed";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 递归收集所有组
static void _collect_all_groups(Node *p_node, HashSet<StringName> &r_groups) {
	if (!p_node) return;
	List<Node::GroupInfo> gi;
	p_node->get_groups(&gi);
	for (const Node::GroupInfo &g : gi) {
		r_groups.insert(g.name);
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		_collect_all_groups(p_node->get_child(i), r_groups);
	}
}

String ExtraTools::list_groups(const Dictionary &p_args) {
	SceneTree *tree = SceneTree::get_singleton();
	if (!tree) return R"json({"error": "SceneTree not available"})json";

	HashSet<StringName> all_groups;
	// tree->get_root() 返回 Window*，遍历其子节点
	Window *root_win = tree->get_root();
	if (root_win) {
		_collect_all_groups(root_win, all_groups);
	}

	Array group_list;
	for (const StringName &g : all_groups) {
		Dictionary entry;
		entry["name"] = String(g);
		entry["node_count"] = tree->get_node_count_in_group(g);
		group_list.append(entry);
	}

	Dictionary result;
	result["groups"] = group_list;
	result["count"] = group_list.size();
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::export_project(const Dictionary &p_args) {
	String preset_name = String(p_args.get("preset_name", "")).strip_edges();
	String export_path = String(p_args.get("export_path", "")).strip_edges();
	bool debug = bool(p_args.get("debug", false));

	Dictionary result;
	result["action"] = "export_project";
	result["preset_name"] = preset_name;
	result["export_path"] = export_path;
	result["debug"] = debug;

	EditorExport *ee = EditorExport::get_singleton();
	if (!ee) {
		result["status"] = "unavailable";
		result["message"] = "EditorExport not available. Use Project > Export in the editor.";
		return JSON::stringify(result, "\t");
	}

	// 查找预设
	int preset_count = ee->get_export_preset_count();
	for (int i = 0; i < preset_count; i++) {
		Ref<EditorExportPreset> p = ee->get_export_preset(i);
		if (p.is_valid() && (p->get_name() == preset_name || preset_name.is_empty())) {
			if (!export_path.is_empty()) {
				p->set_export_path(export_path);
			}
			result["status"] = "preset_found";
			result["preset"] = p->get_name();
			result["message"] = "Export preset configured. Use Project > Export to export.";
			return JSON::stringify(result, "\t");
		}
	}

	result["status"] = "no_preset";
	result["message"] = "Export preset not found. Create one in Project > Export first.";
	return JSON::stringify(result, "\t");
}

// ============================================================
String ExtraTools::set_export_preset(const Dictionary &p_args) {
	String preset_name = String(p_args.get("preset_name", "")).strip_edges();
	String platform = String(p_args.get("platform", "windows")).strip_edges().to_lower();
	String export_path = String(p_args.get("export_path", "")).strip_edges();

	Dictionary result;
	result["action"] = "set_export_preset";
	result["preset_name"] = preset_name;
	result["platform"] = platform;

	EditorExport *ee = EditorExport::get_singleton();
	if (!ee) {
		result["status"] = "unavailable";
		result["message"] = "EditorExport not available. Use Project > Export in the editor.";
		return JSON::stringify(result, "\t");
	}

	// 查找已有预设
	int preset_count = ee->get_export_preset_count();
	for (int i = 0; i < preset_count; i++) {
		Ref<EditorExportPreset> p = ee->get_export_preset(i);
		if (p.is_valid() && p->get_name() == preset_name) {
			if (!export_path.is_empty()) {
				p->set_export_path(export_path);
			}
			// 应用自定义设置
			Variant settings_var = p_args.get("settings", Variant());
			if (settings_var.get_type() == Variant::DICTIONARY) {
				Dictionary settings = settings_var;
				Array keys = settings.keys();
				for (int k = 0; k < keys.size(); k++) {
					p->set(String(keys[k]), settings[keys[k]]);
				}
			}
			result["status"] = "updated";
			return JSON::stringify(result, "\t");
		}
	}

	// 查找平台并创建新预设
	int platform_count = ee->get_export_platform_count();
	for (int i = 0; i < platform_count; i++) {
		Ref<EditorExportPlatform> ep = ee->get_export_platform(i);
		if (ep.is_valid()) {
			String plat_name = ep->get_name().to_lower();
			if (plat_name.find(platform) >= 0) {
				Ref<EditorExportPreset> new_preset;
				new_preset.instantiate();
				new_preset->set_name(preset_name);
				if (!export_path.is_empty()) {
					new_preset->set_export_path(export_path);
				}
				ee->add_export_preset(new_preset);
				result["status"] = "created";
				result["platform"] = ep->get_name();
				return JSON::stringify(result, "\t");
			}
		}
	}

	result["status"] = "platform_not_found";
	result["message"] = vformat("Export platform '%s' not found.", platform);
	return JSON::stringify(result, "\t");
}

// ============================================================
void ExtraTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_viewport", "args"), &ExtraTools::create_viewport, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("disconnect_node_signal", "args"), &ExtraTools::disconnect_node_signal, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_node_connections", "args"), &ExtraTools::get_node_connections, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("add_node_to_group", "args"), &ExtraTools::add_node_to_group, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("remove_node_from_group", "args"), &ExtraTools::remove_node_from_group, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_groups", "args"), &ExtraTools::list_groups, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("export_project", "args"), &ExtraTools::export_project, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_export_preset", "args"), &ExtraTools::set_export_preset, DEFVAL(Dictionary()));
}
