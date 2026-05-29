/**************************************************************************/
/*  mcp_sandbox.h                                                         */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* AI 代码执行沙箱 - 隔离错误影响，限制资源消耗                             */
/**************************************************************************/

#ifndef MCP_SANDBOX_H
#define MCP_SANDBOX_H

#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/templates/hash_set.h"

class MCPSandbox : public RefCounted {
	GDCLASS(MCPSandbox, RefCounted);

public:
	// 沙箱类型
	enum SandboxType {
		SANDBOX_GDSCRIPT, // GDScript 动态执行（加限制）
		SANDBOX_DRY_RUN,  // 只验证不执行
	};

	// 资源限制
	struct Limits {
		int max_memory_mb = 64;
		double max_time_sec = 5.0;
		int max_node_operations = 50;
		int max_file_operations = 20;
		int max_sub_instances = 5;     // 最多创建子实例数
		int max_output_length = 10000; // 输出最大字符数
	};

	// 构造 / 析构
	MCPSandbox();
	~MCPSandbox();

	// ---- 公开 API ----

	// 在沙箱中执行代码
	// 返回 Dictionary: { "success": bool, "result": Variant, "error": String, "logs": Array, "stats": Dictionary }
	Dictionary execute(const String &p_code, const Dictionary &p_context = Dictionary(), int p_type = 0);

	// 仅验证代码（不执行）
	bool validate_code(const String &p_code) const;

	// 设置允许访问的文件路径白名单
	void set_allowed_paths(const Vector<String> &p_paths);

	// 获取允许的路径列表
	Vector<String> get_allowed_paths() const;

	// 添加允许的路径
	void add_allowed_path(const String &p_path);

	// 移除允许的路径
	void remove_allowed_path(const String &p_path);

	// 检查路径是否在白名单中
	bool is_path_allowed(const String &p_path) const;

	// 设置/获取默认限制
	void set_default_limits(const Dictionary &p_limits);
	Dictionary get_default_limits() const;

	// 重置操作计数器
	void reset_counters();

	// 获取操作计数
	int get_node_operations_count() const;
	int get_file_operations_count() const;

	// 通知沙箱发生了节点操作（由外部工具调用）
	bool notify_node_operation();

	// 通知沙箱发生了文件操作（由外部工具调用）
	bool notify_file_operation();

protected:
	static void _bind_methods();

private:
	// 允许访问的路径
	HashSet<String> _allowed_paths;

	// 操作计数器
	int _node_ops_count = 0;
	int _file_ops_count = 0;
	int _sub_instance_count = 0;

	// 默认限制
	Limits _default_limits;

	// 检查是否超出限制
	bool _check_limits(const Limits &p_limits) const;

	// 将 Limits 转为 Dictionary
	Dictionary _limits_to_dict(const Limits &p_limits) const;

	// 从 Dictionary 读取 Limits
	Limits _dict_to_limits(const Dictionary &p_dict) const;

	// 构建安全执行上下文
	Dictionary _build_safe_context(const Dictionary &p_user_context) const;

	// 包装代码到安全的 GDScript 模板
	String _wrap_code(const String &p_code) const;

	// 截断过长输出
	String _truncate_output(const String &p_output, int p_max_length) const;

	// 解析执行错误信息
	String _parse_error_message(const String &p_raw_error) const;
};

#endif // MCP_SANDBOX_H
