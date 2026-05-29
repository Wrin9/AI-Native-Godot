/**
 * diagnostic_tools.h - 诊断工具
 *
 * 控制台日志、性能快照、场景复杂度分析等工具的 C++ 实现。
 * 从 funplay_core_tools.gd 的诊断相关方法迁移而来。
 */

#ifndef DIAGNOSTIC_TOOLS_H
#define DIAGNOSTIC_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class Node;

class EditorPlugin;

class DiagnosticTools : public RefCounted {
	GDCLASS(DiagnosticTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 获取控制台日志
	String get_console_logs(const Dictionary &p_args);

	// 获取性能快照
	String get_performance_snapshot(const Dictionary &p_args);

	// 分析场景复杂度
	String analyze_scene_complexity(const Dictionary &p_args);

	// 获取项目信息
	String get_project_info(const Dictionary &p_args);

	// 映射项目结构
	String map_project(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;

	// 内部辅助方法
	int _count_nodes(Node *p_node) const;
	void _analyze_node_recursive(Node *p_node, int p_depth, Dictionary &p_stats) const;
	void _collect_matching_files(const String &p_path, bool p_recursive, int p_max_entries, Array &p_results, const Vector<String> &p_extensions) const;
	String _to_absolute(const String &p_path) const;
};

#endif // DIAGNOSTIC_TOOLS_H
