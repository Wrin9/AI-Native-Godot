/**************************************************************************/
/*  mcp_command_queue.cpp                                                 */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 命令队列核心实现                                                         */
/**************************************************************************/

#include "mcp_command_queue.h"

#include "core/os/os.h"
#include "core/variant/callable.h"

// ============================================================
// 构造 / 析构
// ============================================================

MCPCommandQueue::MCPCommandQueue() :
		max_commands_per_frame(3),
		min_interval_msec(50),
		max_queue_size(100),
		default_timeout_msec(10000),
		_mutex(Mutex::create()) {
}

MCPCommandQueue::~MCPCommandQueue() {
	if (_mutex) {
		memdelete(_mutex);
		_mutex = nullptr;
	}
}

// ============================================================
// 公开 API
// ============================================================

String MCPCommandQueue::enqueue(const String &p_tool, const Dictionary &p_args, Priority p_priority, uint64_t p_timeout_msec) {
	_mutex->lock();

	// 检查队列容量
	if ((int)_commands_by_id.size() >= max_queue_size) {
		_mutex->unlock();
		WARN_PRINT(vformat("MCPCommandQueue: 队列已满（%d/%d），拒绝命令 '%s'", _commands_by_id.size(), max_queue_size, p_tool));
		_emit_error("", vformat("Queue full (%d/%d)", _commands_by_id.size(), max_queue_size));
		return "";
	}

	Command cmd;
	cmd.id = _generate_id();
	cmd.tool_name = p_tool;
	cmd.arguments = p_args;
	cmd.priority = p_priority;
	cmd.enqueued_at_msec = OS::get_singleton()->get_ticks_msec();
	cmd.timeout_msec = (p_timeout_msec > 0) ? p_timeout_msec : default_timeout_msec;
	cmd.status = STATUS_QUEUED;

	_commands_by_id[cmd.id] = cmd;
	_queue.push(cmd);
	_total_enqueued++;

	_mutex->unlock();

	// 发出命令入队信号
	emit_signal("command_queued", cmd.id, p_tool);

	return cmd.id;
}

String MCPCommandQueue::enqueue_transaction(const Vector<Dictionary> &p_commands_data, Priority p_priority) {
	if (p_commands_data.is_empty()) {
		WARN_PRINT("MCPCommandQueue: 事务命令列表为空");
		return "";
	}

	_mutex->lock();

	// 检查队列容量（含事务中所有命令）
	int projected_size = (int)_commands_by_id.size() + p_commands_data.size();
	if (projected_size > max_queue_size) {
		_mutex->unlock();
		WARN_PRINT(vformat("MCPCommandQueue: 队列容量不足，无法添加事务（需要 %d，当前 %d/%d）",
				p_commands_data.size(), _commands_by_id.size(), max_queue_size));
		return "";
	}

	String tx_id = _generate_id();
	tx_id = "tx_" + tx_id;

	Transaction tx;
	tx.id = tx_id;
	tx.priority = p_priority;
	tx.created_at_msec = OS::get_singleton()->get_ticks_msec();

	uint64_t now = OS::get_singleton()->get_ticks_msec();

	for (int i = 0; i < p_commands_data.size(); i++) {
		const Dictionary &cmd_data = p_commands_data[i];

		Command cmd;
		cmd.id = _generate_id();
		cmd.tool_name = cmd_data.get("tool", "");
		cmd.arguments = cmd_data.get("arguments", Dictionary());
		cmd.priority = p_priority; // 事务内所有命令继承事务优先级
		cmd.enqueued_at_msec = now;
		cmd.timeout_msec = default_timeout_msec;
		cmd.status = STATUS_QUEUED;
		cmd.transaction_id = tx_id;
		cmd.transaction_index = i;

		// 从命令数据中提取 undo 信息
		cmd.undo_action = cmd_data.get("undo_action", "");
		cmd.undo_args = cmd_data.get("undo_args", Dictionary());

		// 验证工具名不为空
		if (cmd.tool_name.is_empty()) {
			_mutex->unlock();
			WARN_PRINT(vformat("MCPCommandQueue: 事务 '%s' 第 %d 个命令缺少工具名", tx_id, i));
			return "";
		}

		tx.commands.push_back(cmd);
		_commands_by_id[cmd.id] = cmd;
		_total_enqueued++;
	}

	_transactions[tx_id] = tx;

	// 只将事务的第一个命令入队，后续命令在事务推进时入队
	_queue.push(tx.commands[0]);

	_mutex->unlock();

	emit_signal("transaction_queued", tx_id, p_commands_data.size());

	return tx_id;
}

