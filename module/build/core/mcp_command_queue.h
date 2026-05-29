/**************************************************************************/
/*  mcp_command_queue.h                                                   */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPCommandQueue — 命令队列                                              */
/*                                                                        */
/* 接收来自 WebSocket 的 JSON-RPC 请求，解析后入队，                       */
/* 在主线程帧回调中依次处理。确保所有编辑器操作在主线程执行。              */
/**************************************************************************/

#ifndef MCP_COMMAND_QUEUE_H
#define MCP_COMMAND_QUEUE_H

#include "core/object/object.h"
#include "core/variant/dictionary.h"

class MCPCommandQueue : public Object {
	GDCLASS(MCPCommandQueue, Object);

public:
	/** 入队一条命令 */
	void enqueue(const Dictionary &p_command);

	/** 处理队列中的所有命令（由 MCPEditorPlugin::_process 调用） */
	void process_queue();

	/** 当前队列长度 */
	int get_pending_count() const;

	/** 清空队列 */
	void clear();

	MCPCommandQueue() = default;
	~MCPCommandQueue() = default;

protected:
	static void _bind_methods();

private:
	/** 命令条目 */
	struct Command {
		String method; // JSON-RPC 方法名
		Dictionary params; // 请求参数
		int id = 0; // 请求 ID（用于响应匹配）
	};

	List<Command> _queue;

	/** 处理单条命令 */
	void _process_one(const Command &p_cmd);
};

#endif // MCP_COMMAND_QUEUE_H
