void MCPEditorPlugin::_register_tool_schemas() {
	// Auto-generated tool parameter schemas

	{  // add_animation_track
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "library name"; props["library_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "track type"; props["track_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "keys"; props["keys"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["add_animation_track"] = schema;
	}

	{  // add_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "deadzone"; props["deadzone"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of input events for the action"; props["events"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["add_input_action"] = schema;
	}

	{  // add_node_to_group
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "group"; props["group"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["add_node_to_group"] = schema;
	}

	{  // add_tile_atlas_source
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "texture path"; props["texture_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size x"; props["tile_size_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size y"; props["tile_size_y"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "separation x"; props["separation_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "separation y"; props["separation_y"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "margin x"; props["margin_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "margin y"; props["margin_y"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("tileset_path");
		req.push_back("texture_path");
		schema["required"] = req; }
		_tool_schemas["add_tile_atlas_source"] = schema;
	}

	{  // assign_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "material path"; props["material_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "surface index"; props["surface_index"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["assign_material"] = schema;
	}

	{  // capture_editor_view
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "view"; props["view"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "index"; props["index"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save to file"; props["save_to_file"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "save path"; props["save_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "return data uri"; props["return_data_uri"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["capture_editor_view"] = schema;
	}

	{  // clear_layer
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["clear_layer"] = schema;
	}

	{  // connect_node_signal
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "signal name"; props["signal_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "method name"; props["method_name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "flags"; props["flags"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["connect_node_signal"] = schema;
	}

	{  // copy_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from path"; props["from_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to path"; props["to_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["copy_file"] = schema;
	}

	{  // create_animation_clip
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "library name"; props["library_name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "length"; props["length"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "loop mode"; props["loop_mode"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "step"; props["step"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "set current"; props["set_current"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["create_animation_clip"] = schema;
	}

	{  // create_animation_player
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_animation_player"] = schema;
	}

	{  // create_audio_player
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "stream path"; props["stream_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "autoplay"; props["autoplay"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_audio_player"] = schema;
	}

	{  // create_camera_3d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "fov"; props["fov"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "near"; props["near"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "far"; props["far"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_camera_3d"] = schema;
	}

	{  // create_collision_shape
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "width"; props["width"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "height"; props["height"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "radius"; props["radius"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "from"; props["from"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to"; props["to"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "disabled"; props["disabled"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_collision_shape"] = schema;
	}

	{  // create_container
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "container type"; props["container_type"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("container_type");
		schema["required"] = req; }
		_tool_schemas["create_container"] = schema;
	}

	{  // create_control
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "control type"; props["control_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("control_type");
		schema["required"] = req; }
		_tool_schemas["create_control"] = schema;
	}

	{  // create_light
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "light type"; props["light_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "color"; props["color"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "energy"; props["energy"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_light"] = schema;
	}

	{  // create_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "material type"; props["material_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "properties"; props["properties"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_material"] = schema;
	}

	{  // create_mesh_instance
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mesh path"; props["mesh_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_mesh_instance"] = schema;
	}

	{  // create_navigation_region
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "vertices"; props["vertices"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "polygons"; props["polygons"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_navigation_region"] = schema;
	}

	{  // create_new_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "root type"; props["root_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "root name"; props["root_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "open after"; props["open_after"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_new_scene"] = schema;
	}

	{  // create_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node type"; props["node_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_type");
		schema["required"] = req; }
		_tool_schemas["create_node"] = schema;
	}

	{  // create_packed_scene_from_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select file"; props["select_file"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_packed_scene_from_node"] = schema;
	}

	{  // create_particles
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "particle type"; props["particle_type"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "amount"; props["amount"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_particles"] = schema;
	}

	{  // create_ray_cast
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target position"; props["target_position"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_ray_cast"] = schema;
	}

	{  // create_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "language"; props["language"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "class name"; props["class_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "namespace"; props["namespace"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "extends"; props["extends"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "body"; props["body"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "tool"; props["tool"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "partial"; props["partial"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include system"; props["include_system"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "open in editor"; props["open_in_editor"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_script"] = schema;
	}

	{  // create_shader
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shader type"; props["shader_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "code"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_shader"] = schema;
	}

	{  // create_tile_map
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layers"; props["layers"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_tile_map"] = schema;
	}

	{  // create_tile_set
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size x"; props["tile_size_x"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "tile size y"; props["tile_size_y"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["create_tile_set"] = schema;
	}

	{  // create_ui_root
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "kind"; props["kind"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "control name"; props["control_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "layout preset"; props["layout_preset"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_ui_root"] = schema;
	}

	{  // create_viewport
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "size w"; props["size_w"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "size h"; props["size_h"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["create_viewport"] = schema;
	}

	{  // delete_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["delete_file"] = schema;
	}

	{  // disconnect_node_signal
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "signal name"; props["signal_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "target path"; props["target_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "method name"; props["method_name"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["disconnect_node_signal"] = schema;
	}

	{  // duplicate_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new name"; props["new_name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["duplicate_node"] = schema;
	}

	{  // edit_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "content"; props["content"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["edit_script"] = schema;
	}

	{  // enter_play_mode
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "scene path"; props["scene_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("scene_path");
		schema["required"] = req; }
		_tool_schemas["enter_play_mode"] = schema;
	}

	{  // export_project
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "preset name"; props["preset_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "export path"; props["export_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "debug"; props["debug"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["export_project"] = schema;
	}

	{  // file_exists
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["file_exists"] = schema;
	}

	{  // find_nodes
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name contains"; props["name_contains"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "class name"; props["class_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["find_nodes"] = schema;
	}

	{  // find_usages
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "symbol"; props["symbol"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "case sensitive"; props["case_sensitive"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("symbol");
		schema["required"] = req; }
		_tool_schemas["find_usages"] = schema;
	}

	{  // get_console_logs
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max lines"; props["max_lines"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include rotated"; props["include_rotated"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "filter"; props["filter"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "severity"; props["severity"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_console_logs"] = schema;
	}

	{  // get_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["get_input_action"] = schema;
	}

	{  // get_node_connections
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_node_connections"] = schema;
	}

	{  // get_node_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_node_info"] = schema;
	}

	{  // get_packed_scene_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max depth"; props["max_depth"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["get_packed_scene_info"] = schema;
	}

	{  // get_project_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("key");
		schema["required"] = req; }
		_tool_schemas["get_project_setting"] = schema;
	}

	{  // get_resource_info
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_resource_info"] = schema;
	}

	{  // get_script_errors
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max files"; props["max_files"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["get_script_errors"] = schema;
	}

	{  // get_tile_map_data
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["get_tile_map_data"] = schema;
	}

	{  // import_resource
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "source path"; props["source_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "dest path"; props["dest_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["import_resource"] = schema;
	}

	{  // instantiate_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "scene path"; props["scene_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "select new node"; props["select_new_node"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("scene_path");
		schema["required"] = req; }
		_tool_schemas["instantiate_scene"] = schema;
	}

	{  // list_animations
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_animations"] = schema;
	}

	{  // list_files
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include hidden"; props["include_hidden"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_files"] = schema;
	}

	{  // list_node_methods
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include private"; props["include_private"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_methods"] = schema;
	}

	{  // list_node_properties
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include usage"; props["include_usage"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_properties"] = schema;
	}

	{  // list_node_signals
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["list_node_signals"] = schema;
	}

	{  // list_project_settings
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "prefix"; props["prefix"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include internal"; props["include_internal"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_project_settings"] = schema;
	}

	{  // list_resources
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "type filter"; props["type_filter"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_resources"] = schema;
	}

	{  // list_scenes
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_scenes"] = schema;
	}

	{  // list_scripts
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max entries"; props["max_entries"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "language"; props["language"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["list_scripts"] = schema;
	}

	{  // map_project
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "format"; props["format"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "include scripts"; props["include_scripts"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max files"; props["max_files"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["map_project"] = schema;
	}

	{  // move_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from path"; props["from_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to path"; props["to_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["move_file"] = schema;
	}

	{  // open_scene
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "set inherited"; props["set_inherited"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["open_scene"] = schema;
	}

	{  // open_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "line"; props["line"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "column"; props["column"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["open_script"] = schema;
	}

	{  // patch_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "find"; props["find"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "replace"; props["replace"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "prepend"; props["prepend"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "append"; props["append"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["patch_script"] = schema;
	}

	{  // play_animation
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation player path"; props["animation_player_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "animation name"; props["animation_name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "custom blend"; props["custom_blend"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "custom speed"; props["custom_speed"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "from end"; props["from_end"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("animation_name");
		schema["required"] = req; }
		_tool_schemas["play_animation"] = schema;
	}

	{  // read_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max chars"; props["max_chars"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["read_file"] = schema;
	}

	{  // remove_autoload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("name");
		schema["required"] = req; }
		_tool_schemas["remove_autoload"] = schema;
	}

	{  // remove_input_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["remove_input_action"] = schema;
	}

	{  // remove_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["remove_node"] = schema;
	}

	{  // remove_node_from_group
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "group"; props["group"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["remove_node_from_group"] = schema;
	}

	{  // rename_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new name"; props["new_name"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["rename_node"] = schema;
	}

	{  // reparent_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "new parent path"; props["new_parent_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "keep global transform"; props["keep_global_transform"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["reparent_node"] = schema;
	}

	{  // request_script_reload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["request_script_reload"] = schema;
	}

	{  // run_code
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "GDScript code to execute"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("code");
		schema["required"] = req; }
		_tool_schemas["run_code"] = schema;
	}

	{  // save_scene_as
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "with preview"; props["with_preview"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["save_scene_as"] = schema;
	}

	{  // search_files
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "pattern"; props["pattern"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "recursive"; props["recursive"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "max results"; props["max_results"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("pattern");
		schema["required"] = req; }
		_tool_schemas["search_files"] = schema;
	}

	{  // select_node
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["select_node"] = schema;
	}

	{  // set_addon_enabled
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "addon"; props["addon"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "enabled"; props["enabled"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("addon");
		req.push_back("enabled");
		schema["required"] = req; }
		_tool_schemas["set_addon_enabled"] = schema;
	}

	{  // set_autoload
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_autoload"] = schema;
	}

	{  // set_cell
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "coords"; props["coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "atlas coords"; props["atlas_coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "alternative tile"; props["alternative_tile"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_cell"] = schema;
	}

	{  // set_cells_terrain_connect
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "layer"; props["layer"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "terrain set"; props["terrain_set"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "terrain"; props["terrain"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "cells"; props["cells"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_cells_terrain_connect"] = schema;
	}

	{  // set_collision_shape_data
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "width"; props["width"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "height"; props["height"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "radius"; props["radius"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_collision_shape_data"] = schema;
	}

	{  // set_control_layout
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_layout"] = schema;
	}

	{  // set_control_size_flags
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_size_flags"] = schema;
	}

	{  // set_control_text
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "text"; props["text"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_text"] = schema;
	}

	{  // set_control_theme_override
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "override type"; props["override_type"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "resource path"; props["resource_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_control_theme_override"] = schema;
	}

	{  // set_editor_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_editor_setting"] = schema;
	}

	{  // set_environment
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "parent path"; props["parent_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "name"; props["name"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "sky color"; props["sky_color"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "ambient color"; props["ambient_color"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_environment"] = schema;
	}

	{  // set_export_preset
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "preset name"; props["preset_name"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "platform"; props["platform"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "export path"; props["export_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "settings"; props["settings"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_export_preset"] = schema;
	}

	{  // set_node_properties
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "properties"; props["properties"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_node_properties"] = schema;
	}

	{  // set_node_property
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_node_property"] = schema;
	}

	{  // set_node_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "script path"; props["script_path"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_node_script"] = schema;
	}

	{  // set_physics_material
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_physics_material"] = schema;
	}

	{  // set_project_setting
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "save"; props["save"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("key");
		req.push_back("value");
		schema["required"] = req; }
		_tool_schemas["set_project_setting"] = schema;
	}

	{  // set_resource_property
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "property"; props["property"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["set_resource_property"] = schema;
	}

	{  // set_shader_code
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "code"; props["code"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		req.push_back("code");
		schema["required"] = req; }
		_tool_schemas["set_shader_code"] = schema;
	}

	{  // set_shader_param
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shader path"; props["shader_path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "param"; props["param"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "params"; props["params"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_shader_param"] = schema;
	}

	{  // set_tile_set_collision
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "tileset path"; props["tileset_path"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "source id"; props["source_id"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "atlas coords"; props["atlas_coords"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "physics layer"; props["physics_layer"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "shape type"; props["shape_type"] = p; }
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of collision points"; props["points"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "one way collision"; props["one_way_collision"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("tileset_path");
		schema["required"] = req; }
		_tool_schemas["set_tile_set_collision"] = schema;
	}

	{  // set_time_scale
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "value"; props["value"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("value");
		schema["required"] = req; }
		_tool_schemas["set_time_scale"] = schema;
	}

	{  // set_transform_2d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_transform_2d"] = schema;
	}

	{  // set_transform_3d
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "node path"; props["node_path"] = p; }
		{ Dictionary p; p["type"] = "boolean"; p["description"] = "undoable"; props["undoable"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("node_path");
		schema["required"] = req; }
		_tool_schemas["set_transform_3d"] = schema;
	}

	{  // simulate_action
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "action"; props["action"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "number"; p["description"] = "strength"; props["strength"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("action");
		schema["required"] = req; }
		_tool_schemas["simulate_action"] = schema;
	}

	{  // simulate_input_sequence
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "array"; p["description"] = "array of input events"; props["events"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_input_sequence"] = schema;
	}

	{  // simulate_key_event
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "key"; props["key"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "physical key"; props["physical_key"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("physical_key");
		schema["required"] = req; }
		_tool_schemas["simulate_key_event"] = schema;
	}

	{  // simulate_mouse_button
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "mode"; props["mode"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "button"; props["button"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "position"; props["position"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_mouse_button"] = schema;
	}

	{  // simulate_mouse_drag
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "from position"; props["from_position"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "to position"; props["to_position"] = p; }
		{ Dictionary p; p["type"] = "integer"; p["description"] = "steps"; props["steps"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "button"; props["button"] = p; }
		schema["properties"] = props;
		schema["required"] = Array();
		_tool_schemas["simulate_mouse_drag"] = schema;
	}

	{  // validate_script
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["validate_script"] = schema;
	}

	{  // write_file
		Dictionary schema;
		schema["type"] = "object";
		Dictionary props;
		{ Dictionary p; p["type"] = "string"; p["description"] = "path"; props["path"] = p; }
		{ Dictionary p; p["type"] = "string"; p["description"] = "content"; props["content"] = p; }
		schema["properties"] = props;
		{ Array req;
		req.push_back("path");
		schema["required"] = req; }
		_tool_schemas["write_file"] = schema;
	}

}