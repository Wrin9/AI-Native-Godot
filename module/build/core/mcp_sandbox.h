/**************************************************************************/
/*  mcp_sandbox.h                                                         */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPSandbox — 安全执行沙箱                                              */
/*                                                                        */
/* 在受限环境中执行 AI 生成的代码/命令，防止意外修改引擎关键状态。          */
/* 支持超时控制、权限限制、执行日志。                                      */
/**************************************************************************/

#ifndef MCP_SANDBOX_H
#define MCP_SANDBOX_H

#include "core/object/object.h"
#include "core/variant/dictionary.h"

// 前向声明
class MCPCommandQueue;

class MCPSandbox : public Object {
	GDCLASS(MCPSandbox, Object);

public:
	/** 设置命令队列依赖 */
	void set_command_queue(MCPCommandQueue *p_queue) { _queue = p_queue; }

	/** 在沙箱中执行一段脚本 */
	Dictionary execute(const String &p_code, double p_timeout_sec = 5.0);

	/** 检查当前是否正在执行 */
	bool is_executing() const { return _executing; }

	/** 设置允许的操作列表 */
	void set_allowed_operations(const PackedStringArray &p_ops);

	/** 获取执行日志 */
	Array get_execution_log() const;

	/** 清除日志 */
	void clear_log();

	MCPSandbox() = default;
	~MCPSandbox() = default;

protected:
	static void _bind_methods();

private:
	MCPCommandQueue *_queue = nullptr;
	bool _executing = false;
	PackedStringArray _allowed_ops;
	Array _log;

	/** 检查操作是否被允许 */
	bool _is_operation_allowed(const String &p_op) const;

	/** 记录执行日志 */
	void _log_entry(const String &p_type, const String &p_message);
};

#endif // MCP_SANDBOX_H
