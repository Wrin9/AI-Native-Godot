/**************************************************************************/
/*  ui_tools.h                                                            */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* UITools — 编辑器 UI 操作 MCP 工具组                                     */
/*                                                                        */
/* 提供：面板控制、停靠窗口操作、编辑器布局等工具。                        */
/**************************************************************************/

#ifndef UI_TOOLS_H
#define UI_TOOLS_H

#include "../mcp_tool_base.h"

class UITools : public MCPToolBase {
	GDCLASS(UITools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String get_editor_layout(const Dictionary &p_args);
	String screenshot_editor(const Dictionary &p_args);
	String get_dock_info(const Dictionary &p_args);
	String select_node_in_tree(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // UI_TOOLS_H
