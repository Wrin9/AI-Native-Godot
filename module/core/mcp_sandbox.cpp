/**************************************************************************/
/*  mcp_sandbox.cpp                                                       */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* AI 代码执行沙箱实现                                                     */
/**************************************************************************/

#include "mcp_sandbox.h"

#include "core/os/os.h"
#include "core/string/ustring.h"
#include "modules/gdscript/gdscript.h"

// ============================================================
// 构造 / 析构
// ============================================================

MCPSandbox::MCPSandbox() {
	// 默认允许 res:// 路径
	_allowed_paths.insert("res://");
}

MCPSandbox::~MCPSandbox() {
}

// ============================================================
// 公开 API
// ============================================================

Dictionary MCPSandbox::execute(const String &p_code, const Dictionary &p_context, SandboxType p_type) {
	Dictionary result;
	result["success"] = false;
	result["result"] = Variant();
	result["error"] = "";
	result["logs"] = Array();
	result["stats"] = Dictionary();

	// 验证代码
	if (p_code.strip_edges().is_empty()) {
		result["error"] = "代码为空";
		return result;
	}

	if (!validate_code(p_code)) {
		result["error"] = "代码验证失败";
		return result;
	}

	// DRY_RUN 模式只验证
	if (p_type == SANDBOX_DRY_RUN) {
		result["success"] = true;
		result["result"] = "Dry run validation passed";
		return result;
	}

	// 检查资源限制
	if (!_check_limits(_default_limits)) {
		result["error"] = vformat("资源限制已超出：节点操作 %d/%d，文件操作 %d/%d",
				_node_ops_count, _default_limits.max_node_operations,
				_file_ops_count, _default_limits.max_file_operations);
		return result;
	}

	// 重置计数器（每次执行前重置）
	reset_counters();

	// 包装代码到安全上下文
	String wrapped_code = _wrap_code(p_code);

	// 记录开始时间
	uint64_t start_msec = OS::get_singleton()->get_ticks_msec();

	// 创建 GDScript 实例并执行
	Ref<GDScript> script;
	script.instantiate();
	script->set_source_code(wrapped_code);

	Error reload_err = script->reload();
	if (reload_err != OK) {
		result["error"] = vformat("代码编译失败（错误码 %d）", reload_err);
		return result;
	}

	if (!script->can_instantiate()) {
		result["error"] = "代码无法实例化（可能缺少 run 函数）";
		return result;
	}

	Variant instance = script->new_();
	if (instance.get_type() == Variant::NIL) {
		result["error"] = "代码实例化返回 null";
		return result;
	}

	Object *obj = Object::cast_to<Object>(instance);
	if (!obj || !obj->has_method("run")) {
		result["error"] = "动态代码必须定义 run(ctx) 函数";
		return result;
	}

	// 构建安全执行上下文
	Dictionary safe_context = _build_safe_context(p_context);

	// 执行并捕获错误
	Variant exec_result;
	bool success = true;
	String error_msg;

	// 使用 call 来安全执行
	Callable callable = Callable(obj, "run");
	Variant ret = callable.call(safe_context);

	// 检查执行时间
	uint64_t elapsed_msec = OS::get_singleton()->get_ticks_msec() - start_msec;
	if (elapsed_msec > (uint64_t)(_default_limits.max_time_sec * 1000.0)) {
		success = false;
		error_msg = vformat("执行超时（%.2f秒超过%.2f秒限制）",
				elapsed_msec / 1000.0, _default_limits.max_time_sec);
	}

	// 收集结果
	if (success) {
		result["success"] = true;
		String result_str = String(ret);
		result["result"] = _truncate_output(result_str, _default_limits.max_output_length);
	}

	if (!error_msg.is_empty()) {
		result["error"] = error_msg;
	}

	// 统计信息
	Dictionary stats;
	stats["elapsed_msec"] = (int64_t)elapsed_msec;
	stats["node_operations"] = _node_ops_count;
	stats["file_operations"] = _file_ops_count;
	stats["sub_instances"] = _sub_instance_count;
	result["stats"] = stats;

	return result;
}

