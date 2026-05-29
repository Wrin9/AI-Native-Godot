/**
 * tilemap_tools.cpp - TileMap/TileSet 工具实现
 *
 * TileMap 编辑和 TileSet 资源管理。
 * 支持瓦片设置、地形连接、图层清除、TileSet 创建和碰撞配置。
 */

#include "tilemap_tools.h"

#include "mcp_tool_helpers.h"

#include "editor/editor_interface.h"
#include "editor/file_system/editor_file_system.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/main/window.h"
#include "scene/main/scene_tree.h"
#include "scene/2d/tile_map.h"
#include "scene/resources/2d/tile_set.h"
#include "core/io/resource_saver.h"
#include "core/io/resource_loader.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/io/dir_access.h"

// ============================================================
void TileMapTools::set_editor_plugin(EditorPlugin *p_plugin) {
	_plugin = p_plugin;
}

// ============================================================
// 创建 TileMap 节点
// ============================================================
String TileMapTools::create_tile_map(const Dictionary &p_args) {
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

	String name = String(p_args.get("name", "TileMap")).strip_edges();
	if (name.is_empty()) {
		name = "TileMap";
	}

	// 创建 TileMap 节点
	TileMap *tile_map = memnew(TileMap);
	tile_map->set_name(name);

	// 可选：设置 TileSet
	String tileset_path = mcp_normalize_path(p_args.get("tileset_path", ""));
	if (!tileset_path.is_empty()) {
		Ref<TileSet> tile_set = ResourceLoader::load(tileset_path);
		if (tile_set.is_valid()) {
			tile_map->set_tileset(tile_set);
		}
	}

	// 设置图层
	int layers = int(p_args.get("layers", 1));
	if (layers > 1) {
		for (int i = tile_map->get_layers_count(); i < layers; i++) { tile_map->add_layer(tile_map->get_layers_count()); }
	}

	// 添加到场景树
	parent->add_child(tile_map);
	tile_map->set_owner(scene_root);

	Dictionary result;
	result["created"] = mcp_node_summary(tile_map);
	result["parent_path"] = String(parent->get_path());
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置单个瓦片
// ============================================================
String TileMapTools::set_cell(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	TileMap *tile_map = Object::cast_to<TileMap>(node);
	if (!tile_map) {
		return vformat(R"json({"error": "Node '%s' is not a TileMap."})json", node_path);
	}

	// 解析参数
	int layer = int(p_args.get("layer", 0));

	// 解析坐标
	Vector2i coords;
	Variant coords_var = p_args.get("coords", Variant());
	if (coords_var.get_type() == Variant::VECTOR2I) {
		coords = coords_var;
	} else if (coords_var.get_type() == Variant::DICTIONARY) {
		Dictionary d = coords_var;
		coords = Vector2i(int(d.get("x", 0)), int(d.get("y", 0)));
	} else if (coords_var.get_type() == Variant::ARRAY) {
		Array arr = coords_var;
		if (arr.size() >= 2) {
			coords = Vector2i(int(arr[0]), int(arr[1]));
		}
	}

	int source_id = int(p_args.get("source_id", 0));

	// 解析图集坐标
	Vector2i atlas_coords;
	Variant atlas_var = p_args.get("atlas_coords", Variant());
	if (atlas_var.get_type() == Variant::VECTOR2I) {
		atlas_coords = atlas_var;
	} else if (atlas_var.get_type() == Variant::DICTIONARY) {
		Dictionary d = atlas_var;
		atlas_coords = Vector2i(int(d.get("x", 0)), int(d.get("y", 0)));
	} else if (atlas_var.get_type() == Variant::ARRAY) {
		Array arr = atlas_var;
		if (arr.size() >= 2) {
			atlas_coords = Vector2i(int(arr[0]), int(arr[1]));
		}
	}

	int alternative_tile = int(p_args.get("alternative_tile", 0));

	// 设置瓦片
	tile_map->set_cell(layer, coords, source_id, atlas_coords, alternative_tile);

	Dictionary result;
	result["node"] = mcp_node_summary(tile_map);
	result["layer"] = layer;
	Dictionary coords_dict;
	coords_dict["x"] = coords.x;
	coords_dict["y"] = coords.y;
	result["coords"] = coords_dict;
	result["source_id"] = source_id;
	Dictionary atlas_dict;
	atlas_dict["x"] = atlas_coords.x;
	atlas_dict["y"] = atlas_coords.y;
	result["atlas_coords"] = atlas_dict;
	result["alternative_tile"] = alternative_tile;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置地形连接（批量设置瓦片并连接地形）
// ============================================================
String TileMapTools::set_cells_terrain_connect(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	TileMap *tile_map = Object::cast_to<TileMap>(node);
	if (!tile_map) {
		return vformat(R"json({"error": "Node '%s' is not a TileMap."})json", node_path);
	}

	int layer = int(p_args.get("layer", 0));
	int source_id = int(p_args.get("source_id", 0));
	int terrain_set = int(p_args.get("terrain_set", 0));
	int terrain = int(p_args.get("terrain", 0));

	// 解析坐标列表
	TypedArray<Vector2i> cells;
	Variant cells_var = p_args.get("cells", Variant());
	if (cells_var.get_type() == Variant::ARRAY) {
		Array arr = cells_var;
		for (int i = 0; i < arr.size(); i++) {
			Variant v = arr[i];
			if (v.get_type() == Variant::VECTOR2I) {
				cells.push_back(v);
			} else if (v.get_type() == Variant::DICTIONARY) {
				Dictionary d = v;
				cells.push_back(Vector2i(int(d.get("x", 0)), int(d.get("y", 0))));
			} else if (v.get_type() == Variant::ARRAY) {
				Array a = v;
				if (a.size() >= 2) {
					cells.push_back(Vector2i(int(a[0]), int(a[1])));
				}
			}
		}
	}

	if (cells.is_empty()) {
		return R"json({"error": "'cells' must be a non-empty array of coordinates."})json";
	}

	tile_map->set_cells_terrain_connect(layer, cells, source_id, terrain_set, terrain);

	Dictionary result;
	result["node"] = mcp_node_summary(tile_map);
	result["layer"] = layer;
	result["source_id"] = source_id;
	result["terrain_set"] = terrain_set;
	result["terrain"] = terrain;
	result["cells_count"] = cells.size();
	return JSON::stringify(result, "\t");
}

// ============================================================
// 清除图层
// ============================================================
String TileMapTools::clear_layer(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	TileMap *tile_map = Object::cast_to<TileMap>(node);
	if (!tile_map) {
		return vformat(R"json({"error": "Node '%s' is not a TileMap."})json", node_path);
	}

	int layer = int(p_args.get("layer", 0));
	if (layer < 0 || layer >= tile_map->get_layers_count()) {
		return vformat(R"json({"error": "Invalid layer %d. TileMap has %d layers."})json", layer, tile_map->get_layers_count());
	}

	tile_map->clear_layer(layer);

	Dictionary result;
	result["node"] = mcp_node_summary(tile_map);
	result["layer"] = layer;
	result["action"] = "cleared";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 获取 TileMap 数据
// ============================================================
String TileMapTools::get_tile_map_data(const Dictionary &p_args) {
	String node_path = String(p_args.get("node_path", "")).strip_edges();
	if (node_path.is_empty()) {
		return R"json({"error": "'node_path' is required."})json";
	}

	Node *node = mcp_resolve_node_path(node_path);
	if (!node) {
		return vformat(R"json({"error": "Node not found: %s"})json", node_path);
	}

	TileMap *tile_map = Object::cast_to<TileMap>(node);
	if (!tile_map) {
		return vformat(R"json({"error": "Node '%s' is not a TileMap."})json", node_path);
	}

	int layer = int(p_args.get("layer", 0));
	if (layer < 0 || layer >= tile_map->get_layers_count()) {
		return vformat(R"json({"error": "Invalid layer %d. TileMap has %d layers."})json", layer, tile_map->get_layers_count());
	}

	// 获取所有使用的单元格
	TypedArray<Vector2i> used_cells = tile_map->get_used_cells(layer);

	Array cells_data;
	for (int i = 0; i < used_cells.size(); i++) {
		Vector2i coords = used_cells[i];

		Dictionary cell_info;
		Dictionary coords_dict;
		coords_dict["x"] = coords.x;
		coords_dict["y"] = coords.y;
		cell_info["coords"] = coords_dict;
		cell_info["source_id"] = tile_map->get_cell_source_id(layer, coords);

		Vector2i atlas = tile_map->get_cell_atlas_coords(layer, coords);
		Dictionary atlas_dict;
		atlas_dict["x"] = atlas.x;
		atlas_dict["y"] = atlas.y;
		cell_info["atlas_coords"] = atlas_dict;

		cell_info["alternative_tile"] = tile_map->get_cell_alternative_tile(layer, coords);
		cells_data.push_back(cell_info);
	}

	Dictionary result;
	result["node"] = mcp_node_summary(tile_map);
	result["layer"] = layer;
	result["layers_count"] = tile_map->get_layers_count();
	result["cells_count"] = cells_data.size();
	result["cells"] = cells_data;
	return JSON::stringify(result, "\t");
}

// ============================================================
// 创建 TileSet 资源
// ============================================================
String TileMapTools::create_tile_set(const Dictionary &p_args) {
	String path = mcp_normalize_path(p_args.get("path", ""));
	if (path.is_empty()) {
		return R"json({"error": "'path' is required."})json";
	}

	// 创建 TileSet 资源
	Ref<TileSet> tile_set;
	tile_set.instantiate();

	// 设置瓦片大小
	int tile_size_x = int(p_args.get("tile_size_x", 16));
	int tile_size_y = int(p_args.get("tile_size_y", 16));
	tile_set->set_tile_size(Vector2i(tile_size_x, tile_size_y));

	// 确保父目录存在
	String parent_dir = path.get_base_dir();
	if (!parent_dir.is_empty() && parent_dir != "res://" && parent_dir != "user://") {
		DirAccess::make_dir_recursive_absolute(parent_dir);
	}

	// 保存资源
	Error save_err = ResourceSaver::save(tile_set, path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"json({"error": "Failed to save TileSet to %s (code %d)."})json", path, (int)save_err);
	}

	// 刷新编辑器文件系统
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}

	Dictionary result;
	result["path"] = path;
	Dictionary size_dict;
	size_dict["x"] = tile_size_x;
	size_dict["y"] = tile_size_y;
	result["tile_size"] = size_dict;
	result["action"] = "created";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 添加图集源到 TileSet
// ============================================================
String TileMapTools::add_tile_atlas_source(const Dictionary &p_args) {
	String tileset_path = mcp_normalize_path(p_args.get("tileset_path", ""));
	if (tileset_path.is_empty()) {
		return R"json({"error": "'tileset_path' is required."})json";
	}

	// 加载 TileSet
	Ref<TileSet> tile_set = ResourceLoader::load(tileset_path);
	if (tile_set.is_null()) {
		return vformat(R"json({"error": "TileSet not found or invalid: %s"})json", tileset_path);
	}

	String texture_path = mcp_normalize_path(p_args.get("texture_path", ""));
	if (texture_path.is_empty()) {
		return R"json({"error": "'texture_path' is required."})json";
	}

	// 加载纹理
	Ref<Texture2D> texture = ResourceLoader::load(texture_path);
	if (texture.is_null()) {
		return vformat(R"json({"error": "Texture not found or invalid: %s"})json", texture_path);
	}

	// 创建图集源
	TileSetAtlasSource *atlas_source = memnew(TileSetAtlasSource);
	atlas_source->set_texture(texture);

	// 设置瓦片大小
	int tile_size_x = int(p_args.get("tile_size_x", 16));
	int tile_size_y = int(p_args.get("tile_size_y", 16));
	atlas_source->set_texture_region_size(Vector2i(tile_size_x, tile_size_y));

	// 设置间距
	int separation_x = int(p_args.get("separation_x", 0));
	int separation_y = int(p_args.get("separation_y", 0));
	if (separation_x > 0 || separation_y > 0) {
		atlas_source->set_separation(Vector2i(separation_x, separation_y));
	}

	// 设置边距
	int margin_x = int(p_args.get("margin_x", 0));
	int margin_y = int(p_args.get("margin_y", 0));
	if (margin_x > 0 || margin_y > 0) {
		atlas_source->set_margins(Vector2i(margin_x, margin_y));
	}

	// 计算并创建瓦片区域
	if (texture.is_valid()) {
		int tex_w = texture->get_width();
		int tex_h = texture->get_height();
		// 考虑边距
		int effective_w = tex_w - margin_x;
		int effective_h = tex_h - margin_y;

		int cols = (effective_w + separation_x) / (tile_size_x + separation_x);
		int rows = (effective_h + separation_y) / (tile_size_y + separation_y);
		cols = MAX(cols, 1);
		rows = MAX(rows, 1);

		// 创建瓦片
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				atlas_source->create_tile(Vector2i(c, r));
			}
		}
	}

	// 添加到 TileSet
	int source_id = tile_set->get_next_source_id();
	tile_set->add_source(atlas_source, source_id);
	tile_set->set_source_id(tile_set->get_source_count() - 1, source_id);

	// 保存
	Error save_err = ResourceSaver::save(tile_set, tileset_path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"json({"error": "Failed to save TileSet to %s (code %d)."})json", tileset_path, (int)save_err);
	}

	// 刷新编辑器文件系统
	EditorInterface *editor = EditorInterface::get_singleton();
	if (editor) {
		EditorFileSystem *efs = editor->get_resource_filesystem();
		if (efs) {
			efs->scan();
		}
	}

	Dictionary result;
	result["tileset_path"] = tileset_path;
	result["source_id"] = source_id;
	result["texture_path"] = texture_path;
	Dictionary size_dict;
	size_dict["x"] = tile_size_x;
	size_dict["y"] = tile_size_y;
	result["tile_size"] = size_dict;
	result["action"] = "atlas_source_added";
	return JSON::stringify(result, "\t");
}

