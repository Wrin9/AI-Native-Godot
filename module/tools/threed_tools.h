/**
 * threed_tools.h - 3D 场景工具
 *
 * 音频播放器、粒子、网格实例、灯光、相机和环境创建工具。
 */

#ifndef THREED_TOOLS_H
#define THREED_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class ThreeDTools : public RefCounted {
	GDCLASS(ThreeDTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建音频播放器
	String create_audio_player(const Dictionary &p_args);

	// 创建粒子系统
	String create_particles(const Dictionary &p_args);

	// 创建网格实例
	String create_mesh_instance(const Dictionary &p_args);

	// 创建灯光
	String create_light(const Dictionary &p_args);

	// 创建 3D 相机
	String create_camera_3d(const Dictionary &p_args);

	// 设置环境
	String set_environment(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // THREED_TOOLS_H
