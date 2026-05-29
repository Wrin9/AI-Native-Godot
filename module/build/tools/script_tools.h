/**************************************************************************/
/*  script_tools.h                                                        */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ScriptTools — 脚本操作 MCP 工具组                                       */
/*                                                                        */
/* 提供：脚本读写、方法列表、信号查询、脚本附加/分离等工具。                */
/**************************************************************************/

#ifndef SCRIPT_TOOLS_H
#define SCRIPT_TOOLS_H

#include "../mcp_tool_base.h"

class ScriptTools : public MCPToolBase {
	GDCLASS(ScriptTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String read_script(const Dictionary &p_args);
	String write_script(const Dictionary &p_args);
	String list_methods(const Dictionary &p_args);
	String list_signals(const Dictionary &p_args);
	String attach_script(const Dictionary &p_args);
	String detach_script(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // SCRIPT_TOOLS_H
