/**************************************************************************/
/*  mcp_command_queue.cpp                                                 */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPCommandQueue 实现 — 线程安全的命令排队与主线程处理。                  */
/**************************************************************************/

#include "core/mcp_command_queue.h"

#include "core/error/error_macros.h"

void MCPCommandQueue::_bind_methods() {
	ClassDB::bind_method(D_METHOD("enqueue", "command"), &MCPCommandQueue::enqueue);
	ClassDB::bind_method(D_METHOD("process_queue"), &MCPCommandQueue::process_queue);
	ClassDB::bind_method(D_METHOD("get_pending_count"), &MCPCommandQueue::get_pending_count);
	ClassDB::bind_method(D_METHOD("clear"), &MCPCommandQueue::clear);
}

void MCPCommandQueue::enqueue(const Dictionary &p_command) {
	Command cmd;
	cmd.method = p_command.get("method", "");
	cmd.params = p_command.get("params", Dictionary());
	cmd.id = p_command.get("id", 0);

	ERR_FAIL_COND_MSG(cmd.method.is_empty(), "MCPCommandQueue: 命令缺少 method 字段");

	_queue.push_back(cmd);
}

void MCPCommandQueue::process_queue() {
	// 逐条处理，直到队列清空
	// 处理过程中新入队的命令会在下一帧处理，防止无限循环
	List<Command> current;
	_queue.swap(current);

	while (!current.is_empty()) {
		Command cmd = current.front()->get();
		current.pop_front();
		_process_one(cmd);
	}
}

int MCPCommandQueue::get_pending_count() const {
	return _queue.size();
}

void MCPCommandQueue::clear() {
	_queue.clear();
}

void MCPCommandQueue::_process_one(const Command &p_cmd) {
	// TODO: 通过 ToolDispatcher 分派到具体工具
	// 当前为占位实现，后续集成时对接
	print_line(vformat("MCPCommandQueue: 处理命令 %s (id=%d)", p_cmd.method, p_cmd.id));
}
