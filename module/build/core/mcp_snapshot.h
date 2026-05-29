/**************************************************************************/
/*  mcp_snapshot.h                                                        */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPSnapshot — 编辑器状态快照                                            */
/*                                                                        */
/* 采集当前编辑器场景树、选中节点、文件系统等状态，                         */
/* 供 MCP 客户端在 tool_call 前获取上下文。                                 */
/**************************************************************************/

#ifndef MCP_SNAPSHOT_H
#define MCP_SNAPSHOT_H

#include "core/object/object.h"
#include "core/variant/dictionary.h"

// 前向声明
class MCPEventBus;

class MCPSnapshot : public Object {
	GDCLASS(MCPSnapshot, Object);

public:
	/** 设置事件总线依赖 */
	void set_event_bus(MCPEventBus *p_bus) { _event_bus = p_bus; }

	/** 采集当前编辑器完整快照 */
	Dictionary capture() const;

	/** 采集场景树快照 */
	Dictionary capture_scene_tree() const;

	/** 采集文件系统快照 */
	Dictionary capture_filesystem() const;

	/** 采集选中节点快照 */
	Dictionary capture_selection() const;

	/** 采集项目设置快照 */
	Dictionary capture_project_settings() const;

	MCPSnapshot() = default;
	~MCPSnapshot() = default;

protected:
	static void _bind_methods();

private:
	MCPEventBus *_event_bus = nullptr;

	/** 递归采集节点信息 */
	Dictionary _capture_node(const Node *p_node, int p_depth = 0) const;
};

#endif // MCP_SNAPSHOT_H
