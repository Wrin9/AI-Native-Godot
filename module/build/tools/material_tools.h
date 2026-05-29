/**************************************************************************/
/*  material_tools.h                                                      */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MaterialTools — 材质操作 MCP 工具组                                     */
/*                                                                        */
/* 提供：材质属性读写、着色器编辑、资源操作等工具。                        */
/**************************************************************************/

#ifndef MATERIAL_TOOLS_H
#define MATERIAL_TOOLS_H

#include "../mcp_tool_base.h"

class MaterialTools : public MCPToolBase {
	GDCLASS(MaterialTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String get_material(const Dictionary &p_args);
	String set_material_property(const Dictionary &p_args);
	String get_shader_code(const Dictionary &p_args);
	String set_shader_code(const Dictionary &p_args);
	String list_materials(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // MATERIAL_TOOLS_H
