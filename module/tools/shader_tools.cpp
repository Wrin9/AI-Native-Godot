/**
 * shader_tools.cpp - Shader 工具实现
 *
 * Shader 资源的创建、代码编辑和参数设置。
 * 支持 Shader 文件创建、代码更新和 ShaderMaterial 参数配置。
 */

#include "shader_tools.h"

#include "mcp_tool_helpers.h"

#include "editor/editor_interface.h"
#include "editor/file_system/editor_file_system.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/material.h"
#include "scene/resources/shader.h"
#include "core/io/file_access.h"
#include "core/io/resource_saver.h"
#include "core/io/resource_loader.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/io/dir_access.h"

// ============================================================
void ShaderTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建 Shader 资源
// ============================================================
String ShaderTools::create_shader(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	// 检测 Shader 类型
	String shader_type = String(p_args.get("shader_type", "spatial")).strip_edges().to_lower();

	// 创建 Shader 对象
	Ref<Shader> shader;
	shader.instantiate();

	// 生成默认代码
	String default_code;
	if (shader_type == "spatial") {
		default_code = String(
				"shader_type spatial;\n"
				"\n"
				"void fragment() {\n"
				"\tALBEDO = vec3(1.0);\n"
				"}\n");
	} else if (shader_type == "canvas_item") {
		default_code = String(
				"shader_type canvas_item;\n"
				"\n"
				"void fragment() {\n"
				"\tCOLOR = vec4(1.0);\n"
				"}\n");
	} else if (shader_type == "particles") {
		default_code = String(
				"shader_type particles;\n"
				"\n"
				"void process() {\n"
				"\tCOLOR = vec4(1.0);\n"
				"\tVELOCITY = vec3(0.0);\n"
				"}\n");
	} else if (shader_type == "sky") {
		default_code = String(
				"shader_type sky;\n"
				"\n"
				"void sky() {\n"
				"\tCOLOR = vec3(0.0);\n"
				"}\n");
	} else if (shader_type == "fog") {
		default_code = String(
				"shader_type fog;\n"
				"\n"
				"void fog() {\n"
				"\tCOLOR = vec3(0.0);\n"
				"}\n");
	} else {
		return vformat(R"json({"error": "Unknown shader type '%s'. Supported: spatial, canvas_item, particles, sky, fog."})json", shader_type);
	}

	// 使用自定义代码覆盖默认（如果提供）
	String code = String(p_args.get("code", "")).strip_edges();
	if (!code.is_empty()) {
		shader->set_code(code);
	} else {
		shader->set_code(default_code);
	}

	// 确保父目录存在
	String parent_dir = path.get_base_dir();
	if (!parent_dir.is_empty() && parent_dir != "res://" && parent_dir != "user://") {
		DirAccess::make_dir_recursive_absolute(parent_dir);
	}

	// 保存 Shader 资源
	Error save_err = ResourceSaver::save(shader, path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"json({"error": "Failed to save shader to %s (code %d)."})json", path, (int)save_err);
	}

	// 刷新编辑器文件系统
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}

	Dictionary result;
	result["path"] = path;
	result["shader_type"] = shader_type;
	result["action"] = "created";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 Shader 代码
// ============================================================
String ShaderTools::set_shader_code(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	String code = String(p_args.get("code", ""));
	if (code.is_empty()) {
		return R"json({"error": "'code' is required."})json";
	}

	// 加载 Shader
	Ref<Shader> shader = ResourceLoader::load(path);
	if (shader.is_null()) {
		return vformat(R"json({"error": "Shader not found or invalid: %s"})json", path);
	}

	// 更新代码
	shader->set_code(code);

	// 保存
	Error save_err = ResourceSaver::save(shader, path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"json({"error": "Failed to save shader to %s (code %d)."})json", path, (int)save_err);
	}

	Dictionary result;
	result["path"] = path;
	result["code_length"] = code.length();
	result["action"] = "code_updated";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 Shader 参数（通过 ShaderMaterial）
// ============================================================
String ShaderTools::set_shader_param(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	// 获取 ShaderMaterial（通过 material 属性）
	Variant mat_var = node->get("material");
	ShaderMaterial *shader_mat = nullptr;

	if (mat_var.get_type() == Variant::OBJECT) {
		shader_mat = Object::cast_to<ShaderMaterial>(mat_var);
	}

	// 也尝试 material_override（3D）
	if (!shader_mat) {
		mat_var = node->get("material_override");
		if (mat_var.get_type() == Variant::OBJECT) {
			shader_mat = Object::cast_to<ShaderMaterial>(mat_var);
		}
	}

	if (!shader_mat) {
		// 可选：自动创建 ShaderMaterial
		String shader_path = mcp_normalize_path(p_args.get("shader_path", ""));
		if (shader_path.is_empty()) {
			return vformat(R"json({"error": "Node '%s' does not have a ShaderMaterial. Provide 'shader_path' to auto-create one."})json", node_path);
		}

		Ref<Shader> shader = ResourceLoader::load(shader_path);
		if (shader.is_null()) {
			return vformat(R"json({"error": "Shader not found or invalid: %s"})json", shader_path);
		}

		shader_mat = memnew(ShaderMaterial);
		shader_mat->set_shader(shader);

		// 设置到节点的 material 属性
		node->set("material", Ref<ShaderMaterial>(shader_mat));

		// ShaderMaterial 是 Resource，不需要 set_owner
	}

	// 设置参数
	String param_name = String(p_args.get("param", "")).strip_edges();
	if (param_name.is_empty()) {
		// 批量设置
		Variant params_var = p_args.get("params", Variant());
		if (params_var.get_type() != Variant::DICTIONARY) {
			return R"json({"error": "Either 'param' and 'value', or 'params' dictionary is required."})json";
		}
		Dictionary params = params_var;
		Array keys = params.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			shader_mat->set_shader_parameter(key, params[key]);
		}

		Dictionary result;
		result["node"] = mcp_node_summary(node);
		result["params"] = params;
		result["action"] = "shader_params_set";
		return JSON::stringify(result, "\t");
	}

	// 单个参数设置
	Variant value = p_args.get("value", Variant());
	shader_mat->set_shader_parameter(param_name, value);

	Dictionary result;
	result["node"] = mcp_node_summary(node);
	result["param"] = param_name;
	result["value"] = value;
	result["action"] = "shader_param_set";
	return JSON::stringify(result, "\t");
}

// ============================================================
void ShaderTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_shader", "args"), &ShaderTools::create_shader);
	ClassDB::bind_method(D_METHOD("set_shader_code", "args"), &ShaderTools::set_shader_code);
	ClassDB::bind_method(D_METHOD("set_shader_param", "args"), &ShaderTools::set_shader_param);
}
