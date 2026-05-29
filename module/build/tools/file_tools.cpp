/**************************************************************************/
/*  file_tools.cpp                                                        */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* FileTools 实现 — 文件系统操作。                                         */
/**************************************************************************/

#include "file_tools.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"

void FileTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("read_file", "args"), &FileTools::read_file);
	ClassDB::bind_method(D_METHOD("write_file", "args"), &FileTools::write_file);
	ClassDB::bind_method(D_METHOD("list_dir", "args"), &FileTools::list_dir);
	ClassDB::bind_method(D_METHOD("delete_file", "args"), &FileTools::delete_file);
	ClassDB::bind_method(D_METHOD("make_dir", "args"), &FileTools::make_dir);
	ClassDB::bind_method(D_METHOD("file_exists", "args"), &FileTools::file_exists);
}

Array FileTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"file/read",
			"读取项目文件内容",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "文件路径（如 res://scripts/main.gd）" } } } } },
			},
			Callable(this, "read_file")));

	defs.append(make_tool_def(
			"file/write",
			"写入项目文件",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "文件路径" } } },
					{ "content", Dictionary{ { "type", "string" }, { "description", "文件内容" } } } } },
			},
			Callable(this, "write_file")));

	defs.append(make_tool_def(
			"file/list_dir",
			"列出目录内容",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "目录路径" } } },
					{ "recursive", Dictionary{ { "type", "boolean" }, { "description", "是否递归列出" } } } } },
			},
			Callable(this, "list_dir")));

	defs.append(make_tool_def(
			"file/delete",
			"删除文件",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "要删除的文件路径" } } } } },
			},
			Callable(this, "delete_file")));

	defs.append(make_tool_def(
			"file/make_dir",
			"创建目录",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "目录路径" } } } } },
			},
			Callable(this, "make_dir")));

	defs.append(make_tool_def(
			"file/exists",
			"检查文件是否存在",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "path", Dictionary{ { "type", "string" }, { "description", "文件路径" } } } } },
			},
			Callable(this, "file_exists")));

	return defs;
}

String FileTools::read_file(const Dictionary &p_args) {
	String path = p_args.get("path", "");
	// TODO: 使用 FileAccess 读取文件
	return vformat("{\"path\": \"%s\", \"content\": \"\"}", path);
}

String FileTools::write_file(const Dictionary &p_args) {
	// TODO: 使用 FileAccess 写入文件
	return "{\"success\": true}";
}

String FileTools::list_dir(const Dictionary &p_args) {
	// TODO: 使用 DirAccess 列出目录
	return "{\"entries\": []}";
}

String FileTools::delete_file(const Dictionary &p_args) {
	// TODO: 删除文件
	return "{\"success\": true}";
}

String FileTools::make_dir(const Dictionary &p_args) {
	// TODO: 创建目录
	return "{\"success\": true}";
}

String FileTools::file_exists(const Dictionary &p_args) {
	// TODO: 检查文件存在
	return "{\"exists\": false}";
}
