/**************************************************************************/
/*  diagnostic_tools.h                                                    */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* DiagnosticTools — 诊断 MCP 工具组                                       */
/*                                                                        */
/* 提供：错误检查、性能分析、场景验证等工具。                              */
/**************************************************************************/

#ifndef DIAGNOSTIC_TOOLS_H
#define DIAGNOSTIC_TOOLS_H

#include "../mcp_tool_base.h"

class DiagnosticTools : public MCPToolBase {
	GDCLASS(DiagnosticTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String check_errors(const Dictionary &p_args);
	String validate_scene(const Dictionary &p_args);
	String get_performance_stats(const Dictionary &p_args);
	String get_debug_info(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // DIAGNOSTIC_TOOLS_H
