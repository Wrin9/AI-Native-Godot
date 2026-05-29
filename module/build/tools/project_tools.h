/**************************************************************************/
/*  project_tools.h                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* ProjectTools — 项目管理 MCP 工具组                                      */
/*                                                                        */
/* 提供：项目设置、导入配置、版本信息等工具。                              */
/**************************************************************************/

#ifndef PROJECT_TOOLS_H
#define PROJECT_TOOLS_H

#include "../mcp_tool_base.h"

class ProjectTools : public MCPToolBase {
	GDCLASS(ProjectTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String get_project_info(const Dictionary &p_args);
	String get_project_settings(const Dictionary &p_args);
	String set_project_setting(const Dictionary &p_args);
	String get_godot_version(const Dictionary &p_args);
	String get_export_presets(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // PROJECT_TOOLS_H
