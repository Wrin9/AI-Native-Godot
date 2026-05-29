# MCP 工具参考文档

AI-Native Godot 通过 MCP 协议暴露 100+ 工具，覆盖游戏开发全流程。本文档列出所有工具的分类、参数及返回值。

> **Profile 说明**：工具按安全级别分为两个 Profile——`core`（只读为主，安全）和 `full`（包含写操作）。AI-Native Godot 默认使用 `full` profile。

---

## 目录

- [代码执行 (Execution)](#代码执行-execution)
- [帮助与引导 (Guidance)](#帮助与引导-guidance)
- [项目地图 (Project Map)](#项目地图-project-map)
- [诊断 (Diagnostics)](#诊断-diagnostics)
- [项目 (Project)](#项目-project)
- [输入映射 (Input)](#输入映射-input)
- [运行时 (Runtime)](#运行时-runtime)
- [撤销重做 (Undo/Redo)](#撤销重做-undoredo)
- [场景 (Scene)](#场景-scene)
- [节点 (Nodes)](#节点-nodes)
- [脚本 (Scripts)](#脚本-scripts)
- [播放模式 (Play)](#播放模式-play)
- [断言 (Assertions)](#断言-assertions)
- [动画 (Animation)](#动画-animation)
- [摄像机 (Camera)](#摄像机-camera)
- [材质 (Materials)](#材质-materials)
- [UI 构建 (UI)](#ui-构建-ui)
- [文件操作 (Files)](#文件操作-files)
- [插件 (Addons)](#插件-addons)

---

## 代码执行 (Execution)

### execute_code
运行 GDScript 代码片段。代码被包装为 `func run(ctx)` 执行，`ctx` 提供 `log()`、`log_warning()`、`log_error()` 方法。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| code | string | 是 | GDScript 代码片段 |

**返回**: JSON
```json
{
  "result": "代码返回值",
  "logs": ["..."],
  "changes": [
    {"action": "created", "type": "Node", "name": "...", "path": "..."}
  ]
}
```

### capture_editor_view
截取 2D/3D 视口的屏幕截图。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| view | string | 否 | `"2d"` 或 `"3d"`（默认自动检测） |
| save_to_file | string | 否 | 保存到指定文件路径 |
| return_data_uri | bool | 否 | 是否返回 base64 data URI（默认 true） |

**返回**: base64 编码的 PNG 图片（data URI 格式）

### log_message
向 Godot 输出面板写入消息。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| message | string | 是 | 消息内容 |
| level | string | 否 | `"info"` / `"warning"` / `"error"`（默认 `"info"`） |

**返回**: 确认信息

### wait_msec
短暂阻塞等待，用于操作间稳定化。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| duration | int | 是 | 等待毫秒数 |

**返回**: 确认信息

---

## 帮助与引导 (Guidance)

### funplay_help
获取工作流帮助信息。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| topic | string | 否 | 帮助主题：`"overview"` / `"scene"` / `"runtime"` / `"scripts"` / `"ui"` |

**返回**: 帮助文本

### list_tool_catalog
获取分组工具目录。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| profile | string | 否 | 过滤 profile：`"core"` / `"full"` |
| group | string | 否 | 过滤分组名 |
| include_hidden | bool | 否 | 是否包含隐藏工具（默认 false） |

**返回**: 按组分类的工具列表

### get_capability_status
获取编辑器能力状态。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "project_name": "my_game",
  "capabilities": {
    "mcp_server": true,
    "tool_registry": true,
    "scene_open": true,
    "play_mode": false,
    "dotnet": false,
    "lsp": true,
    "dap": false,
    "undo_redo": true,
    "runtime_bridge": false
  }
}
```

### list_workflow_coverage
获取工作流覆盖矩阵，显示每个工作领域可用的工具。

**Profile**: core+full

**参数**: 无

**返回**: 6 个工作流领域的工具可用性列表

---

## 项目地图 (Project Map)

### map_project
获取项目地图（场景、脚本、依赖关系图）。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| format | string | 否 | `"json"` 或 `"html"`（默认 `"json"`） |
| include_scripts | bool | 否 | 包含脚本信息（默认 true） |
| include_graph | bool | 否 | 包含依赖图（默认 true） |
| max_files | int | 否 | 最大文件数（默认 100） |
| max_script_members | int | 否 | 最大脚本成员数（默认 30） |

**返回**: 项目结构 JSON 或 HTML

### find_usages
在项目文件中搜索符号使用。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| symbol | string | 是 | 要搜索的符号名称 |
| path | string | 否 | 限定搜索路径 |
| case_sensitive | bool | 否 | 大小写敏感（默认 false） |
| max_results | int | 否 | 最大结果数（默认 20） |

**返回**: 匹配位置列表

---

## 诊断 (Diagnostics)

### get_editor_protocol_status
获取 MCP 协议状态（支持版本、服务器信息等）。

**Profile**: core+full

**参数**: 无

**返回**: 协议版本和服务器信息

### get_script_errors
获取当前脚本错误列表。

**Profile**: core+full

**参数**: 无

**返回**: 错误列表，包含文件路径、行号、消息

### validate_script
验证脚本文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本文件路径 |
| language | string | 否 | `"gdscript"` / `"csharp"`（自动检测） |

**返回**: 验证结果（通过/错误列表）

### request_script_reload
请求重新加载脚本。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本文件路径 |

**返回**: 确认信息

### get_console_logs
读取编辑器控制台日志。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| max_lines | int | 否 | 最大行数（默认 50） |
| severity | string | 否 | 过滤级别：`"all"` / `"warning"` / `"error"` |
| filter | string | 否 | 文本过滤 |

**返回**: 日志行列表

### get_performance_snapshot
获取性能快照（FPS、节点数、视口大小等）。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "fps": 60,
  "node_count": 42,
  "viewport_size": {"x": 1152, "y": 648},
  "object_count": 156
}
```

### analyze_scene_complexity
分析当前场景的复杂度。

**Profile**: core+full

**参数**: 无

**返回**: 按类型统计的节点数量和复杂度评分

---

## 项目 (Project)

### get_project_info
获取项目基本信息。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "project_name": "my_game",
  "godot_version": "4.3.custom",
  "root_path": "res://",
  "current_scene_path": "res://scenes/main.tscn",
  "open_scene_count": 2,
  "play_state": "stopped",
  "settings": { ... }
}
```

### list_project_features
列出项目特性。

**Profile**: core+full

**参数**: 无

**返回**: 项目名称、主场景、渲染方法、脚本语言模式、输入动作、自动加载等

### list_project_settings
列出项目设置。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| prefix | string | 否 | 设置键前缀过滤 |
| include_internal | bool | 否 | 包含内部设置（默认 false） |
| max_results | int | 否 | 最大结果数（默认 50） |

**返回**: 设置键值对列表

### get_project_setting
获取单个项目设置。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| key | string | 是 | 设置键（如 `"application/run/main_scene"`） |

**返回**: 设置值

### set_project_setting
修改项目设置。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| key | string | 是 | 设置键 |
| value | any | 是 | 新值 |
| save | bool | 否 | 是否立即保存（默认 true） |

**返回**: 确认信息

### get_project_skills_status
获取 Project Skills 状态。

**Profile**: core+full

**参数**: 无

**返回**: 技能文件是否已生成

### generate_project_skills
生成 Project Skills 文件。

**Profile**: core+full

**参数**: 无

**返回**: 生成结果

---

## 输入映射 (Input)

### list_input_actions
列出所有输入动作。

**Profile**: core+full

**参数**: 无

**返回**: 动作名称列表

### get_input_action
获取指定输入动作的详细绑定。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | 动作名称 |

**返回**: 绑定的事件列表

### add_input_action
添加新的输入动作。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | 动作名称 |

**返回**: 确认信息

### remove_input_action
移除输入动作。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | 动作名称 |

**返回**: 确认信息

### add_input_event_to_action
为动作添加输入事件。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | 动作名称 |
| event | object | 是 | 输入事件定义 |

**返回**: 确认信息

### clear_input_events
清除动作的所有输入事件绑定。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | 动作名称 |

**返回**: 确认信息

---

## 运行时 (Runtime)

### list_autoloads
列出所有自动加载（Autoload）。

**Profile**: core+full

**参数**: 无

**返回**: 自动加载名称和路径列表

### get_runtime_bridge_status
获取 Runtime Bridge 状态。

**Profile**: core+full

**参数**: 无

**返回**: Bridge 是否安装和运行

### set_autoload
设置自动加载。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| name | string | 是 | 自动加载名称 |
| path | string | 是 | 脚本路径 |

**返回**: 确认信息

### remove_autoload
移除自动加载。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| name | string | 是 | 自动加载名称 |

**返回**: 确认信息

### install_runtime_bridge
安装 Runtime Bridge（播放模式状态捕获的自动加载）。

**Profile**: full

**参数**: 无

**返回**: 确认信息

### remove_runtime_bridge
移除 Runtime Bridge。

**Profile**: full

**参数**: 无

**返回**: 确认信息

---

## 撤销重做 (Undo/Redo)

### get_undo_redo_status
获取撤销/重做栈状态。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "can_undo": true,
  "can_redo": false,
  "undo_count": 5,
  "redo_count": 0,
  "current_action": "Set Node3D:position"
}
```

### editor_undo
执行撤销操作。

**Profile**: core+full

**参数**: 无

**返回**: 确认信息

### editor_redo
执行重做操作。

**Profile**: core+full

**参数**: 无

**返回**: 确认信息

---

## 场景 (Scene)

### get_scene_info
获取当前场景信息。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "current_scene_path": "res://scenes/main.tscn",
  "open_scene_count": 2,
  "root_node_type": "Node3D",
  "root_node_name": "Main",
  "node_count": 15,
  "editable_children": [],
  "is_modified": false
}
```

### get_scene_tree
获取场景树结构。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| max_depth | int | 否 | 最大深度（默认 -1 无限制） |

**返回**: 层级化的节点树

### list_scenes
列出场景文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 否 | 搜索路径（默认 `"res://"`） |
| max_entries | int | 否 | 最大条目数（默认 50） |
| recursive | bool | 否 | 递归搜索（默认 true） |

**返回**: 场景文件路径列表

### open_scene
打开场景。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 场景文件路径 |

**返回**: 确认信息

### save_scene
保存当前场景。

**Profile**: core+full

**参数**: 无

**返回**: 确认信息

### save_scene_as
另存为场景。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 新文件路径 |

**返回**: 确认信息

### create_new_scene
创建新场景。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 否 | 场景保存路径 |
| root_type | string | 否 | 根节点类型（默认 `"Node3D"`） |
| root_name | string | 否 | 根节点名称（默认 `"Root"`） |
| script_path | string | 否 | 附加脚本路径 |

**返回**: 确认信息

### instantiate_scene
在场景树中实例化一个场景。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| scene_path | string | 是 | 要实例化的场景路径 |
| parent_path | string | 否 | 父节点路径 |
| name | string | 否 | 实例名称 |

**返回**: 创建的节点信息

### create_packed_scene_from_node
从节点创建 PackedScene。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 源节点路径 |
| path | string | 是 | 保存路径 |

**返回**: 确认信息

### get_packed_scene_info
获取 PackedScene 信息。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | PackedScene 路径 |
| max_depth | int | 否 | 最大展开深度 |

**返回**: 场景结构信息

---

## 节点 (Nodes)

### get_node_info
获取节点详细信息。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |

**返回**: JSON
```json
{
  "name": "Player",
  "type": "CharacterBody3D",
  "path": "Root/Player",
  "script": "res://scripts/player.gd",
  "child_count": 3,
  "properties": { ... }
}
```

### get_selection
获取当前编辑器选中节点。

**Profile**: core+full

**参数**: 无

**返回**: 选中节点路径列表

### find_nodes
搜索节点。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| name_contains | string | 否 | 名称包含文本 |
| class_name | string | 否 | 类名过滤 |
| script_path | string | 否 | 脚本路径过滤 |
| max_results | int | 否 | 最大结果数（默认 20） |

**返回**: 匹配的节点列表

### select_node
选中节点。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| focus | bool | 否 | 是否聚焦到节点（默认 true） |

**返回**: 确认信息

### list_node_properties
列出节点属性。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| include_usage | bool | 否 | 包含属性用法标志 |

**返回**: 属性列表（名称、类型、值）

### list_node_signals
列出节点信号。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |

**返回**: 信号列表（名称、参数）

### list_node_methods
列出节点方法。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| include_private | bool | 否 | 包含 `_` 前缀方法（默认 false） |

**返回**: 方法列表（名称、返回类型、参数）

### create_node
创建新节点并添加到场景树。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| type | string | 是 | 节点类型（如 `"Node3D"`, `"Camera3D"`, `"MeshInstance3D"`） |
| parent | string | 否 | 父节点路径（默认场景根节点） |
| name | string | 否 | 节点名称（自动生成） |
| script_path | string | 否 | 附加脚本路径 |

**返回**: 创建的节点信息

### duplicate_node
复制节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 要复制的节点路径 |
| new_name | string | 否 | 新节点名称 |

**返回**: 新节点信息

### rename_node
重命名节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| new_name | string | 是 | 新名称 |

**返回**: 确认信息

### reparent_node
改变节点父级。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| new_parent_path | string | 是 | 新父节点路径 |
| keep_global_transform | bool | 否 | 保持全局变换（默认 true） |

**返回**: 确认信息

### remove_node
删除节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 要删除的节点路径 |

**返回**: 确认信息

### set_node_property
设置节点属性。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| property | string | 是 | 属性名（如 `"position"`, `"visible"`） |
| value | any | 是 | 新值 |

**返回**: 确认信息

### set_node_properties
批量设置节点属性。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| properties | object | 是 | 属性键值对 |

**返回**: 确认信息

### set_transform_2d
设置 2D 变换（Node2D / Control）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| position | object | 否 | `{"x": 0, "y": 0}` |
| rotation_degrees | float | 否 | 旋转角度 |
| scale | object | 否 | `{"x": 1, "y": 1}` |

**返回**: 确认信息

### set_transform_3d
设置 3D 变换（Node3D）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| position | object | 否 | `{"x": 0, "y": 0, "z": 0}` |
| rotation_degrees | object | 否 | `{"x": 0, "y": 0, "z": 0}` |
| scale | object | 否 | `{"x": 1, "y": 1, "z": 1}` |

**返回**: 确认信息

### set_node_script
设置节点脚本。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| script_path | string | 是 | 脚本资源路径 |

**返回**: 确认信息

---

## 脚本 (Scripts)

### create_script
创建新脚本。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本路径（如 `"res://scripts/player.gd"`） |
| language | string | 否 | `"gdscript"` / `"csharp"`（默认 `"gdscript"`） |
| extends | string | 否 | 继承类（默认 `"RefCounted"`） |
| class_name | string | 否 | 类名声明 |
| body | string | 否 | 脚本正文 |
| tool | bool | 否 | 是否为 @tool 脚本 |

**返回**: 确认信息

### list_scripts
列出脚本文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 否 | 搜索路径 |
| language | string | 否 | 过滤语言：`"gdscript"` / `"csharp"` |

**返回**: 脚本路径列表

### open_script
在脚本编辑器中打开脚本。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本路径 |
| line | int | 否 | 跳转到行号 |
| column | int | 否 | 跳转到列号 |

**返回**: 确认信息

### edit_script
编辑脚本内容。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本路径 |
| find | string | 否 | 查找文本 |
| replace | string | 否 | 替换文本 |
| prepend | string | 否 | 在文件开头添加内容 |
| append | string | 否 | 在文件末尾添加内容 |

**返回**: 修改结果

### patch_script
脚本补丁操作（查找替换）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 脚本路径 |
| find | string | 是 | 查找文本 |
| replace | string | 是 | 替换文本 |

**返回**: 替换次数

### get_dotnet_project_info
获取 .NET 项目信息（仅 dotnet/mixed 模式）。

**Profile**: core+full（需 `dotnet`/`mixed` 语言模式）

**参数**: 无

**返回**: .NET 项目信息（csproj 路径、解决方案等）

---

## 播放模式 (Play)

### get_play_state
获取播放状态。

**Profile**: core+full

**参数**: 无

**返回**: JSON
```json
{
  "state": "stopped",
  "scene": "",
  "is_playing": false,
  "is_paused": false
}
```
(`state`: `"stopped"` / `"playing"` / `"paused"`)

### enter_play_mode
进入播放模式。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| mode | string | 否 | `"play"` / `"pause"` |
| scene_path | string | 否 | 播放指定场景 |

**返回**: 确认信息

### play_main_scene
播放主场景。

**Profile**: core+full

**参数**: 无

**返回**: 确认信息

### exit_play_mode
退出播放模式。

**Profile**: core+full

**参数**: 无

**返回**: 确认信息

### simulate_action
模拟输入动作。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| action | string | 是 | InputMap 动作名 |
| mode | string | 否 | `"press"` / `"release"` / `"tap"`（默认 `"tap"`） |
| strength | float | 否 | 按键强度（默认 1.0） |

**返回**: 确认信息

### simulate_key_event
模拟键盘事件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| key | string | 否 | 按键名（如 `"enter"`, `"space"`, `"w"`） |
| physical_key | string | 否 | 物理按键 |
| mode | string | 否 | `"press"` / `"release"` / `"tap"` |

**返回**: 确认信息

### simulate_mouse_button
模拟鼠标按键。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| button | string | 否 | `"left"` / `"right"` / `"middle"`（默认 `"left"`） |
| position | object | 否 | `{"x": 100, "y": 200}` |
| mode | string | 否 | `"press"` / `"release"` / `"tap"` |

**返回**: 确认信息

### simulate_mouse_drag
模拟鼠标拖拽。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| from_position | object | 否 | 起始位置 `{"x": 0, "y": 0}` |
| to_position | object | 否 | 结束位置 |
| steps | int | 否 | 拖拽步数（默认 10） |
| button | string | 否 | 鼠标按键 |

**返回**: 确认信息

### simulate_input_sequence
模拟输入事件序列。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| events | array | 是 | 事件数组，每个事件包含 type 和参数 |

**返回**: 确认信息

### get_time_scale
获取时间缩放。

**Profile**: core+full

**参数**: 无

**返回**: 当前时间缩放值

### set_time_scale
设置时间缩放。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| value | float | 是 | 时间缩放值（0.0-10.0） |

**返回**: 确认信息

---

## 断言 (Assertions)

### assert_node_exists
断言节点存在或不存在。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| should_exist | bool | 否 | 期望是否存在（默认 true） |

**返回**: 断言结果（通过/失败）

### assert_node_property
断言节点属性值。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 节点路径 |
| property | string | 是 | 属性名 |
| expected | any | 是 | 期望值 |

**返回**: 断言结果

### assert_signal_connected
断言信号已连接。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| source_path | string | 是 | 信号源节点路径 |
| target_path | string | 是 | 目标节点路径 |
| signal_name | string | 是 | 信号名 |
| method_name | string | 是 | 回调方法名 |

**返回**: 断言结果

---

## 动画 (Animation)

### create_animation_player
创建 AnimationPlayer 节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| parent_path | string | 否 | 父节点路径 |
| name | string | 否 | 节点名称 |
| root_node | string | 否 | 动画根节点路径 |

**返回**: 创建的 AnimationPlayer 信息

### create_animation_clip
创建动画剪辑。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| animation_player_path | string | 是 | AnimationPlayer 路径 |
| animation_name | string | 是 | 动画名称 |
| library_name | string | 否 | 动画库名称 |
| length | float | 否 | 时长（默认 1.0） |
| loop_mode | int | 否 | 循环模式（0=none, 1=clamp, 2=loop） |
| step | float | 否 | 步长（默认 0.1） |

**返回**: 确认信息

### add_animation_track
添加动画轨道。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| animation_player_path | string | 是 | AnimationPlayer 路径 |
| animation_name | string | 是 | 动画名称 |
| track_type | string | 是 | 轨道类型：`"value"` / `"position_3d"` / `"rotation_3d"` / `"scale_3d"` / `"blend_shape"` / `"method"` / `"bezier"` / `"audio"` / `"animation"` |
| path | string | 是 | 轨道目标路径 |
| keys | array | 是 | 关键帧数组 `[{"time": 0, "value": ..., "transition": 1.0}]` |
| interpolation_type | int | 否 | 插值类型（0=nearest, 1=linear, 2=cubic） |
| update_mode | int | 否 | 更新模式（0=continuous, 1=discrete, 2=capture） |

**返回**: 确认信息

### list_animations
列出所有动画。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| animation_player_path | string | 是 | AnimationPlayer 路径 |

**返回**: 动画名称和时长列表

### play_animation
播放动画。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| animation_player_path | string | 是 | AnimationPlayer 路径 |
| animation_name | string | 是 | 动画名称 |

**返回**: 确认信息

---

## 摄像机 (Camera)

### get_camera_info
获取摄像机信息。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 摄像机节点路径 |

**返回**: 摄像机参数（投影类型、FOV、近远裁面等）

### set_camera_2d
设置 2D 摄像机参数。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | Camera2D 路径 |
| enabled | bool | 否 | 是否启用 |
| zoom | object | 否 | `{"x": 1, "y": 1}` |
| offset | object | 否 | `{"x": 0, "y": 0}` |
| position | object | 否 | `{"x": 0, "y": 0}` |
| limits | object | 否 | `{"left": -10000000, "top": -10000000, "right": 10000000, "bottom": 10000000}` |

**返回**: 确认信息

### set_camera_3d
设置 3D 摄像机参数。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | Camera3D 路径 |
| current | bool | 否 | 设为当前摄像机 |
| projection | int | 否 | 投影类型（0=perspective, 1=orthogonal） |
| fov | float | 否 | 视场角（默认 75） |
| size | float | 否 | 正交大小 |
| near | float | 否 | 近裁面（默认 0.05） |
| far | float | 否 | 远裁面（默认 4000） |
| position | object | 否 | `{"x": 0, "y": 0, "z": 0}` |
| rotation_degrees | object | 否 | `{"x": 0, "y": 0, "z": 0}` |

**返回**: 确认信息

---

## 材质 (Materials)

### create_material
创建材质资源。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 保存路径（如 `"res://materials/brick.tres"`） |
| material_type | string | 否 | 材质类型：`"StandardMaterial3D"` / `"ORMLiteMaterial3D"` / `"CanvasItemMaterial"`（默认 `"StandardMaterial3D"`） |
| properties | object | 否 | 材质属性设置 |

**返回**: 创建的材质信息

### assign_material
将材质指定给节点的 MeshInstance。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| target_path | string | 是 | 目标 MeshInstance 路径 |
| material_path | string | 是 | 材质资源路径 |
| surface_index | int | 否 | 表面索引（默认 0） |

**返回**: 确认信息

---

## UI 构建 (UI)

### create_ui_root
创建 UI 根节点（CanvasLayer 或 Control）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| kind | string | 否 | `"canvas_layer"` / `"control"`（默认 `"canvas_layer"`） |
| parent_path | string | 否 | 父节点路径 |
| name | string | 否 | 节点名称 |
| layout_preset | string | 否 | 布局预设 |

**返回**: 创建的节点信息

### create_control
创建 Control 节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| control_type | string | 是 | 控件类型（如 `"Button"`, `"Label"`, `"Panel"`） |
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |

**返回**: 创建的节点信息

### create_label
创建 Label 节点（便捷方法）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |
| text | string | 否 | 显示文本 |

**返回**: 创建的节点信息

### create_button
创建 Button 节点（便捷方法）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |
| text | string | 否 | 按钮文本 |

**返回**: 创建的节点信息

### create_panel
创建 Panel 节点（便捷方法）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |

**返回**: 创建的节点信息

### create_texture_rect
创建 TextureRect 节点（便捷方法）。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |
| texture_path | string | 否 | 纹理资源路径 |

**返回**: 创建的节点信息

### create_container
创建容器节点。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| container_type | string | 是 | 容器类型：`"VBoxContainer"` / `"HBoxContainer"` / `"MarginContainer"` / `"CenterContainer"` / `"ScrollContainer"` / `"GridContainer"` / `"TabContainer"` / `"PanelContainer"` |
| parent | string | 否 | 父节点路径 |
| name | string | 否 | 名称 |

**返回**: 创建的节点信息

### set_control_layout
设置控件布局。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 控件路径 |
| layout_preset | string | 否 | 布局预设（如 `"full_rect"`, `"center"`） |
| anchors | object | 否 | `{"left": 0, "top": 0, "right": 1, "bottom": 1}` |
| offsets | object | 否 | `{"left": 0, "top": 0, "right": 0, "bottom": 0}` |
| size | object | 否 | `{"x": 100, "y": 50}` |
| position | object | 否 | `{"x": 0, "y": 0}` |
| grow_horizontal | int | 否 | 水平增长方向 |
| grow_vertical | int | 否 | 垂直增长方向 |

**返回**: 确认信息

### set_control_size_flags
设置控件尺寸标志。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 控件路径 |
| horizontal | string | 否 | `"fill"` / `"expand"` / `"expand_fill"` / `"shrink_center"` / `"shrink_end"` |
| vertical | string | 否 | 同上 |
| stretch_ratio | float | 否 | 拉伸比例（默认 1.0） |

**返回**: 确认信息

### set_control_text
设置控件文本属性。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 控件路径 |
| text | string | 是 | 文本内容 |
| property | string | 否 | 属性名（默认 `"text"`，可为 `"placeholder_text"` 等） |

**返回**: 确认信息

### set_control_theme_override
设置控件主题覆盖。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 控件路径 |
| override_type | string | 是 | `"color"` / `"constant"` / `"font_size"` / `"font"` / `"stylebox"` |
| name | string | 是 | 覆盖项名称 |
| value | any | 否 | 值（用于 color/constant/font_size） |
| resource_path | string | 否 | 资源路径（用于 font/stylebox） |

**返回**: 确认信息

### set_control_texture
设置控件纹理。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| node_path | string | 是 | 控件路径 |
| texture_path | string | 是 | 纹理资源路径 |
| stretch_mode | int | 否 | 拉伸模式 |
| expand_mode | int | 否 | 展开模式 |

**返回**: 确认信息

### connect_node_signal
连接节点信号。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| source_path | string | 是 | 信号源节点路径 |
| target_path | string | 是 | 目标节点路径 |
| signal_name | string | 是 | 信号名 |
| method_name | string | 是 | 回调方法名 |
| flags | int | 否 | 连接标志（默认 0） |

**返回**: 确认信息

---

## 文件操作 (Files)

### list_files
列出目录中的文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 否 | 目录路径（默认 `"res://"`） |
| recursive | bool | 否 | 递归列出（默认 true） |
| include_hidden | bool | 否 | 包含隐藏文件（默认 false） |
| max_entries | int | 否 | 最大条目数（默认 100） |

**返回**: 文件路径列表

### search_files
搜索文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 否 | 搜索路径 |
| pattern | string | 是 | 搜索模式 |
| mode | string | 否 | `"name"` / `"content"` / `"regex"`（默认 `"name"`） |
| recursive | bool | 否 | 递归搜索 |
| max_results | int | 否 | 最大结果数 |

**返回**: 匹配的文件列表

### file_exists
检查文件是否存在。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 文件路径 |

**返回**: `true` / `false`

### read_file
读取文件内容。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 文件路径 |
| max_chars | int | 否 | 最大读取字符数（默认 10000） |

**返回**: 文件内容文本

### write_file
写入文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 文件路径 |
| content | string | 是 | 文件内容 |

**返回**: 确认信息

### select_file
在文件系统面板中选中文件。

**Profile**: core+full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 文件路径 |

**返回**: 确认信息

### delete_file
删除文件。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | 是 | 文件路径 |

**返回**: 确认信息

### move_file
移动/重命名文件。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| from_path | string | 是 | 源路径 |
| to_path | string | 是 | 目标路径 |

**返回**: 确认信息

### copy_file
复制文件。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| from_path | string | 是 | 源路径 |
| to_path | string | 是 | 目标路径 |

**返回**: 确认信息

---

## 插件 (Addons)

### list_addons
列出项目插件。

**Profile**: full

**参数**: 无

**返回**: 插件列表
```json
[
  {
    "name": "funplay_mcp",
    "has_plugin_cfg": true,
    "enabled": true
  }
]
```

### set_addon_enabled
启用/禁用插件。

**Profile**: full

**参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| addon | string | 是 | 插件名称 |
| enabled | bool | 是 | 是否启用 |

**返回**: 确认信息