void MCPCommandQueue::process_queue() {
	if (_executing) {
		return; // 防止重入
	}

	// 检查超时
	_check_timeouts();

	uint64_t now = OS::get_singleton()->get_ticks_msec();

	// 检查执行间隔
	if (now - _last_execute_msec < min_interval_msec) {
		return;
	}

	_executing = true;
	int executed = 0;

	while (executed < max_commands_per_frame && !_queue.empty()) {
		_mutex->lock();
		if (_queue.empty()) {
			_mutex->unlock();
			break;
		}

		Command cmd = _queue.top();
		_queue.pop();

		// 检查命令是否已被取消
		if (!_commands_by_id.has(cmd.id)) {
			_mutex->unlock();
			continue;
		}

		// 更新状态为执行中
		cmd.status = STATUS_EXECUTING;
		_commands_by_id[cmd.id] = cmd;
		_mutex->unlock();

		_execute_command(cmd);

		_last_execute_msec = OS::get_singleton()->get_ticks_msec();
		executed++;
	}

	_executing = false;
}

int MCPCommandQueue::get_queue_size() const {
	_mutex->lock();
	int size = (int)_queue.size();
	_mutex->unlock();
	return size;
}

Dictionary MCPCommandQueue::get_stats() const {
	Dictionary stats;
	stats["queue_size"] = get_queue_size();
	stats["total_commands_by_id"] = (int)_commands_by_id.size();
	stats["transactions_count"] = (int)_transactions.size();
	stats["total_enqueued"] = _total_enqueued;
	stats["total_completed"] = _total_completed;
	stats["total_failed"] = _total_failed;
	stats["total_rolled_back"] = _total_rolled_back;
	stats["max_commands_per_frame"] = max_commands_per_frame;
	stats["min_interval_msec"] = (int64_t)min_interval_msec;
	stats["max_queue_size"] = max_queue_size;
	stats["default_timeout_msec"] = (int64_t)default_timeout_msec;
	return stats;
}

void MCPCommandQueue::clear_queue() {
	_mutex->lock();

	// 清空优先级队列（std::priority_queue 没有 clear 方法）
	while (!_queue.empty()) {
		_queue.pop();
	}

	// 向所有等待中的命令发送取消信号
	for (const KeyValue<String, Command> &E : _commands_by_id) {
		if (E.value.status == STATUS_QUEUED) {
			Command &cmd = _commands_by_id[E.key];
			cmd.status = STATUS_FAILED;
			cmd.error_message = "Queue cleared";
		}
	}

	_commands_by_id.clear();
	_transactions.clear();

	_mutex->unlock();

	emit_signal("queue_cleared");
}

bool MCPCommandQueue::cancel_command(const String &p_id) {
	_mutex->lock();

	if (!_commands_by_id.has(p_id)) {
		_mutex->unlock();
		return false;
	}

	Command &cmd = _commands_by_id[p_id];
	if (cmd.status != STATUS_QUEUED) {
		// 只能取消等待中的命令
		_mutex->unlock();
		return false;
	}

	cmd.status = STATUS_FAILED;
	cmd.error_message = "Cancelled by user";

	// 如果属于事务，取消整个事务
	if (!cmd.transaction_id.is_empty() && _transactions.has(cmd.transaction_id)) {
		String tx_id = cmd.transaction_id;
		_mutex->unlock();
		_rollback_transaction(tx_id);
		return true;
	}

	_commands_by_id.erase(p_id);
	_mutex->unlock();

	emit_signal("command_cancelled", p_id);
	return true;
}

Dictionary MCPCommandQueue::get_command_status(const String &p_id) const {
	Dictionary result;
	_mutex->lock();

	if (!_commands_by_id.has(p_id)) {
		result["found"] = false;
		_mutex->unlock();
		return result;
	}

	const Command &cmd = _commands_by_id[p_id];
	result["found"] = true;
	result["id"] = cmd.id;
	result["tool_name"] = cmd.tool_name;
	result["status"] = cmd.status;
	result["priority"] = cmd.priority;
	result["enqueued_at_msec"] = (int64_t)cmd.enqueued_at_msec;
	result["timeout_msec"] = (int64_t)cmd.timeout_msec;
	result["error_message"] = cmd.error_message;
	result["transaction_id"] = cmd.transaction_id;

	_mutex->unlock();
	return result;
}