// ============================================================
// 设置 TileSet 碰撞
// ============================================================
String TileMapTools::set_tile_set_collision(const Dictionary &p_args) {
	String tileset_path = mcp_normalize_path(p_args.get("tileset_path", ""));
	if (tileset_path.is_empty()) {
		return R"json({"error": "'tileset_path' is required."})json";
	}

	// 加载 TileSet
	Ref<TileSet> tile_set = ResourceLoader::load(tileset_path);
	if (tile_set.is_null()) {
		return vformat(R"json({"error": "TileSet not found or invalid: %s"})json", tileset_path);
	}

	int source_id = int(p_args.get("source_id", 0));

	// 获取图集源
	TileSetAtlasSource *atlas_source = Object::cast_to<TileSetAtlasSource>(tile_set->get_source(source_id).ptr());
	if (!atlas_source) {
		return vformat(R"json({"error": "Source ID %d not found in TileSet."})json", source_id);
	}

	// 解析瓦片坐标
	Vector2i atlas_coords;
	Variant coords_var = p_args.get("atlas_coords", Variant());
	if (coords_var.get_type() == Variant::VECTOR2I) {
		atlas_coords = coords_var;
	} else if (coords_var.get_type() == Variant::DICTIONARY) {
		Dictionary d = coords_var;
		atlas_coords = Vector2i(int(d.get("x", 0)), int(d.get("y", 0)));
	} else if (coords_var.get_type() == Variant::ARRAY) {
		Array arr = coords_var;
		if (arr.size() >= 2) {
			atlas_coords = Vector2i(int(arr[0]), int(arr[1]));
		}
	}

	// 检查瓦片是否存在
	if (!atlas_source->has_tile(atlas_coords)) {
		return vformat(R"json({"error": "Tile at atlas coords (%d, %d) does not exist."})json", atlas_coords.x, atlas_coords.y);
	}

	// 获取碰撞层
	int physics_layer = int(p_args.get("physics_layer", 0));

	// 如果物理层不存在，添加
	while (tile_set->get_physics_layers_count() <= physics_layer) {
		tile_set->add_physics_layer();
	}

	// 创建碰撞形状
	String shape_type = String(p_args.get("shape_type", "rectangle")).strip_edges().to_lower();

	TileData *tile_data = atlas_source->get_tile_data(atlas_coords, 0);
	if (!tile_data) {
		return R"json({"error": "Failed to get tile data."})json";
	}

	// 启用碰撞
	tile_data->set_collision_polygons_count(physics_layer, 1);

	if (shape_type == "rectangle") {
		// 矩形碰撞：设置多边形点
		Vector2i tile_size = atlas_source->get_texture_region_size();
		real_t half_w = tile_size.x / 2.0;
		real_t half_h = tile_size.y / 2.0;

		Vector<Vector2> points;
		points.push_back(Vector2(-half_w, -half_h));
		points.push_back(Vector2(half_w, -half_h));
		points.push_back(Vector2(half_w, half_h));
		points.push_back(Vector2(-half_w, half_h));
		tile_data->set_collision_polygon_points(physics_layer, 0, points);
	} else if (shape_type == "circle") {
		// 圆形碰撞：用多边形近似
		Vector2i tile_size = atlas_source->get_texture_region_size();
		real_t radius = MIN(tile_size.x, tile_size.y) / 2.0;
		int segments = 16;

		Vector<Vector2> points;
		for (int i = 0; i < segments; i++) {
			real_t angle = (real_t)i / segments * Math::TAU;
			points.push_back(Vector2(Math::cos(angle) * radius, Math::sin(angle) * radius));
		}
		tile_data->set_collision_polygon_points(physics_layer, 0, points);
	} else {
		// 自定义多边形点
		Variant points_var = p_args.get("points", Variant());
		if (points_var.get_type() == Variant::ARRAY) {
			Array arr = points_var;
			Vector<Vector2> points;
			for (int i = 0; i < arr.size(); i++) {
				Variant v = arr[i];
				if (v.get_type() == Variant::VECTOR2) {
					points.push_back(v);
				} else if (v.get_type() == Variant::DICTIONARY) {
					Dictionary d = v;
					points.push_back(Vector2(real_t(d.get("x", 0.0)), real_t(d.get("y", 0.0))));
				} else if (v.get_type() == Variant::ARRAY) {
					Array a = v;
					if (a.size() >= 2) {
						points.push_back(Vector2(real_t(a[0]), real_t(a[1])));
					}
				}
			}
			tile_data->set_collision_polygon_points(physics_layer, 0, points);
		} else {
			return R"json({"error": "'points' must be an array for custom shape_type."})json";
		}
	}

	// 可选：单向碰撞
	bool one_way = p_args.get("one_way_collision", false);
	tile_data->set_collision_polygon_one_way(physics_layer, 0, one_way);

	// 保存
	Error save_err = ResourceSaver::save(tile_set, tileset_path, ResourceSaver::FLAG_CHANGE_PATH);
	if (save_err != OK) {
		return vformat(R"json({"error": "Failed to save TileSet to %s (code %d)."})json", tileset_path, (int)save_err);
	}

	Dictionary result;
	result["tileset_path"] = tileset_path;
	result["source_id"] = source_id;
	Dictionary coords_dict;
	coords_dict["x"] = atlas_coords.x;
	coords_dict["y"] = atlas_coords.y;
	result["atlas_coords"] = coords_dict;
	result["physics_layer"] = physics_layer;
	result["shape_type"] = shape_type;
	result["one_way_collision"] = one_way;
	result["action"] = "collision_set";
	return JSON::stringify(result, "\t");
}

// ============================================================
void TileMapTools::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_tile_map", "args"), &TileMapTools::create_tile_map);
	ClassDB::bind_method(D_METHOD("set_cell", "args"), &TileMapTools::set_cell);
	ClassDB::bind_method(D_METHOD("set_cells_terrain_connect", "args"), &TileMapTools::set_cells_terrain_connect);
	ClassDB::bind_method(D_METHOD("clear_layer", "args"), &TileMapTools::clear_layer);
	ClassDB::bind_method(D_METHOD("get_tile_map_data", "args"), &TileMapTools::get_tile_map_data);
	ClassDB::bind_method(D_METHOD("create_tile_set", "args"), &TileMapTools::create_tile_set);
	ClassDB::bind_method(D_METHOD("add_tile_atlas_source", "args"), &TileMapTools::add_tile_atlas_source);
	ClassDB::bind_method(D_METHOD("set_tile_set_collision", "args"), &TileMapTools::set_tile_set_collision);
}
