/**************************************************************************/
/*  mcp_sandbox.cpp                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPSandbox 实现 — 安全执行 AI 生成代码。                                */
/**************************************************************************/

#include "core/mcp_sandbox.h"

#include "core/mcp_command_queue.h"

void MCPSandbox::_bind_methods() {
	ClassDB::bind_method(D_METHOD("execute", "code", "timeout_sec"), &MCPSandbox::execute, DEFVAL(5.0));
	ClassDB::bind_method(D_METHOD("is_executing"), &MCPSandbox::is_executing);
	ClassDB::bind_method(D_METHOD("set_allowed_operations", "ops"), &MCPSandbox::set_allowed_operations);
	ClassDB::bind_method(D_METHOD("get_execution_log"), &MCPSandbox::get_execution_log);
	ClassDB::bind_method(D_METHOD("clear_log"), &MCPSandbox::clear_log);
}

Dictionary MCPSandbox::execute(const String &p_code, double p_timeout_sec) {
	Dictionary result;

	if (_executing) {
		result["success"] = false;
		result["error"] = "沙箱正在执行其他代码，请等待完成";
		return result;
	}

	_executing = true;
	_log_entry("start", vformat("开始执行，超时 %.1f 秒", p_timeout_sec));

	// TODO: 实际执行逻辑
	// 1. 解析代码 AST，检查是否访问禁止的 API
	// 2. 在受限环境中执行
	// 3. 超时自动终止
	result["success"] = true;
	result["output"] = "";

	_executing = false;
	_log_entry("end", "执行完成");
	return result;
}

void MCPSandbox::set_allowed_operations(const PackedStringArray &p_ops) {
	_allowed_ops = p_ops;
}

bool MCPSandbox::_is_operation_allowed(const String &p_op) const {
	if (_allowed_ops.is_empty()) {
		return true; // 未设置白名单则全部允许
	}
	return _allowed_ops.has(p_op);
}

Array MCPSandbox::get_execution_log() const {
	return _log;
}

void MCPSandbox::clear_log() {
	_log.clear();
}

void MCPSandbox::_log_entry(const String &p_type, const String &p_message) {
	Dictionary entry;
	entry["type"] = p_type;
	entry["message"] = p_message;
	entry["timestamp"] = Time::get_singleton()->get_ticks_msec();
	_log.append(entry);
}