Dictionary MCPCommandQueue::get_transaction_status(const String &p_id) const {
	Dictionary result;
	_mutex->lock();

	if (!_transactions.has(p_id)) {
		result["found"] = false;
		_mutex->unlock();
		return result;
	}

	const Transaction &tx = _transactions[p_id];
	result["found"] = true;
	result["id"] = tx.id;
	result["priority"] = tx.priority;
	result["created_at_msec"] = (int64_t)tx.created_at_msec;
	result["total_commands"] = tx.commands.size();
	result["completed_count"] = tx.completed_count;
	result["failed_index"] = tx.failed_index;
	result["rolling_back"] = tx.rolling_back;
	result["rolled_back_count"] = tx.rolled_back_count;

	// 判定事务整体状态
	String overall_status;
	if (tx.failed_index >= 0) {
		if (tx.rolling_back) {
			overall_status = "rolling_back";
		} else if (tx.rolled_back_count > 0) {
			overall_status = "rolled_back";
		} else {
			overall_status = "failed";
		}
	} else if (tx.completed_count == tx.commands.size()) {
		overall_status = "completed";
	} else {
		overall_status = "in_progress";
	}
	result["status"] = overall_status;

	_mutex->unlock();
	return result;
}

void MCPCommandQueue::set_execute_callback(const Callable &p_callback) {
	_execute_callback = p_callback;
}

// ============================================================
// 内部方法
// ============================================================

void MCPCommandQueue::_execute_command(const Command &p_cmd) {
	// 发出命令开始信号
	emit_signal("command_started", p_cmd.id, p_cmd.tool_name);

	Dictionary exec_result;
	bool success = false;

	if (_execute_callback.is_valid()) {
		// 通过回调执行工具调用
		Variant ret = _execute_callback.call(p_cmd.tool_name, p_cmd.arguments);
		if (ret.get_type() == Variant::DICTIONARY) {
			exec_result = ret;
			success = !exec_result.get("isError", false);
		} else {
			// 兼容返回字符串结果
			String text = ret;
			success = !text.begins_with("Error:");
			exec_result["content"] = text;
			exec_result["isError"] = !success;
		}
	} else {
		// 没有设置执行回调，尝试通过类方法调用
		String method_name = "execute_" + p_cmd.tool_name;
		if (has_method(method_name)) {
			Variant ret = call(method_name, p_cmd.arguments);
			if (ret.get_type() == Variant::DICTIONARY) {
				exec_result = ret;
			} else {
				exec_result["content"] = ret;
			}
			success = true;
		} else {
			exec_result["isError"] = true;
			exec_result["error"] = vformat("No execute callback set and method '%s' not found", method_name);
			success = false;
		}
	}

	_mutex->lock();
	if (!_commands_by_id.has(p_cmd.id)) {
		// 命令已被清理
		_mutex->unlock();
		return;
	}

	Command &cmd_ref = _commands_by_id[p_cmd.id];
	if (success) {
		cmd_ref.status = STATUS_COMPLETED;
		cmd_ref.result = exec_result;
		_total_completed++;
	} else {
		cmd_ref.status = STATUS_FAILED;
		cmd_ref.error_message = exec_result.get("error", "Unknown error");
		_total_failed++;
	}
	_mutex->unlock();

	// 发出结果信号
	if (success) {
		_emit_result(p_cmd.id, exec_result);

		// 如果属于事务，推进事务
		if (!p_cmd.transaction_id.is_empty()) {
			_process_transaction_step(p_cmd.transaction_id);
		}
	} else {
		_emit_error(p_cmd.id, cmd_ref.error_message);

		// 如果属于事务，触发回滚
		if (!p_cmd.transaction_id.is_empty()) {
			_mutex->lock();
			if (_transactions.has(p_cmd.transaction_id)) {
				_transactions[p_cmd.transaction_id].failed_index = p_cmd.transaction_index;
			}
			_mutex->unlock();
			_rollback_transaction(p_cmd.transaction_id);
		}
	}
}

void MCPCommandQueue::_emit_result(const String &p_cmd_id, const Dictionary &p_result) {
	emit_signal("command_completed", p_cmd_id, p_result);
}

