/**
 * resource_tools.h - 资源工具
 *
 * 资源的导入、查询、属性修改和列表遍历工具。
 */

#ifndef RESOURCE_TOOLS_H
#define RESOURCE_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class ResourceTools : public RefCounted {
	GDCLASS(ResourceTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 导入资源（复制文件）
	String import_resource(const Dictionary &p_args);

	// 获取资源信息
	String get_resource_info(const Dictionary &p_args);

	// 设置资源属性并保存
	String set_resource_property(const Dictionary &p_args);

	// 列出目录中的资源（按类型过滤）
	String list_resources(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // RESOURCE_TOOLS_H
