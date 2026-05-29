#!/usr/bin/env python3
"""
集成所有新工具到 editor_plugin.cpp/h
- 在 editor_plugin.h 中声明新的 Ref<XXXTools> 成员
- 在 editor_plugin.cpp 的 _register_tools() 中注册所有新工具
- 在 _notification 中初始化新工具对象
"""
import re, os, sys

MODULE_DIR = r"F:\game\godot-source\modules\mcp_editor"
TOOLS_DIR = os.path.join(MODULE_DIR, "tools")

# 所有新工具组（类名, 文件前缀, 工具列表）
NEW_TOOL_GROUPS = [
    {
        "class": "TileMapTools",
        "member": "_tilemap_tools",
        "file": "tilemap_tools",
        "tools": [
            ("create_tile_map", "Create a TileMap node"),
            ("set_cell", "Set a single tile on TileMap"),
            ("set_cells_terrain_connect", "Paint terrain tiles with auto-connection"),
            ("clear_layer", "Clear a TileMap layer"),
            ("get_tile_map_data", "Get TileMap data (used cells)"),
            ("create_tile_set", "Create a TileSet resource"),
            ("add_tile_atlas_source", "Add atlas source to TileSet"),
            ("set_tile_set_collision", "Set collision shape for a tile"),
        ]
    },
    {
        "class": "PhysicsTools",
        "member": "_physics_tools",
        "file": "physics_tools",
        "tools": [
            ("create_collision_shape", "Create collision shape node"),
            ("set_collision_shape_data", "Set collision shape parameters"),
            ("create_ray_cast", "Create RayCast node"),
            ("create_navigation_region", "Create NavigationRegion node"),
            ("set_physics_material", "Set physics material properties"),
        ]
    },
    {
        "class": "ShaderTools",
        "member": "_shader_tools",
        "file": "shader_tools",
        "tools": [
            ("create_shader", "Create a shader file"),
            ("set_shader_code", "Set shader code content"),
            ("set_shader_param", "Set shader parameter on material"),
        ]
    },
    {
        "class": "ResourceTools",
        "member": "_resource_tools",
        "file": "resource_tools",
        "tools": [
            ("import_resource", "Import/copy resource into project"),
            ("get_resource_info", "Get resource file information"),
            ("set_resource_property", "Set resource property"),
            ("list_resources", "List resource files by type"),
        ]
    },
    {
        "class": "EditorTools",
        "member": "_editor_tools",
        "file": "editor_tools",
        "tools": [
            ("undo", "Undo last action"),
            ("redo", "Redo last action"),
            ("run_code", "Run GDScript code snippet"),
            ("get_editor_state", "Get current editor state"),
            ("set_editor_setting", "Set editor setting"),
        ]
    },
    {
        "class": "ThreeDTools",
        "member": "_threed_tools",
        "file": "threed_tools",
        "tools": [
            ("create_audio_player", "Create audio player node"),
            ("create_particles", "Create particle system"),
            ("create_mesh_instance", "Create 3D mesh instance"),
            ("create_light", "Create light node"),
            ("create_camera_3d", "Create 3D camera"),
            ("set_environment", "Set environment (sky, fog, ambient)"),
        ]
    },
    {
        "class": "ExtraTools",
        "member": "_extra_tools",
        "file": "extra_tools",
        "tools": [
            ("create_viewport", "Create SubViewport node"),
            ("disconnect_node_signal", "Disconnect a signal"),
            ("get_node_connections", "Get node signal connections"),
            ("add_node_to_group", "Add node to group"),
            ("remove_node_from_group", "Remove node from group"),
            ("list_groups", "List all node groups"),
            ("export_project", "Export project"),
            ("set_export_preset", "Configure export preset"),
        ]
    },
]