void MCPCommandQueue::_emit_error(const String &p_cmd_id, const String &p_error) {
	emit_signal("command_failed", p_cmd_id, p_error);
}

void MCPCommandQueue::_check_timeouts() {
	uint64_t now = OS::get_singleton()->get_ticks_msec();

	_mutex->lock();

	// 收集超时命令 ID
	Vector<String> timed_out;
	for (const KeyValue<String, Command> &E : _commands_by_id) {
		const Command &cmd = E.value;
		if (cmd.status == STATUS_QUEUED || cmd.status == STATUS_EXECUTING) {
			if (now - cmd.enqueued_at_msec > cmd.timeout_msec) {
				timed_out.push_back(cmd.id);
			}
		}
	}

	_mutex->unlock();

	// 处理超时
	for (const String &id : timed_out) {
		_mutex->lock();
		if (_commands_by_id.has(id)) {
			Command &cmd = _commands_by_id[id];
			cmd.status = STATUS_FAILED;
			cmd.error_message = "Command timed out";
			_total_failed++;

			String tx_id = cmd.transaction_id;
			_mutex->unlock();

			_emit_error(id, "Command timed out");

			// 事务中的命令超时，触发回滚
			if (!tx_id.is_empty()) {
				_mutex->lock();
				if (_transactions.has(tx_id)) {
					_transactions[tx_id].failed_index = cmd.transaction_index;
				}
				_mutex->unlock();
				_rollback_transaction(tx_id);
			}
		} else {
			_mutex->unlock();
		}
	}
}

void MCPCommandQueue::_process_transaction_step(const String &p_transaction_id) {
	_mutex->lock();

	if (!_transactions.has(p_transaction_id)) {
		_mutex->unlock();
		return;
	}

	Transaction &tx = _transactions[p_transaction_id];
	tx.completed_count++;

	// 事务是否全部完成
	if (tx.completed_count >= tx.commands.size()) {
		_mutex->unlock();
		emit_signal("transaction_completed", p_transaction_id);
		return;
	}

	// 推进到下一步：将下一个命令入队
	int next_index = tx.completed_count;
	if (next_index < tx.commands.size()) {
		Command next_cmd = tx.commands[next_index];
		_queue.push(next_cmd);
	}

	_mutex->unlock();
}

void MCPCommandQueue::_rollback_transaction(const String &p_transaction_id) {
	_mutex->lock();

	if (!_transactions.has(p_transaction_id)) {
		_mutex->unlock();
		return;
	}

	Transaction &tx = _transactions[p_transaction_id];
	tx.rolling_back = true;

	int rollback_count = 0;

	// 从失败命令之前的位置开始，逆序回滚
	for (int i = tx.failed_index - 1; i >= 0; i--) {
		const Command &cmd = tx.commands[i];

		// 只有已完成的命令才需要回滚
		if (cmd.status != STATUS_COMPLETED) {
			continue;
		}

		// 检查是否有 undo 操作
		if (!cmd.undo_action.is_empty()) {
			_mutex->unlock();

			// 通过执行回调调用 undo 操作
			if (_execute_callback.is_valid()) {
				Dictionary undo_args = cmd.undo_args;
				undo_args["_undo_for"] = cmd.id;
				_execute_callback.call(cmd.undo_action, undo_args);
			}

			_mutex->lock();
		}

		// 更新命令状态
		if (_commands_by_id.has(cmd.id)) {
			_commands_by_id[cmd.id].status = STATUS_ROLLED_BACK;
		}

		rollback_count++;
	}

	tx.rolled_back_count = rollback_count;
	_total_rolled_back += rollback_count;

	// 取消事务中尚未执行的命令
	for (int i = tx.failed_index + 1; i < tx.commands.size(); i++) {
		const Command &cmd = tx.commands[i];
		if (_commands_by_id.has(cmd.id)) {
			_commands_by_id[cmd.id].status = STATUS_FAILED;
			_commands_by_id[cmd.id].error_message = "Transaction rolled back";
		}
	}

	_mutex->unlock();

	emit_signal("transaction_rolled_back", p_transaction_id, rollback_count);
}

String MCPCommandQueue::_generate_id() const {
	// 使用时间戳 + 随机数生成唯一 ID
	uint64_t msec = OS::get_singleton()->get_ticks_msec();
	uint32_t rand = Math::rand();
	return vformat("cmd_%d_%d", msec, rand);
}

