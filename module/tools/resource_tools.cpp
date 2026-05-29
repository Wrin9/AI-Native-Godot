/**
 * resource_tools.cpp - 资源工具实现
 *
 * 资源的导入、查询、属性修改和列表遍历。
 */

#include "resource_tools.h"

#include "mcp_tool_helpers.h"
#include "editor/editor_interface.h"
#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/io/dir_access.h"
#include "core/object/class_db.h"

// ============================================================
void ResourceTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 导入资源（复制文件）
// ============================================================
String ResourceTools::import_resource(const Dictionary &p_args) {
	String source_path = p_args.get("source_path", "");
	String dest_path = p_args.get("dest_path", "");

	if (source_path.is_empty() || dest_path.is_empty()) {
		return R"json({"error": "'source_path' 和 'dest_path' 均为必填参数。"})json";
	}

	String src_abs = mcp_to_absolute(source_path);
	String dst_abs = mcp_to_absolute(dest_path);

	// 确保目标目录存在
	String parent_dir = mcp_normalize_path(dest_path).get_base_dir();
	if (!parent_dir.is_empty() && parent_dir != "res://" && parent_dir != "user://") {
		Error mk_err = DirAccess::make_dir_recursive_absolute(mcp_to_absolute(parent_dir));
		if (mk_err != OK) {
			return vformat(R"json({"error": "无法创建目标目录: %s"})json", parent_dir);
		}
	}

	Error err = DirAccess::copy_absolute(src_abs, dst_abs);
	if (err != OK) {
		return vformat(R"json({"error": "复制文件失败 (错误码 %d): %s -> %s"})json", (int)err, src_abs, dst_abs);
	}

	Dictionary result;
	result["source"] = source_path;
	result["destination"] = dest_path;
	result["success"] = true;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 获取资源信息
// ============================================================
String ResourceTools::get_resource_info(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' 为必填参数。"})json";
	}

	Ref<Resource> res = ResourceLoader::load(path);
	if (res.is_null()) {
		return vformat(R"json({"error": "无法加载资源: %s"})json", path);
	}

	Dictionary info;
	info["path"] = path;
	info["type"] = res->get_class();
	info["resource_name"] = res->get_name();
	info["resource_path"] = res->get_path();
	return JSON::stringify(info, "\t");
}

// ============================================================
// 设置资源属性并保存
// ============================================================
String ResourceTools::set_resource_property(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", ""));
	String property = p_args.get("property", "");
	Variant value = p_args.get("value", Variant());

	if (path.is_empty() || property.is_empty()) {
		return R"json({"error": "'path' 和 'property' 均为必填参数。"})json";
	}

	Ref<Resource> res = ResourceLoader::load(path);
	if (res.is_null()) {
		return vformat(R"json({"error": "无法加载资源: %s"})json", path);
	}

	res->set(property, value);

	Error err = ResourceSaver::save(res, path, ResourceSaver::FLAG_CHANGE_PATH);
	if (err != OK) {
		return vformat(R"json({"error": "保存资源失败 (错误码 %d): %s"})json", (int)err, path);
	}

	Dictionary result;
	result["path"] = path;
	result["property"] = property;
	result["value"] = value;
	result["success"] = true;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 列出目录中的资源（按类型过滤）
// ============================================================
String ResourceTools::list_resources(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", "res://"));
	String type_filter = p_args.get("type_filter", "all");

	Ref<DirAccess> dir = DirAccess::open(path);
	if (dir.is_null()) {
		return vformat(R"json({"error": "无法打开目录: %s"})json", path);
	}

	// 构建扩展名白名单
	HashSet<String> allowed_exts;
	if (type_filter == "image") {
		allowed_exts.insert("png");
		allowed_exts.insert("jpg");
		allowed_exts.insert("jpeg");
		allowed_exts.insert("webp");
		allowed_exts.insert("svg");
		allowed_exts.insert("bmp");
		allowed_exts.insert("tga");
		allowed_exts.insert("exr");
		allowed_exts.insert("hdr");
	} else if (type_filter == "audio") {
		allowed_exts.insert("mp3");
		allowed_exts.insert("wav");
		allowed_exts.insert("ogg");
		allowed_exts.insert("opus");
	} else if (type_filter == "model") {
		allowed_exts.insert("glb");
		allowed_exts.insert("gltf");
		allowed_exts.insert("fbx");
		allowed_exts.insert("obj");
		allowed_exts.insert("dae");
		allowed_exts.insert("blend");
	} else if (type_filter == "font") {
		allowed_exts.insert("ttf");
		allowed_exts.insert("otf");
		allowed_exts.insert("fnt");
		allowed_exts.insert("woff");
		allowed_exts.insert("woff2");
	}
	// "all" 不限制扩展名

	Error err = dir->list_dir_begin();
	if (err != OK) {
		return vformat(R"json({"error": "无法列出目录内容: %s"})json", path);
	}

	Array files;
	String file = dir->get_next();
	while (!file.is_empty()) {
		if (file == "." || file == "..") {
			file = dir->get_next();
			continue;
		}
		if (!dir->current_is_dir()) {
			String ext = file.get_extension().to_lower();
			if (allowed_exts.is_empty() || allowed_exts.has(ext)) {
				Dictionary entry;
				entry["name"] = file;
				entry["extension"] = ext;
				files.push_back(entry);
			}
		}
		file = dir->get_next();
	}
	dir->list_dir_end();

	Dictionary result;
	result["path"] = path;
	result["type_filter"] = type_filter;
	result["count"] = files.size();
	result["files"] = files;
	return JSON::stringify(result, "\t");
}

// ============================================================
void ResourceTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("import_resource", "args"), &ResourceTools::import_resource, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_resource_info", "args"), &ResourceTools::get_resource_info, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_resource_property", "args"), &ResourceTools::set_resource_property, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("list_resources", "args"), &ResourceTools::list_resources, DEFVAL(Dictionary()));
}
