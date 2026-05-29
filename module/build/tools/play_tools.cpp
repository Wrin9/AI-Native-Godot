/**************************************************************************/
/*  play_tools.cpp                                                        */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* PlayTools 实现 — 项目运行控制。                                         */
/**************************************************************************/

#include "play_tools.h"

void PlayTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("play", "args"), &PlayTools::play);
	ClassDB::bind_method(D_METHOD("stop", "args"), &PlayTools::stop);
	ClassDB::bind_method(D_METHOD("pause", "args"), &PlayTools::pause);
	ClassDB::bind_method(D_METHOD("is_playing", "args"), &PlayTools::is_playing);
	ClassDB::bind_method(D_METHOD("get_game_output", "args"), &PlayTools::get_game_output);
}

Array PlayTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"play/start",
			"启动项目运行（F5）",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "play")));

	defs.append(make_tool_def(
			"play/stop",
			"停止项目运行",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "stop")));

	defs.append(make_tool_def(
			"play/pause",
			"暂停/恢复项目运行",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "pause")));

	defs.append(make_tool_def(
			"play/is_playing",
			"检查项目是否正在运行",
			Dictionary{ { "type", "object" }, { "properties", Dictionary{} } },
			Callable(this, "is_playing")));

	defs.append(make_tool_def(
			"play/get_output",
			"获取游戏运行输出日志",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "lines", Dictionary{ { "type", "integer" }, { "description", "获取最后 N 行日志" } } } } },
			},
			Callable(this, "get_game_output")));

	return defs;
}

String PlayTools::play(const Dictionary &p_args) {
	// TODO: 调用 EditorNode::get_singleton()->play_custom_scene()
	return "{\"success\": true, \"status\": \"running\"}";
}

String PlayTools::stop(const Dictionary &p_args) {
	// TODO: 调用 EditorNode::get_singleton()->stop_playing()
	return "{\"success\": true, \"status\": \"stopped\"}";
}

String PlayTools::pause(const Dictionary &p_args) {
	// TODO: 暂停/恢复
	return "{\"success\": true, \"status\": \"paused\"}";
}

String PlayTools::is_playing(const Dictionary &p_args) {
	// TODO: 检查运行状态
	return "{\"playing\": false}";
}

String PlayTools::get_game_output(const Dictionary &p_args) {
	// TODO: 获取运行日志
	return "{\"output\": []}";
}
