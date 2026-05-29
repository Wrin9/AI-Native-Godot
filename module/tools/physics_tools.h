/**
 * physics_tools.h - 物理工具
 *
 * 碰撞形状、射线检测、导航区域和物理材质的创建和配置。
 */
#ifndef PHYSICS_TOOLS_H
#define PHYSICS_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class PhysicsTools : public RefCounted {
	GDCLASS(PhysicsTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建碰撞形状节点
	String create_collision_shape(const Dictionary &p_args);

	// 设置碰撞形状数据
	String set_collision_shape_data(const Dictionary &p_args);

	// 创建 RayCast2D 节点
	String create_ray_cast(const Dictionary &p_args);

	// 创建 NavigationRegion2D 节点
	String create_navigation_region(const Dictionary &p_args);

	// 设置物理材质
	String set_physics_material(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // PHYSICS_TOOLS_H
