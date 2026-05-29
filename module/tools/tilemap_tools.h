/**
 * tilemap_tools.h - TileMap/TileSet 工具
 *
 * TileMap 编辑和 TileSet 资源管理。
 */
#ifndef TILEMAP_TOOLS_H
#define TILEMAP_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class TileMapTools : public RefCounted {
	GDCLASS(TileMapTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);
	String create_tile_map(const Dictionary &p_args);
	String set_cell(const Dictionary &p_args);
	String set_cells_terrain_connect(const Dictionary &p_args);
	String clear_layer(const Dictionary &p_args);
	String get_tile_map_data(const Dictionary &p_args);
	String create_tile_set(const Dictionary &p_args);
	String add_tile_atlas_source(const Dictionary &p_args);
	String set_tile_set_collision(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // TILEMAP_TOOLS_H