void MCPCommandQueue::_update_command_status(const String &p_id, CommandStatus p_status) {
	_mutex->lock();
	if (_commands_by_id.has(p_id)) {
		_commands_by_id[p_id].status = p_status;
	}
	_mutex->unlock();
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPCommandQueue::_bind_methods() {
	// 绑定枚举
	BIND_ENUM_CONSTANT(PRIORITY_LOW);
	BIND_ENUM_CONSTANT(PRIORITY_NORMAL);
	BIND_ENUM_CONSTANT(PRIORITY_HIGH);
	BIND_ENUM_CONSTANT(PRIORITY_CRITICAL);

	BIND_ENUM_CONSTANT(STATUS_QUEUED);
	BIND_ENUM_CONSTANT(STATUS_EXECUTING);
	BIND_ENUM_CONSTANT(STATUS_COMPLETED);
	BIND_ENUM_CONSTANT(STATUS_FAILED);
	BIND_ENUM_CONSTANT(STATUS_ROLLED_BACK);

	// 绑定方法
	ClassDB::bind_method(D_METHOD("enqueue", "tool", "args", "priority", "timeout_msec"),
			&MCPCommandQueue::enqueue,
			DEFVAL(PRIORITY_NORMAL),
			DEFVAL((uint64_t)0));

	ClassDB::bind_method(D_METHOD("enqueue_transaction", "commands_data", "priority"),
			&MCPCommandQueue::enqueue_transaction,
			DEFVAL(PRIORITY_NORMAL));

	ClassDB::bind_method(D_METHOD("process_queue"), &MCPCommandQueue::process_queue);
	ClassDB::bind_method(D_METHOD("get_queue_size"), &MCPCommandQueue::get_queue_size);
	ClassDB::bind_method(D_METHOD("get_stats"), &MCPCommandQueue::get_stats);
	ClassDB::bind_method(D_METHOD("clear_queue"), &MCPCommandQueue::clear_queue);
	ClassDB::bind_method(D_METHOD("cancel_command", "id"), &MCPCommandQueue::cancel_command);
	ClassDB::bind_method(D_METHOD("get_command_status", "id"), &MCPCommandQueue::get_command_status);
	ClassDB::bind_method(D_METHOD("get_transaction_status", "id"), &MCPCommandQueue::get_transaction_status);
	ClassDB::bind_method(D_METHOD("set_execute_callback", "callback"), &MCPCommandQueue::set_execute_callback);

	// 绑定属性
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_commands_per_frame", PROPERTY_HINT_RANGE, "1,20,1"), "set_max_commands_per_frame", "get_max_commands_per_frame");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_interval_msec", PROPERTY_HINT_RANGE, "0,500,1"), "set_min_interval_msec", "get_min_interval_msec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_queue_size", PROPERTY_HINT_RANGE, "10,1000,1"), "set_max_queue_size", "get_max_queue_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "default_timeout_msec", PROPERTY_HINT_RANGE, "1000,60000,1000"), "set_default_timeout_msec", "get_default_timeout_msec");

	// 信号
	ADD_SIGNAL(MethodInfo("command_queued",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::STRING, "tool_name")));

	ADD_SIGNAL(MethodInfo("command_started",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::STRING, "tool_name")));

	ADD_SIGNAL(MethodInfo("command_completed",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::DICTIONARY, "result")));

	ADD_SIGNAL(MethodInfo("command_failed",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::STRING, "error")));

	ADD_SIGNAL(MethodInfo("command_cancelled",
			PropertyInfo(Variant::STRING, "id")));

	ADD_SIGNAL(MethodInfo("transaction_queued",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::INT, "command_count")));

	ADD_SIGNAL(MethodInfo("transaction_completed",
			PropertyInfo(Variant::STRING, "id")));

	ADD_SIGNAL(MethodInfo("transaction_rolled_back",
			PropertyInfo(Variant::STRING, "id"),
			PropertyInfo(Variant::INT, "rolled_back_count")));

	ADD_SIGNAL(MethodInfo("queue_cleared"));
}

// 属性 getter/setter 是内联的需求，在头文件中声明
// 以下为非内联实现（因为需要设置标记位）

// 注意：由于 max_commands_per_frame 等成员是 public 的，
// Godot 的 ADD_PROPERTY 宏需要 setter/getter 方法。
// 我们需要手动添加这些方法。
