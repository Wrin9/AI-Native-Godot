/**
 * material_tools.h - 材质工具
 *
 * 材质资源的创建和分配工具。
 * 从 funplay_core_tools.gd 的材质相关方法迁移而来。
 */

#ifndef MATERIAL_TOOLS_H
#define MATERIAL_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;
class Node;

class MaterialTools : public RefCounted {
	GDCLASS(MaterialTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建材质
	String create_material(const Dictionary &p_args);

	// 分配材质
	String assign_material(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	Node *_get_edited_scene_root() const;
	Node *_resolve_node_path(const String &p_path) const;
	String _normalize_path(const String &p_path) const;
	String _ensure_parent_dir(const String &p_path) const;
	Dictionary _node_to_summary(Node *p_node) const;
	bool _has_property(Object *p_object, const String &p_property) const;
	void _refresh_filesystem() const;
};

#endif // MATERIAL_TOOLS_H
