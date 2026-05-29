/**************************************************************************/
/*  mcp_snapshot.h                                                        */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 编辑器状态快照 - 用于增量同步                                            */
/* 捕获场景树、项目文件等状态，支持全量和增量快照                            */
/**************************************************************************/

#ifndef MCP_SNAPSHOT_H
#define MCP_SNAPSHOT_H

#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

class Node;

class MCPSnapshot : public RefCounted {
	GDCLASS(MCPSnapshot, RefCounted);

public:
	// 构造 / 析构
	MCPSnapshot();
	~MCPSnapshot();

	// ---- 公开 API ----

	// 捕获完整快照
	Dictionary capture_full();

	// 捕获增量快照（与前一次快照对比）
	Dictionary capture_diff();

	// 获取场景树快照
	Dictionary get_scene_tree_snapshot(int p_max_depth = -1);

	// 获取项目文件快照
	Dictionary get_project_files_snapshot() const;

	// 获取当前编辑器选择快照
	Dictionary get_selection_snapshot() const;

	// 获取脚本列表快照
	Dictionary get_scripts_snapshot() const;

	// 获取项目设置快照（仅包含部分常用设置）
	Dictionary get_project_settings_snapshot() const;

	// 设置/获取最大场景树深度
	void set_max_depth(int p_depth);
	int get_max_depth() const;

	// 设置/获取是否包含资源路径
	void set_include_resource_paths(bool p_include);
	bool get_include_resource_paths() const;

	// 获取上次快照的哈希（用于快速判断是否有变化）
	String get_last_snapshot_hash() const;

	// 清除快照缓存
	void clear_cache();

protected:
	static void _bind_methods();

private:
	// 上次完整快照数据
	Dictionary _last_full_snapshot;

	// 上次快照的哈希
	String _last_snapshot_hash;

	// 配置
	int _max_depth = -1; // -1 表示无限制
	bool _include_resource_paths = true;

	// 缓存的场景树（避免每帧重新遍历）
	Dictionary _cached_scene_tree;
	uint64_t _cache_timestamp_msec = 0;
	uint64_t _cache_ttl_msec = 200; // 缓存有效期 200ms

	// 内部方法
	void _walk_scene_tree(Node *p_node, Dictionary &p_out, int p_depth, int p_max_depth);
	Dictionary _node_to_dict(Node *p_node) const;
	String _compute_hash(const Dictionary &p_snapshot) const;

	// 收集项目文件
	void _collect_project_files(const String &p_path, Array &p_files, int p_depth = 0) const;

	// 比较两个字典的差异
	Dictionary _diff_dictionaries(const Dictionary &p_old, const Dictionary &p_new, const String &p_prefix = "") const;

	// 比较场景树差异
	Dictionary _diff_scene_trees(const Dictionary &p_old_tree, const Dictionary &p_new_tree) const;
};

#endif // MCP_SNAPSHOT_H