def patch_header():
    """在 editor_plugin.h 中添加新成员声明"""
    h_path = os.path.join(MODULE_DIR, "editor_plugin.h")
    with open(h_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 检查是否已经添加过
    if "_tilemap_tools" in content:
        print("Header already patched, skipping.")
        return
    
    # 添加前向声明
    forward_decls = "\n".join(f"class {g['class']};" for g in NEW_TOOL_GROUPS)
    
    # 找到现有的前向声明区域后面添加
    if "class SceneTools;" in content:
        content = content.replace(
            "class SceneTools;",
            "class SceneTools;\n" + forward_decls
        )
    
    # 添加成员变量 - 找到最后一个 Ref<> 成员
    members = "\n".join(f"\tRef<{g['class']}> {g['member']};" for g in NEW_TOOL_GROUPS)
    
    # 在最后一个 _xxx_tools 成员后面添加
    last_member = "_material_tools;"
    if last_member in content:
        content = content.replace(
            last_member,
            last_member + "\n" + members
        )
    
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Patched editor_plugin.h with {len(NEW_TOOL_GROUPS)} new tool members")

def patch_cpp():
    """在 editor_plugin.cpp 中添加注册和初始化代码"""
    cpp_path = os.path.join(MODULE_DIR, "editor_plugin.cpp")
    with open(cpp_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    if "_tilemap_tools" in content:
        print("CPP already patched, skipping.")
        return
    
    # 添加 includes
    includes = "\n".join(f'#include "tools/{g["file"]}.h"' for g in NEW_TOOL_GROUPS)
    # 在最后一个 tools/ include 后面添加
    last_include = '#include "tools/material_tools.h"'
    if last_include in content:
        content = content.replace(
            last_include,
            last_include + "\n" + includes
        )
    
    # 添加初始化代码 (在 _material_tools 初始化后)
    init_code = "\n".join(
        f"\t{g['member']}.instantiate();\n\t{g['member']}->set_editor_plugin(this);"
        for g in NEW_TOOL_GROUPS
    )
    # 找到 _material_tools 初始化
    mat_init = "_material_tools->set_editor_plugin(this);"
    if mat_init in content:
        content = content.replace(
            mat_init,
            mat_init + "\n" + init_code
        )
    
    # 添加工具注册
    registrations = []
    for g in NEW_TOOL_GROUPS:
        group_name = g['file'].replace('_tools', '')
        for tool_name, desc in g['tools']:
            registrations.append(
                f'\t_tool_map["{tool_name}"].object = {g["member"]};\n'
                f'\t_tool_map["{tool_name}"].method = "{tool_name}";\n'
                f'\t_tool_map["{tool_name}"].group = "{group_name}";\n'
                f'\t_tool_map["{tool_name}"].description = "{desc}";'
            )
    
    reg_block = "\n".join(registrations)
    # 在最后一个 _tool_map 注册后添加
    # 找到最后一个 description 行
    last_desc_pattern = r'(_tool_map\["[^"]+"\]\.description = "[^"]+";)\n(\n\t// )'
    match = None
    for m in re.finditer(last_desc_pattern, content):
        match = m
    if match:
        insert_pos = match.end() - len(match.group(2))
        content = content[:insert_pos] + "\n" + reg_block + "\n\n" + content[insert_pos:]
    else:
        # Fallback: 找到 _register_tools 方法的结尾
        print("Warning: Could not find insertion point for registrations, appending at end of _register_tools")
        # 找最后一个 _tool_map 行
        lines = content.split('\n')
        last_tool_line = 0
        for i, line in enumerate(lines):
            if '_tool_map[' in line:
                last_tool_line = i
        # 找到这行后面第一个空行
        insert_line = last_tool_line + 1
        while insert_line < len(lines) and lines[insert_line].strip():
            insert_line += 1
        lines.insert(insert_line, reg_block)
        content = '\n'.join(lines)
    
    with open(cpp_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    total_new = sum(len(g['tools']) for g in NEW_TOOL_GROUPS)
    print(f"Patched editor_plugin.cpp with {total_new} new tool registrations")

if __name__ == "__main__":
    patch_header()
    patch_cpp()
    
    # 统计
    total_new = sum(len(g['tools']) for g in NEW_TOOL_GROUPS)
    print(f"\nTotal new tools registered: {total_new}")
    print(f"Total tools (94 + {total_new}) = {94 + total_new}")
