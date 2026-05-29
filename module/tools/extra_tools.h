/**
 * extra_tools.h - 额外工具
 *
 * 视口创建、信号断开、组管理、导出预设等辅助工具。
 */

#ifndef EXTRA_TOOLS_H
#define EXTRA_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;
class Node;

class ExtraTools : public RefCounted {
	GDCLASS(ExtraTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建 SubViewport
	String create_viewport(const Dictionary &p_args);

	// 断开节点信号
	String disconnect_node_signal(const Dictionary &p_args);

	// 获取节点信号连接
	String get_node_connections(const Dictionary &p_args);

	// 添加节点到组
	String add_node_to_group(const Dictionary &p_args);

	// 从组移除节点
	String remove_node_from_group(const Dictionary &p_args);

	// 列出所有组
	String list_groups(const Dictionary &p_args);

	// 导出项目
	String export_project(const Dictionary &p_args);

	// 设置导出预设
	String set_export_preset(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 递归收集组名
	void _collect_groups(Node *p_node, HashSet<StringName> &p_groups) const;
};

#endif // EXTRA_TOOLS_H
