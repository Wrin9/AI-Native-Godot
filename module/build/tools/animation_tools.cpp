/**************************************************************************/
/*  animation_tools.cpp                                                   */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* AnimationTools 实现 — 动画关键帧操作。                                  */
/**************************************************************************/

#include "animation_tools.h"

void AnimationTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_animations", "args"), &AnimationTools::get_animations);
	ClassDB::bind_method(D_METHOD("add_keyframe", "args"), &AnimationTools::add_keyframe);
	ClassDB::bind_method(D_METHOD("remove_keyframe", "args"), &AnimationTools::remove_keyframe);
	ClassDB::bind_method(D_METHOD("get_animation_player_info", "args"), &AnimationTools::get_animation_player_info);
	ClassDB::bind_method(D_METHOD("play_animation", "args"), &AnimationTools::play_animation);
}

Array AnimationTools::get_tool_definitions() const {
	Array defs;

	defs.append(make_tool_def(
			"animation/list",
			"列出 AnimationPlayer 节点中的所有动画",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "AnimationPlayer 节点路径" } } } } },
			},
			Callable(this, "get_animations")));

	defs.append(make_tool_def(
			"animation/add_keyframe",
			"添加动画关键帧",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "AnimationPlayer 节点路径" } } },
					{ "animation", Dictionary{ { "type", "string" }, { "description", "动画名称" } } },
					{ "track", Dictionary{ { "type", "string" }, { "description", "轨道路径" } } },
					{ "time", Dictionary{ { "type", "number" }, { "description", "时间（秒）" } } },
					{ "value", Dictionary{ { "description", "关键帧值" } } } } },
			},
			Callable(this, "add_keyframe")));

	defs.append(make_tool_def(
			"animation/remove_keyframe",
			"移除动画关键帧",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" } } },
					{ "animation", Dictionary{ { "type", "string" } } },
					{ "track", Dictionary{ { "type", "string" } } },
					{ "time", Dictionary{ { "type", "number" } } } } },
			},
			Callable(this, "remove_keyframe")));

	defs.append(make_tool_def(
			"animation/get_player_info",
			"获取 AnimationPlayer 详细信息",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" }, { "description", "AnimationPlayer 节点路径" } } } } },
			},
			Callable(this, "get_animation_player_info")));

	defs.append(make_tool_def(
			"animation/play",
			"播放指定动画",
			Dictionary{
				{ "type", "object" },
				{ "properties", Dictionary{
					{ "node_path", Dictionary{ { "type", "string" } } },
					{ "animation", Dictionary{ { "type", "string" } } },
					{ "from_start", Dictionary{ { "type", "boolean" }, { "description", "是否从头开始" } } } } },
			},
			Callable(this, "play_animation")));

	return defs;
}

String AnimationTools::get_animations(const Dictionary &p_args) {
	// TODO: 列出动画
	return "{\"animations\": []}";
}

String AnimationTools::add_keyframe(const Dictionary &p_args) {
	// TODO: 添加关键帧
	return "{\"success\": true}";
}

String AnimationTools::remove_keyframe(const Dictionary &p_args) {
	// TODO: 移除关键帧
	return "{\"success\": true}";
}

String AnimationTools::get_animation_player_info(const Dictionary &p_args) {
	// TODO: 获取动画播放器信息
	return "{\"info\": {}}";
}

String AnimationTools::play_animation(const Dictionary &p_args) {
	// TODO: 播放动画
	return "{\"success\": true}";
}
