# MCP Tool 开发指南

## 文件结构
每个工具组两个文件：`xxx_tools.h` 和 `xxx_tools.cpp`，放在 `F:/game/godot-source/modules/mcp_editor/tools/`

## 必须遵守的规则

### 头文件模板
```cpp
#ifndef XXX_TOOLS_H
#define XXX_TOOLS_H
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
class EditorPlugin;
class XxxTools : public RefCounted {
    GDCLASS(XxxTools, RefCounted);
public:
    void set_editor_plugin(EditorPlugin *p_plugin);
    // 每个工具一个 String 方法
    String tool_name(const Dictionary &p_args);
protected:
    static void _bind_methods();
private:
    EditorPlugin *_plugin = nullptr;
};
#endif
```

### CPP 关键规则
1. **包含共享辅助头文件**: `#include "mcp_tool_helpers.h"`（已存在于 tools/ 目录）
2. **包含**: `#include "editor/editor_interface.h"`, `#include "core/io/json.h"`
3. **_bind_methods**: 每个公开方法 `ClassDB::bind_method(D_METHOD("name","args"), &Class::name, DEFVAL(Dictionary()));`
4. **返回 JSON**: 用 `JSON::stringify(dict, "\t")` 生成
5. **参数获取**: `p_args.get("param", default_value)`
6. **节点查找**: 用 `mcp_resolve_node_path(path)` 从 mcp_tool_helpers.h
7. **场景根**: 用 `mcp_get_scene_root()` 从 mcp_tool_helpers.h
8. **路径转换**: 用 `mcp_to_absolute(path)` 将 res:// 转绝对路径
9. **路径规范化**: 用 `mcp_normalize_path(path)`
10. **DirAccess 循环**: MUST skip "." and ".."
11. **中文注释**
12. **错误返回格式**: `R"json({"error": "message"})json"` 或 `vformat(R"json({"error": "%s"})json", var)`

### mcp_tool_helpers.h 提供的函数（直接 #include 即可）
- `mcp_resolve_node_path(String)` → `Node*` — 支持相对路径/绝对路径/实例ID
- `mcp_get_scene_root()` → `Node*`
- `mcp_to_absolute(String)` → `String` — res:// 转绝对路径
- `mcp_normalize_path(String)` → `String` — 规范化路径
- `mcp_node_summary(Node*)` → `Dictionary`

### Godot 4.6 API 注意事项
- `EditorInterface::get_singleton()` 获取编辑器接口
- `ClassDB::instantiate(class_name)` 创建对象
- `ResourceSaver::save(resource, path)` 保存资源
- `ResourceLoader::load(path)` 加载资源
- 节点添加: `parent->add_child(node); node->set_owner(scene_root);`
- `memdelete(obj)` 释放未使用的对象
- `Object::cast_to<T>(obj)` 类型转换
- `String::utf8().get_data()` 转 const char*

### 编译环境
- MSVC 14.40 (VS 2022)
- `JSON::stringify(var, "\t")` 需要2个参数
- `Dictionary::get(key, default)` 需要2个参数
- 不用 `MODULE_REGISTRATION_CLASS` 宏
