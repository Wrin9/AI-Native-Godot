/**************************************************************************/
/*  animation_tools.h                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* AnimationTools — 动画操作 MCP 工具组                                    */
/*                                                                        */
/* 提供：动画播放器操作、关键帧编辑、动画树控制等工具。                    */
/**************************************************************************/

#ifndef ANIMATION_TOOLS_H
#define ANIMATION_TOOLS_H

#include "../mcp_tool_base.h"

class AnimationTools : public MCPToolBase {
	GDCLASS(AnimationTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String get_animations(const Dictionary &p_args);
	String add_keyframe(const Dictionary &p_args);
	String remove_keyframe(const Dictionary &p_args);
	String get_animation_player_info(const Dictionary &p_args);
	String play_animation(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // ANIMATION_TOOLS_H
