/**
 * physics_tools.cpp - 物理工具实现
 *
 * 碰撞形状、射线检测、导航区域和物理材质的创建和配置。
 * 支持矩形、圆形、胶囊体碰撞形状以及 RayCast2D。
 */

#include "physics_tools.h"

#include "mcp_tool_helpers.h"

#include "editor/editor_interface.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/2d/physics/collision_shape_2d.h"
#include "scene/2d/physics/ray_cast_2d.h"
#include "scene/2d/navigation/navigation_region_2d.h"
#include "scene/resources/2d/rectangle_shape_2d.h"
#include "scene/resources/2d/circle_shape_2d.h"
#include "scene/resources/2d/capsule_shape_2d.h"
#include "scene/resources/2d/segment_shape_2d.h"
#include "scene/resources/2d/concave_polygon_shape_2d.h"
#include "scene/resources/2d/convex_polygon_shape_2d.h"
#include "scene/resources/2d/navigation_polygon.h"
#include "scene/resources/physics_material.h"
#include "core/io/resource_saver.h"
#include "core/io/json.h"
#include "core/object/class_db.h"

// ============================================================
// 内部辅助方法
// ============================================================

/// 将 Variant 转为 Vector2（支持数组 [x,y]、字典 {x,y}、字符串 "x,y"）
static Vector2 _to_vector2(const Variant &p_value) {
	switch (p_value.get_type()) {
		case Variant::VECTOR2:
			return p_value;
		case Variant::ARRAY: {
			Array arr = p_value;
			if (arr.size() >= 2) {
				return Vector2(real_t(arr[0]), real_t(arr[1]));
			}
		} break;
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return Vector2(real_t(d.get("x", 0.0)), real_t(d.get("y", 0.0)));
		} break;
		case Variant::STRING: {
			String s = p_value;
			Vector<String> parts = s.split(",");
			if (parts.size() >= 2) {
				return Vector2(parts[0].to_float(), parts[1].to_float());
			}
		} break;
		default:
			break;
	}
	return Vector2();
}

