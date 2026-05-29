/**************************************************************************/
/*  node_tools.h                                                          */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* NodeTools — 节点操作 MCP 工具组                                         */
/*                                                                        */
/* 提供：节点增删查改、属性读写、信号操作等工具。                          */
/**************************************************************************/

#ifndef NODE_TOOLS_H
#define NODE_TOOLS_H

#include "../mcp_tool_base.h"

class NodeTools : public MCPToolBase {
	GDCLASS(NodeTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String create_node(const Dictionary &p_args);
	String delete_node(const Dictionary &p_args);
	String get_node(const Dictionary &p_args);
	String set_node_property(const Dictionary &p_args);
	String get_node_property(const Dictionary &p_args);
	String move_node(const Dictionary &p_args);
	String duplicate_node(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // NODE_TOOLS_H
