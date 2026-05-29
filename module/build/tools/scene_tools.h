/**************************************************************************/
/*  scene_tools.h                                                         */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* SceneTools — 场景相关 MCP 工具组                                        */
/*                                                                        */
/* 提供：场景树查询、场景加载/保存、场景结构操作等工具。                    */
/**************************************************************************/

#ifndef SCENE_TOOLS_H
#define SCENE_TOOLS_H

#include "../mcp_tool_base.h"

class SceneTools : public MCPToolBase {
	GDCLASS(SceneTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	// ---- 工具方法 ----
	String get_tree(const Dictionary &p_args);
	String load_scene(const Dictionary &p_args);
	String save_scene(const Dictionary &p_args);
	String get_scene_properties(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // SCENE_TOOLS_H