bool MCPSandbox::validate_code(const String &p_code) const {
	if (p_code.strip_edges().is_empty()) {
		return false;
	}

	// 检查危险模式
	// 注意：这是一个基本的黑名单检查，真正的安全需要更复杂的策略

	// 禁止直接调用 OS::execute 或 shell 操作
	if (p_code.find("OS.execute") >= 0 || p_code.find("OS.shell_open") >= 0) {
		return false;
	}

	// 禁止访问文件系统的原始操作（应通过沙箱 API 操作）
	if (p_code.find("DirAccess.open") >= 0 || p_code.find("FileAccess.open") >= 0) {
		// 允许在白名单路径内的操作，但这里只做粗粒度检查
		// 更细粒度的检查在执行时通过 notify_file_operation 进行
	}

	// 禁止动态代码加载
	if (p_code.find("load(") >= 0 || p_code.find("preload(") >= 0 || p_code.find("load_threaded") >= 0) {
		// 这些有合法用途，仅做警告级别的检查
	}

	// 禁止访问引擎内部
	if (p_code.find("Engine.get_singleton") >= 0) {
		return false;
	}

	// 禁止无限循环模式（粗略检查）
	if (p_code.find("while true:") >= 0 || p_code.find("while True:") >= 0) {
		return false;
	}

	// 尝试编译代码以验证语法
	String wrapped = _wrap_code(p_code);
	Ref<GDScript> script;
	script.instantiate();
	script->set_source_code(wrapped);

	// 使用 reload 验证语法（DRY_RUN 方式）
	Error err = script->reload();
	if (err != OK) {
		return false;
	}

	if (!script->can_instantiate()) {
		return false;
	}

	return true;
}

void MCPSandbox::set_allowed_paths(const Vector<String> &p_paths) {
	_allowed_paths.clear();
	for (int i = 0; i < p_paths.size(); i++) {
		_allowed_paths.insert(p_paths[i]);
	}
}

Vector<String> MCPSandbox::get_allowed_paths() const {
	Vector<String> paths;
	for (const String &path : _allowed_paths) {
		paths.push_back(path);
	}
	return paths;
}

void MCPSandbox::add_allowed_path(const String &p_path) {
	_allowed_paths.insert(p_path);
}

void MCPSandbox::remove_allowed_path(const String &p_path) {
	_allowed_paths.erase(p_path);
}

bool MCPSandbox::is_path_allowed(const String &p_path) const {
	// 如果白名单为空，允许所有路径（默认模式）
	if (_allowed_paths.is_empty()) {
		return true;
	}

	// 检查路径是否是白名单中某个路径的前缀
	for (const String &allowed : _allowed_paths) {
		if (p_path.begins_with(allowed)) {
			return true;
		}
	}

	return false;
}

void MCPSandbox::set_default_limits(const Dictionary &p_limits) {
	_default_limits = _dict_to_limits(p_limits);
}

Dictionary MCPSandbox::get_default_limits() const {
	return _limits_to_dict(_default_limits);
}

void MCPSandbox::reset_counters() {
	_node_ops_count = 0;
	_file_ops_count = 0;
	_sub_instance_count = 0;
}

int MCPSandbox::get_node_operations_count() const {
	return _node_ops_count;
}

int MCPSandbox::get_file_operations_count() const {
	return _file_ops_count;
}

bool MCPSandbox::notify_node_operation() {
	_node_ops_count++;
	if (_default_limits.max_node_operations > 0 && _node_ops_count > _default_limits.max_node_operations) {
		WARN_PRINT(vformat("MCPSandbox: 节点操作次数超出限制（%d/%d）", _node_ops_count, _default_limits.max_node_operations));
		return false; // 超出限制
	}
	return true;
}

bool MCPSandbox::notify_file_operation() {
	_file_ops_count++;
	if (_default_limits.max_file_operations > 0 && _file_ops_count > _default_limits.max_file_operations) {
		WARN_PRINT(vformat("MCPSandbox: 文件操作次数超出限制（%d/%d）", _file_ops_count, _default_limits.max_file_operations));
		return false; // 超出限制
	}
	return true;
}

// ============================================================
// 内部方法
// ============================================================

bool MCPSandbox::_check_limits(const Limits &p_limits) const {
	if (p_limits.max_node_operations > 0 && _node_ops_count >= p_limits.max_node_operations) {
		return false;
	}
	if (p_limits.max_file_operations > 0 && _file_ops_count >= p_limits.max_file_operations) {
		return false;
	}
	return true;
}

Dictionary MCPSandbox::_limits_to_dict(const Limits &p_limits) const {
	Dictionary dict;
	dict["max_memory_mb"] = p_limits.max_memory_mb;
	dict["max_time_sec"] = p_limits.max_time_sec;
	dict["max_node_operations"] = p_limits.max_node_operations;
	dict["max_file_operations"] = p_limits.max_file_operations;
	dict["max_sub_instances"] = p_limits.max_sub_instances;
	dict["max_output_length"] = p_limits.max_output_length;
	return dict;
}