// ============================================================
void PhysicsTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建碰撞形状节点
// ============================================================
String PhysicsTools::create_collision_shape(const Dictionary &p_args) {
	Node *scene_root = mcp_get_scene_root();
	if (!scene_root) {
		return R"json({"error": "No edited scene is open."})json";
	}

	// 获取父节点
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	String name = String(p_args.get("name", "CollisionShape2D")).strip_edges();
	if (name.is_empty()) {
		name = "CollisionShape2D";
	}

	// 创建碰撞形状节点
	CollisionShape2D *collision_shape = memnew(CollisionShape2D);
	collision_shape->set_name(name);

	// 创建形状资源
	String shape_type = String(p_args.get("shape_type", "rectangle")).strip_edges().to_lower();
	Ref<Shape2D> shape;

	if (shape_type == "rectangle" || shape_type == "rect") {
		Ref<RectangleShape2D> rect_shape;
		rect_shape.instantiate();
		real_t width = real_t(p_args.get("width", 20.0));
		real_t height = real_t(p_args.get("height", 20.0));
		rect_shape->set_size(Vector2(width, height));
		shape = rect_shape;
	} else if (shape_type == "circle") {
		Ref<CircleShape2D> circle_shape;
		circle_shape.instantiate();
		real_t radius = real_t(p_args.get("radius", 10.0));
		circle_shape->set_radius(radius);
		shape = circle_shape;
	} else if (shape_type == "capsule") {
		Ref<CapsuleShape2D> capsule_shape;
		capsule_shape.instantiate();
		real_t radius = real_t(p_args.get("radius", 10.0));
		real_t height = real_t(p_args.get("height", 30.0));
		capsule_shape->set_radius(radius);
		capsule_shape->set_height(height);
		shape = capsule_shape;
	} else if (shape_type == "segment") {
		Ref<SegmentShape2D> segment_shape;
		segment_shape.instantiate();
		// 解析起点和终点
		Variant from_var = p_args.get("from", Variant());
		Variant to_var = p_args.get("to", Variant());
		Vector2 from = _to_vector2(from_var);
		Vector2 to = _to_vector2(to_var);
		segment_shape->set_a(from);
		segment_shape->set_b(to);
		shape = segment_shape;
	} else {
		memdelete(collision_shape);
		return vformat(R"json({"error": "Unknown shape type '%s'. Supported: rectangle, circle, capsule, segment."})json", shape_type);
	}

	if (shape.is_valid()) {
		collision_shape->set_shape(shape);
	}

	// 设置禁用状态
	bool disabled = p_args.get("disabled", false);
	collision_shape->set_disabled(disabled);

	// 添加到场景树
	parent->add_child(collision_shape);
	collision_shape->set_owner(scene_root);

	Dictionary result;
	result["created"] = mcp_node_summary(collision_shape);
	result["parent_path"] = String(parent->get_path());
	result["shape_type"] = shape_type;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置碰撞形状数据
// ============================================================
String PhysicsTools::set_collision_shape_data(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	CollisionShape2D *collision_shape = Object::cast_to<CollisionShape2D>(node);
	if (!collision_shape) {
		return vformat(R"json({"error": "Node '%s' is not a CollisionShape2D."})json", node_path);
	}

	String shape_type = String(p_args.get("shape_type", "")).strip_edges().to_lower();
	Ref<Shape2D> shape;

	if (shape_type == "rectangle" || shape_type == "rect") {
		Ref<RectangleShape2D> rect_shape;
		rect_shape.instantiate();
		real_t width = real_t(p_args.get("width", 20.0));
		real_t height = real_t(p_args.get("height", 20.0));
		rect_shape->set_size(Vector2(width, height));
		shape = rect_shape;
	} else if (shape_type == "circle") {
		Ref<CircleShape2D> circle_shape;
		circle_shape.instantiate();
		real_t radius = real_t(p_args.get("radius", 10.0));
		circle_shape->set_radius(radius);
		shape = circle_shape;
	} else if (shape_type == "capsule") {
		Ref<CapsuleShape2D> capsule_shape;
		capsule_shape.instantiate();
		real_t radius = real_t(p_args.get("radius", 10.0));
		real_t height = real_t(p_args.get("height", 30.0));
		capsule_shape->set_radius(radius);
		capsule_shape->set_height(height);
		shape = capsule_shape;
	} else if (!shape_type.is_empty()) {
		return vformat(R"json({"error": "Unknown shape type '%s'. Supported: rectangle, circle, capsule."})json", shape_type);
	}

	if (shape.is_valid()) {
		collision_shape->set_shape(shape);
	}

	// 设置禁用状态
	if (p_args.has("disabled")) {
		collision_shape->set_disabled(bool(p_args["disabled"]));
	}

	// 设置单向碰撞
	if (p_args.has("one_way_collision")) {
		collision_shape->set_one_way_collision(bool(p_args["one_way_collision"]));
	}

	Dictionary result;
	result["node"] = mcp_node_summary(collision_shape);
	if (shape.is_valid()) {
		result["shape_type"] = shape_type;
	}
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建 RayCast2D 节点
// ============================================================
String PhysicsTools::create_ray_cast(const Dictionary &p_args) {
	Node *scene_root = mcp_get_scene_root();
	if (!scene_root) {
		return R"json({"error": "No edited scene is open."})json";
	}

	// 获取父节点
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	String name = String(p_args.get("name", "RayCast2D")).strip_edges();
	if (name.is_empty()) {
		name = "RayCast2D";
	}

	// 创建 RayCast2D 节点
	RayCast2D *ray_cast = memnew(RayCast2D);
	ray_cast->set_name(name);

	// 设置目标位置
	Variant target_var = p_args.get("target_position", Variant());
	Vector2 target = _to_vector2(target_var);
	if (target != Vector2()) {
		ray_cast->set_target_position(target);
	}

	// 设置其他属性
	if (p_args.has("enabled")) {
		ray_cast->set_enabled(bool(p_args["enabled"]));
	} else {
		ray_cast->set_enabled(true); // 默认启用
	}

	if (p_args.has("exclude_parent")) {
		ray_cast->set_exclude_parent_body(bool(p_args["exclude_parent"]));
	}

	if (p_args.has("collide_with_areas")) {
		ray_cast->set_collide_with_areas(bool(p_args["collide_with_areas"]));
	}

	if (p_args.has("collide_with_bodies")) {
		ray_cast->set_collide_with_bodies(bool(p_args["collide_with_bodies"]));
	}

	// 设置碰撞掩码
	if (p_args.has("collision_mask")) {
		ray_cast->set_collision_mask(int(p_args["collision_mask"]));
	}

	// 添加到场景树
	parent->add_child(ray_cast);
	ray_cast->set_owner(scene_root);

	Dictionary result;
	result["created"] = mcp_node_summary(ray_cast);
	result["parent_path"] = String(parent->get_path());
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建 NavigationRegion2D 节点
// ============================================================
String PhysicsTools::create_navigation_region(const Dictionary &p_args) {
	Node *scene_root = mcp_get_scene_root();
	if (!scene_root) {
		return R"json({"error": "No edited scene is open."})json";
	}

	// 获取父节点
	String parent_path = String(p_args.get("parent_path", "")).strip_edges();
	Node *parent = mcp_resolve_node_path(parent_path);
	if (!parent) {
		parent = scene_root;
	}

	String name = String(p_args.get("name", "NavigationRegion2D")).strip_edges();
	if (name.is_empty()) {
		name = "NavigationRegion2D";
	}

	// 创建 NavigationRegion2D 节点
	NavigationRegion2D *nav_region = memnew(NavigationRegion2D);
	nav_region->set_name(name);

	// 创建导航多边形
	Ref<NavigationPolygon> nav_polygon;
	nav_polygon.instantiate();

	// 解析顶点列表
	Variant vertices_var = p_args.get("vertices", Variant());
	if (vertices_var.get_type() == Variant::ARRAY) {
		Array arr = vertices_var;
		Vector<Vector2> vertices;
		for (int i = 0; i < arr.size(); i++) {
			Vector2 v = _to_vector2(arr[i]);
			vertices.push_back(v);
		}
		nav_polygon->set_vertices(vertices);

		// 解析多边形索引（顶点索引分组）
		Variant polygons_var = p_args.get("polygons", Variant());
		if (polygons_var.get_type() == Variant::ARRAY) {
			Array poly_arr = polygons_var;
			for (int i = 0; i < poly_arr.size(); i++) {
				Variant pv = poly_arr[i];
				if (pv.get_type() == Variant::ARRAY) {
					PackedInt32Array indices;
					Array idx_arr = pv;
					for (int j = 0; j < idx_arr.size(); j++) {
						indices.push_back(int(idx_arr[j]));
					}
					nav_polygon->add_polygon(indices);
				}
			}
		} else if (vertices.size() >= 3) {
			// 默认：所有顶点组成一个多边形
			PackedInt32Array indices;
			for (int i = 0; i < vertices.size(); i++) {
				indices.push_back(i);
			}
			nav_polygon->add_polygon(indices);
		}
	}

	nav_region->set_navigation_polygon(nav_polygon);

	// 设置启用状态
	if (p_args.has("enabled")) {
		nav_region->set_enabled(bool(p_args["enabled"]));
	}

	// 可选：设置导航地图
	if (p_args.has("navigation_layers")) {
		nav_region->set_navigation_layers(int(p_args["navigation_layers"]));
	}

	// 添加到场景树
	parent->add_child(nav_region);
	nav_region->set_owner(scene_root);

	Dictionary result;
	result["created"] = mcp_node_summary(nav_region);
	result["parent_path"] = String(parent->get_path());
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置物理材质
// ============================================================
String PhysicsTools::set_physics_material(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	// 检查节点是否支持 physics_material_override 属性
	bool has_physics_material = false;
	TypedArray<Dictionary> prop_list = node->call("get_property_list");
	for (int i = 0; i < prop_list.size(); i++) {
		Dictionary prop_info = prop_list[i];
		String prop_name = prop_info.get("name", "");
		if (prop_name == "physics_material_override") {
			has_physics_material = true;
			break;
		}
	}

	if (!has_physics_material) {
		return vformat(R"json({"error": "Node '%s' does not support physics material."})json", node_path);
	}

	// 创建或获取物理材质
	Ref<PhysicsMaterial> material;
	Variant existing = node->get("physics_material_override");
	if (existing.get_type() == Variant::OBJECT) {
		material = Object::cast_to<PhysicsMaterial>(existing);
	}
	if (material.is_null()) {
		material.instantiate();
	}

	// 设置摩擦力
	if (p_args.has("friction")) {
		material->set_friction(real_t(p_args["friction"]));
	}

	// 设置粗糙（摩擦力的便捷开关）
	if (p_args.has("rough")) {
		material->set_rough(bool(p_args["rough"]));
	}

	// 设置弹性
	if (p_args.has("bounce")) {
		material->set_bounce(real_t(p_args["bounce"]));
	}

	// 设置绝对弹性
	if (p_args.has("absorbent")) {
		material->set_absorbent(bool(p_args["absorbent"]));
	}

	// 应用材质
	node->set("physics_material_override", material);

	Dictionary result;
	result["node"] = mcp_node_summary(node);
	result["friction"] = material->get_friction();
	result["rough"] = material->is_rough();
	result["bounce"] = material->get_bounce();
	result["absorbent"] = material->is_absorbent();
	result["action"] = "physics_material_set";
	return JSON::stringify(result, "\t");
}

// ============================================================
void PhysicsTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_collision_shape", "args"), &PhysicsTools::create_collision_shape);
	ClassDB::bind_method(D_METHOD("set_collision_shape_data", "args"), &PhysicsTools::set_collision_shape_data);
	ClassDB::bind_method(D_METHOD("create_ray_cast", "args"), &PhysicsTools::create_ray_cast);
	ClassDB::bind_method(D_METHOD("create_navigation_region", "args"), &PhysicsTools::create_navigation_region);
	ClassDB::bind_method(D_METHOD("set_physics_material", "args"), &PhysicsTools::set_physics_material);
}
