/**************************************************************************/
/*  mcp_command_queue.h                                                   */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 命令队列核心 - 替代 GDScript 中的直接同步执行                           */
/* 提供优先级队列、事务支持、超时机制、线程安全的命令调度                    */
/**************************************************************************/

#ifndef MCP_COMMAND_QUEUE_H
#define MCP_COMMAND_QUEUE_H

#include "core/object/object.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/templates/hash_set.h"
#include "core/os/mutex.h"

#include <queue>
#include <vector>
#include <functional>

class MCPCommandQueue : public Object {
	GDCLASS(MCPCommandQueue, Object);

public:
	// 命令优先级
	enum Priority {
		PRIORITY_LOW = 0,
		PRIORITY_NORMAL = 1,
		PRIORITY_HIGH = 2,
		PRIORITY_CRITICAL = 3,
	};

	// 命令状态
	enum CommandStatus {
		STATUS_QUEUED = 0,
		STATUS_EXECUTING = 1,
		STATUS_COMPLETED = 2,
		STATUS_FAILED = 3,
		STATUS_ROLLED_BACK = 4,
	};

	// 暴露给 GDScript 的常量（避免 BIND_ENUM_CONSTANT 的 GetTypeInfo 特化问题）
	static const int PRIORITY_LOW_VAL = PRIORITY_LOW;
	static const int PRIORITY_NORMAL_VAL = PRIORITY_NORMAL;
	static const int PRIORITY_HIGH_VAL = PRIORITY_HIGH;
	static const int PRIORITY_CRITICAL_VAL = PRIORITY_CRITICAL;
	static const int STATUS_QUEUED_VAL = STATUS_QUEUED;
	static const int STATUS_EXECUTING_VAL = STATUS_EXECUTING;
	static const int STATUS_COMPLETED_VAL = STATUS_COMPLETED;
	static const int STATUS_FAILED_VAL = STATUS_FAILED;
	static const int STATUS_ROLLED_BACK_VAL = STATUS_ROLLED_BACK;

	// 命令结构体
	struct Command {
		String id;
		String tool_name;
		Dictionary arguments;
		Priority priority = PRIORITY_NORMAL;
		uint64_t enqueued_at_msec = 0;
		uint64_t timeout_msec = 10000;
		CommandStatus status = STATUS_QUEUED;
		Dictionary result;       // 执行结果
		String error_message;    // 错误信息
		String undo_action;      // 回滚操作标识
		Dictionary undo_args;    // 回滚操作参数
		String transaction_id;   // 所属事务 ID（空则不属于事务）
		int transaction_index = -1; // 在事务中的序号

		// 默认构造（std::priority_queue 需要）
		Command() = default;
	};

	// 事务结构体
	struct Transaction {
		String id;
		Vector<Command> commands;
		Priority priority = PRIORITY_NORMAL;
		uint64_t created_at_msec = 0;
		int completed_count = 0;
		int failed_index = -1;   // 失败命令在事务中的索引
		bool rolling_back = false;
		int rolled_back_count = 0;
	};

	// 构造 / 析构
	MCPCommandQueue();
	~MCPCommandQueue();

	// ---- 公开 API ----

	// 入队单个命令，返回命令 ID
	String enqueue(const String &p_tool, const Dictionary &p_args, int p_priority = 1, uint64_t p_timeout_msec = 0);

	// 入队事务（多命令原子执行），返回事务 ID
	String enqueue_transaction(const Array &p_commands_data, int p_priority = 1);

	// 每帧调用，处理队列中待执行的命令
	void process_queue();

	// 获取队列中等待执行的命令数
	int get_queue_size() const;

	// 获取统计信息
	Dictionary get_stats() const;

	// 清空队列（会取消所有等待中的命令）
	void clear_queue();

	// 取消指定命令
	bool cancel_command(const String &p_id);

	// 获取命令状态
	Dictionary get_command_status(const String &p_id) const;

	// 获取事务状态
	Dictionary get_transaction_status(const String &p_id) const;

	// ---- 配置属性 ----

	// 每帧最多执行命令数
	int max_commands_per_frame;
	// 命令间最小间隔（毫秒）
	uint64_t min_interval_msec;
	// 队列最大容量
	int max_queue_size;
	// 默认超时时间（毫秒）
	uint64_t default_timeout_msec;

	// 设置执行回调（外部注入实际工具调用逻辑）
	void set_execute_callback(const Callable &p_callback);

	// 属性访问器
	void set_max_commands_per_frame(int p_value) { max_commands_per_frame = p_value; }
	int get_max_commands_per_frame() const { return max_commands_per_frame; }
	void set_min_interval_msec(uint64_t p_value) { min_interval_msec = p_value; }
	uint64_t get_min_interval_msec() const { return min_interval_msec; }
	void set_max_queue_size(int p_value) { max_queue_size = p_value; }
	int get_max_queue_size() const { return max_queue_size; }
	void set_default_timeout_msec(uint64_t p_value) { default_timeout_msec = p_value; }
	uint64_t get_default_timeout_msec() const { return default_timeout_msec; }

protected:
	static void _bind_methods();

private:
	// 优先级比较器（priority_queue 默认最大堆，我们希望 CRITICAL 先出队）
	struct PriorityCompare {
		bool operator()(const Command &a, const Command &b) const {
			// 优先级相同则先入队的先执行（FIFO）
			if (a.priority == b.priority) {
				return a.enqueued_at_msec > b.enqueued_at_msec;
			}
			return a.priority < b.priority;
		}
	};

	// 优先级队列
	std::priority_queue<Command, std::vector<Command>, PriorityCompare> _queue;

	// 命令索引（按 ID 查找）
	HashMap<String, Command> _commands_by_id;

	// 事务索引
	HashMap<String, Transaction> _transactions;

	// 执行回调
	Callable _execute_callback;

	// 互斥锁
	mutable Mutex _mutex;

	// 上次执行时间
	uint64_t _last_execute_msec = 0;

	// 是否正在执行中（防止重入）
	bool _executing = false;

	// 统计计数
	int _total_enqueued = 0;
	int _total_completed = 0;
	int _total_failed = 0;
	int _total_rolled_back = 0;

	// 内部方法
	void _execute_command(const Command &p_cmd);
	void _emit_result(const String &p_cmd_id, const Dictionary &p_result);
	void _emit_error(const String &p_cmd_id, const String &p_error);
	void _check_timeouts();
	void _process_transaction_step(const String &p_transaction_id);
	void _rollback_transaction(const String &p_transaction_id);
	String _generate_id() const;
	void _update_command_status(const String &p_id, int p_status);
};

#endif // MCP_COMMAND_QUEUE_H
