/**************************************************************************/
/*  mcp_snapshot.cpp                                                      */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPSnapshot 实现 — 采集编辑器运行时状态。                                */
/**************************************************************************/

#include "core/mcp_snapshot.h"

#include "editor/editor_node.h"
#include "editor/editor_file_system.h"
#include "scene/main/node.h"
#include "scene/main/window.h"

void MCPSnapshot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("capture"), &MCPSnapshot::capture);
	ClassDB::bind_method(D_METHOD("capture_scene_tree"), &MCPSnapshot::capture_scene_tree);
	ClassDB::bind_method(D_METHOD("capture_filesystem"), &MCPSnapshot::capture_filesystem);
	ClassDB::bind_method(D_METHOD("capture_selection"), &MCPSnapshot::capture_selection);
	ClassDB::bind_method(D_METHOD("capture_project_settings"), &MCPSnapshot::capture_project_settings);
}

Dictionary MCPSnapshot::capture() const {
	Dictionary snapshot;
	snapshot["scene_tree"] = capture_scene_tree();
	snapshot["filesystem"] = capture_filesystem();
	snapshot["selection"] = capture_selection();
	snapshot["project_settings"] = capture_project_settings();
	snapshot["timestamp"] = Time::get_singleton()->get_ticks_msec();
	return snapshot;
}

Dictionary MCPSnapshot::capture_scene_tree() const {
	Dictionary result;

	// 获取编辑器当前编辑的场景根节点
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	if (!root) {
		result["root"] = Variant();
		result["node_count"] = 0;
		return result;
	}

	result["root"] = _capture_node(root);
	return result;
}

Dictionary MCPSnapshot::capture_filesystem() const {
	Dictionary result;

	// TODO: 采集编辑器文件系统状态
	// - 扫描目录结构
	// - 列出所有资源文件
	// - 记录最近修改时间

	result["scan_complete"] = false;
	return result;
}

Dictionary MCPSnapshot::capture_selection() const {
	Dictionary result;

	// TODO: 采集当前选中的节点列表
	// - 节点路径
	// - 节点类型
	// - 关键属性值

	result["selected_nodes"] = Array();
	return result;
}

Dictionary MCPSnapshot::capture_project_settings() const {
	Dictionary result;

	// TODO: 采集项目设置
	// - 项目名称
	// - 渲染器类型
	// - 窗口大小
	// - 自定义设置

	result["project_name"] = "";
	return result;
}

Dictionary MCPSnapshot::_capture_node(const Node *p_node, int p_depth) const {
	if (!p_node) {
		return Dictionary();
	}

	Dictionary node_info;
	node_info["name"] = p_node->get_name();
	node_info["type"] = p_node->get_class();
	node_info["path"] = String(p_node->get_path());

	// 递归采集子节点（限制深度防止性能问题）
	if (p_depth < 10) {
		Array children;
		for (int i = 0; i < p_node->get_child_count(); i++) {
			children.append(_capture_node(p_node->get_child(i), p_depth + 1));
		}
		node_info["children"] = children;
	}

	return node_info;
}