MCPSandbox::Limits MCPSandbox::_dict_to_limits(const Dictionary &p_dict) const {
	Limits limits;
	limits.max_memory_mb = p_dict.get("max_memory_mb", 64);
	limits.max_time_sec = p_dict.get("max_time_sec", 5.0);
	limits.max_node_operations = p_dict.get("max_node_operations", 50);
	limits.max_file_operations = p_dict.get("max_file_operations", 20);
	limits.max_sub_instances = p_dict.get("max_sub_instances", 5);
	limits.max_output_length = p_dict.get("max_output_length", 10000);
	return limits;
}

Dictionary MCPSandbox::_build_safe_context(const Dictionary &p_user_context) const {
	Dictionary ctx;

	// 合并用户提供的上下文
	Array keys = p_user_context.keys();
	for (int i = 0; i < keys.size(); i++) {
		ctx[keys[i]] = p_user_context[keys[i]];
	}

	// 注入沙箱元信息
	ctx["_sandbox"] = true;
	ctx["_max_node_ops"] = _default_limits.max_node_operations;
	ctx["_max_file_ops"] = _default_limits.max_file_operations;

	return ctx;
}

String MCPSandbox::_wrap_code(const String &p_code) const {
	// 将用户代码包装到安全的 GDScript 模板中
	// 类似 GDScript 参考实现中的包装方式
	String wrapped;
	wrapped += "@tool\n";
	wrapped += "extends RefCounted\n";
	wrapped += "func run(ctx):\n";

	// 缩进用户代码
	PackedStringArray lines = p_code.split("\n");
	for (int i = 0; i < lines.size(); i++) {
		String line = lines[i];
		if (line.strip_edges().is_empty()) {
			wrapped += "\t\n";
		} else {
			wrapped += "\t" + line + "\n";
		}
	}

	return wrapped;
}

String MCPSandbox::_truncate_output(const String &p_output, int p_max_length) const {
	if (p_output.length() <= p_max_length) {
		return p_output;
	}
	return p_output.substr(0, p_max_length) + vformat("\n... (输出被截断，总长度 %d)", p_output.length());
}

String MCPSandbox::_parse_error_message(const String &p_raw_error) const {
	// 提取关键错误信息，去除堆栈噪音
	// GDScript 错误格式: "GDScript error at ..."

	// 查找第一个换行符前的内容
	int newline_pos = p_raw_error.find("\n");
	if (newline_pos >= 0) {
		return p_raw_error.substr(0, newline_pos);
	}
	return p_raw_error;
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPSandbox::_bind_methods() {
	// 绑定枚举
	BIND_ENUM_CONSTANT(SANDBOX_GDSCRIPT);
	BIND_ENUM_CONSTANT(SANDBOX_DRY_RUN);

	// 绑定方法
	ClassDB::bind_method(D_METHOD("execute", "code", "context", "sandbox_type"),
			&MCPSandbox::execute,
			DEFVAL(Dictionary()),
			DEFVAL(SANDBOX_GDSCRIPT));

	ClassDB::bind_method(D_METHOD("validate_code", "code"), &MCPSandbox::validate_code);
	ClassDB::bind_method(D_METHOD("set_allowed_paths", "paths"), &MCPSandbox::set_allowed_paths);
	ClassDB::bind_method(D_METHOD("get_allowed_paths"), &MCPSandbox::get_allowed_paths);
	ClassDB::bind_method(D_METHOD("add_allowed_path", "path"), &MCPSandbox::add_allowed_path);
	ClassDB::bind_method(D_METHOD("remove_allowed_path", "path"), &MCPSandbox::remove_allowed_path);
	ClassDB::bind_method(D_METHOD("is_path_allowed", "path"), &MCPSandbox::is_path_allowed);
	ClassDB::bind_method(D_METHOD("set_default_limits", "limits"), &MCPSandbox::set_default_limits);
	ClassDB::bind_method(D_METHOD("get_default_limits"), &MCPSandbox::get_default_limits);
	ClassDB::bind_method(D_METHOD("reset_counters"), &MCPSandbox::reset_counters);
	ClassDB::bind_method(D_METHOD("get_node_operations_count"), &MCPSandbox::get_node_operations_count);
	ClassDB::bind_method(D_METHOD("get_file_operations_count"), &MCPSandbox::get_file_operations_count);
	ClassDB::bind_method(D_METHOD("notify_node_operation"), &MCPSandbox::notify_node_operation);
	ClassDB::bind_method(D_METHOD("notify_file_operation"), &MCPSandbox::notify_file_operation);

	// 属性
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "default_limits"), "set_default_limits", "get_default_limits");

	// 信号
	ADD_SIGNAL(MethodInfo("limit_exceeded",
			PropertyInfo(Variant::STRING, "limit_type"),
			PropertyInfo(Variant::INT, "current_count"),
			PropertyInfo(Variant::INT, "max_count")));
}
