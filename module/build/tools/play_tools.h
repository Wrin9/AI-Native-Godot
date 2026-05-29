/**************************************************************************/
/*  play_tools.h                                                          */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* PlayTools — 运行控制 MCP 工具组                                         */
/*                                                                        */
/* 提供：播放/暂停/停止项目运行、调试控制等工具。                          */
/**************************************************************************/

#ifndef PLAY_TOOLS_H
#define PLAY_TOOLS_H

#include "../mcp_tool_base.h"

class PlayTools : public MCPToolBase {
	GDCLASS(PlayTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String play(const Dictionary &p_args);
	String stop(const Dictionary &p_args);
	String pause(const Dictionary &p_args);
	String is_playing(const Dictionary &p_args);
	String get_game_output(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // PLAY_TOOLS_H
