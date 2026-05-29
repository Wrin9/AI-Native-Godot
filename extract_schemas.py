#!/usr/bin/env python3
"""
从 MCP 工具源码中提取参数定义，生成 inputSchema 注册代码。
分析 p_args.get("param_name", default_value) 模式。
"""
import re
import os
import json

TOOLS_DIR = r"F:\game\godot-source\modules\mcp_editor\tools"

def extract_params_from_file(filepath):
    """从 cpp 文件中提取 p_args.get() 调用的参数名和默认值"""
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    
    # Match: p_args.get("param_name", default_value)
    # Also match required checks: "'param' is required"
    pattern = r'p_args\.get\(\s*"([^"]+)"\s*,\s*([^)]+)\)'
    matches = re.findall(pattern, content)
    
    params = {}
    required_params = set()
    
    # Check for required patterns
    req_pattern = r'"\'(\w+)\' is required"'
    req_matches = re.findall(req_pattern, content)
    required_params = set(req_matches)
    
    for name, default in matches:
        # Determine type from default value
        default = default.strip()
        ptype = "string"
        if default in ('true', 'false'):
            ptype = "boolean"
        elif default.startswith('"') or default.startswith("'"):
            ptype = "string"
        elif '.' in default and not default.startswith('"'):
            ptype = "number"
        elif default.isdigit() or (default.startswith('-') and default[1:].isdigit()):
            ptype = "integer"
        elif default == 'Array()' or default.startswith('Array['):
            ptype = "array"
        elif default == 'Dictionary()' or default.startswith('Dictionary('):
            ptype = "object"
        
        is_required = name in required_params
        params[name] = {"type": ptype, "required": is_required, "default": default}
    
    return params

def find_tool_methods(filepath):
    """Find all tool methods and which params belong to which method"""
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()
    
    methods = {}
    current_method = None
    
    for i, line in enumerate(lines):
        # Match method definition
        m = re.match(r'^String \w+::(\w+)\(const Dictionary &p_args', line)
        if m:
            current_method = m.group(1)
            methods[current_method] = {}
            continue
        
        if current_method:
            # Match p_args.get
            pm = re.search(r'p_args\.get\(\s*"([^"]+)"\s*,\s*([^)]+)\)', line)
            if pm:
                name = pm.group(1)
                default = pm.group(2).strip()
                ptype = "string"
                if default in ('true', 'false'):
                    ptype = "boolean"
                elif '.' in default and not default.startswith('"'):
                    ptype = "number"
                elif default.isdigit() or (default.startswith('-') and default[1:].isdigit()):
                    ptype = "integer"
                elif default.startswith('Array') or default.startswith('['):
                    ptype = "array"
                
                methods[current_method][name] = {"type": ptype}
            
            # Check required
            req = re.search(r'"\'(\w+)\' is required"', line)
            if req:
                pname = req.group(1)
                if pname in methods[current_method]:
                    methods[current_method][pname]["required"] = True
    
    return methods

# Process all tool files
tool_files = [
    "scene_tools.cpp", "node_tools.cpp", "script_tools.cpp", "file_tools.cpp",
    "play_tools.cpp", "diagnostic_tools.cpp", "ui_tools.cpp", "animation_tools.cpp",
    "material_tools.cpp", "project_tools.cpp", "tilemap_tools.cpp", "physics_tools.cpp",
    "shader_tools.cpp", "resource_tools.cpp", "editor_tools.cpp", "threed_tools.cpp",
    "extra_tools.cpp"
]

all_schemas = {}

for tf in tool_files:
    fp = os.path.join(TOOLS_DIR, tf)
    if not os.path.exists(fp):
        print(f"SKIP: {tf}")
        continue
    
    methods = find_tool_methods(fp)
    for method, params in methods.items():
        if params:
            all_schemas[method] = params
        else:
            all_schemas[method] = {}  # no params needed

# Now read editor_plugin.cpp to get tool_name -> method mapping
plugin_cpp = os.path.join(os.path.dirname(TOOLS_DIR), "editor_plugin.cpp")
with open(plugin_cpp, 'r', encoding='utf-8', errors='replace') as f:
    plugin_content = f.read()

# Extract _tool_map registrations: _tool_map["tool_name"].method = "method_name"
tool_to_method = {}
reg_pattern = r'_tool_map\["([^"]+)"\]\.method\s*=\s*"([^"]+)"'
for match in re.finditer(reg_pattern, plugin_content):
    tool_to_method[match.group(1)] = match.group(2)

# Generate C++ code for each tool schema
print(f"// Total tools: {len(tool_to_method)}")
print(f"// Schemas extracted: {len(all_schemas)}")
print()

# Generate the schema registration code
for tool_name, method_name in sorted(tool_to_method.items()):
    params = all_schemas.get(method_name, {})
    required = [k for k, v in params.items() if v.get("required")]
    
    if not params:
        # No params - skip (empty schema is fine)
        continue
    
    print(f'\t// {tool_name}')
    print(f'\t{{')
    print(f'\t\tDictionary props;')
    
    for pname, pinfo in params.items():
        ptype = pinfo["type"]
        desc = ""
        # Generate description from param name
        desc = pname.replace('_', ' ').title()
        
        if ptype == "string":
            print(f'\t\tprops["{pname}"] = _mk_prop("string", "{desc}");')
        elif ptype == "integer":
            print(f'\t\tprops["{pname}"] = _mk_prop("integer", "{desc}");')
        elif ptype == "number":
            print(f'\t\tprops["{pname}"] = _mk_prop("number", "{desc}");')
        elif ptype == "boolean":
            print(f'\t\tprops["{pname}"] = _mk_prop("boolean", "{desc}");')
        elif ptype == "array":
            print(f'\t\tprops["{pname}"] = _mk_prop("array", "{desc}");')
    
    if required:
        req_str = '", "'.join(required)
        print(f'\t\tArray req; req.push_back("{required[0]}");' if len(required)==1 else f'\t\tArray req;')
        for r in required:
            print(f'\t\treq.push_back("{r}");')
        print(f'\t\t_tool_map["{tool_name}"].input_schema["required"] = req;')
    
    print(f'\t\t_tool_map["{tool_name}"].input_schema["properties"] = props;')
    print(f'\t}}')
    print()

# Also output JSON for reference
print("\n// === JSON Reference ===")
output = {}
for tool_name, method_name in sorted(tool_to_method.items()):
    params = all_schemas.get(method_name, {})
    required = [k for k, v in params.items() if v.get("required")]
    schema = {"type": "object", "properties": {}, "required": required}
    for pname, pinfo in params.items():
        schema["properties"][pname] = {"type": pinfo["type"]}
    output[tool_name] = schema

print(json.dumps(output, indent=2))
