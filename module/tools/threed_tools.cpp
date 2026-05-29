/**
 * threed_tools.cpp - 3D 场景工具实现
 *
 * 音频播放器、粒子、网格实例、灯光、相机和环境创建。
 */

#include "threed_tools.h"

#include "mcp_tool_helpers.h"
#include "editor/editor_interface.h"
#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "scene/audio/audio_stream_player.h"
#include "servers/audio/audio_stream.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/3d/gpu_particles_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/world_environment.h"
#include "scene/resources/environment.h"

// ============================================================
void ThreeDTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建音频播放器
// ============================================================
String ThreeDTools::create_audio_player(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "AudioStreamPlayer");
	String stream_path = mcp_normalize_path(p_args.get("stream_path", ""));
	bool autoplay = p_args.get("autoplay", false);

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	AudioStreamPlayer *player = memnew(AudioStreamPlayer);
	player->set_name(name);

	if (!stream_path.is_empty()) {
		Ref<AudioStream> stream = ResourceLoader::load(stream_path);
		if (stream.is_valid()) {
			player->set_stream(stream);
		}
	}
	player->set_autoplay(autoplay);

	parent->add_child(player);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		player->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(player);
	result["stream_path"] = stream_path;
	result["autoplay"] = autoplay;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建粒子系统
// ============================================================
String ThreeDTools::create_particles(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "GPUParticles");
	String particle_type = p_args.get("particle_type", "GPUParticles3D");
	int amount = p_args.get("amount", 100);

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	Node *particles = nullptr;
	if (particle_type == "GPUParticles2D") {
		particles = memnew(GPUParticles2D);
		Object::cast_to<GPUParticles2D>(particles)->set_amount(amount);
	} else {
		particles = memnew(GPUParticles3D);
		Object::cast_to<GPUParticles3D>(particles)->set_amount(amount);
	}

	particles->set_name(name);

	parent->add_child(particles);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		particles->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(particles);
	result["particle_type"] = particle_type;
	result["amount"] = amount;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建网格实例
// ============================================================
String ThreeDTools::create_mesh_instance(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "MeshInstance3D");
	String mesh_path = mcp_normalize_path(p_args.get("mesh_path", ""));

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_name(name);

	if (!mesh_path.is_empty()) {
		Ref<Mesh> mesh = ResourceLoader::load(mesh_path);
		if (mesh.is_valid()) {
			mesh_instance->set_mesh(mesh);
		}
	}

	parent->add_child(mesh_instance);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		mesh_instance->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(mesh_instance);
	result["mesh_path"] = mesh_path;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建灯光
// ============================================================
String ThreeDTools::create_light(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "Light3D");
	String light_type = p_args.get("light_type", "OmniLight3D");
	Color color = p_args.get("color", Color(1, 1, 1));
	float energy = p_args.get("energy", 1.0);

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	Light3D *light = nullptr;
	if (light_type == "DirectionalLight3D") {
		light = memnew(DirectionalLight3D);
	} else if (light_type == "SpotLight3D") {
		light = memnew(SpotLight3D);
	} else {
		light = memnew(OmniLight3D);
	}

	light->set_name(name);
	light->set_color(color);
	light->set_param(Light3D::PARAM_ENERGY, energy);

	parent->add_child(light);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		light->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(light);
	result["light_type"] = light_type;
	result["color"] = color;
	result["energy"] = energy;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建 3D 相机
// ============================================================
String ThreeDTools::create_camera_3d(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "Camera3D");
	float fov = p_args.get("fov", 75.0);
	float near_clip = p_args.get("near", 0.05);
	float far_clip = p_args.get("far", 4000.0);

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	Camera3D *camera = memnew(Camera3D);
	camera->set_name(name);
	camera->set_fov(fov);
	camera->set_near(near_clip);
	camera->set_far(far_clip);

	parent->add_child(camera);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		camera->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(camera);
	result["fov"] = fov;
	result["near"] = near_clip;
	result["far"] = far_clip;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置环境（WorldEnvironment + Environment）
// ============================================================
String ThreeDTools::set_environment(const Dictionary &p_args) {
	String parent_path = p_args.get("parent_path", ".");
	String name = p_args.get("name", "WorldEnvironment");
	Color sky_color = p_args.get("sky_color", Color(0.2, 0.5, 0.9));
	Color ambient_color = p_args.get("ambient_color", Color(0.1, 0.1, 0.1));

	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		return vformat(R"json({"error": "找不到父节点: %s"})json", parent_path);
	}

	WorldEnvironment *world_env = memnew(WorldEnvironment);
	world_env->set_name(name);

	Ref<Environment> env;
	env.instantiate();
	env->set_background(Environment::BG_COLOR);
	env->set_bg_color(sky_color);
	env->set_ambient_source(Environment::AMBIENT_SOURCE_COLOR);
	env->set_ambient_light_color(ambient_color);
	world_env->set_environment(env);

	parent->add_child(world_env);
	Node *scene_root = mcp_get_scene_root();
	if (scene_root) {
		world_env->set_owner(scene_root);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(world_env);
	result["sky_color"] = sky_color;
	result["ambient_color"] = ambient_color;
	return JSON::stringify(result, "\t");
}

// ============================================================
void ThreeDTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_audio_player", "args"), &ThreeDTools::create_audio_player, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_particles", "args"), &ThreeDTools::create_particles, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_mesh_instance", "args"), &ThreeDTools::create_mesh_instance, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_light", "args"), &ThreeDTools::create_light, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("create_camera_3d", "args"), &ThreeDTools::create_camera_3d, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("set_environment", "args"), &ThreeDTools::set_environment, DEFVAL(Dictionary()));
}
