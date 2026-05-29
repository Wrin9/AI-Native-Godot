/**************************************************************************/
/*  mcp_websocket_server.cpp                                              */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPWebSocketServer 实现 — 基于 Godot WebSocket 的 MCP 服务器。          */
/**************************************************************************/

#include "core/mcp_websocket_server.h"

#include "core/config/engine.h"
#include "core/io/json.h"

void MCPWebSocketServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start", "port"), &MCPWebSocketServer::start, DEFVAL(9877));
	ClassDB::bind_method(D_METHOD("stop"), &MCPWebSocketServer::stop);
	ClassDB::bind_method(D_METHOD("poll"), &MCPWebSocketServer::poll);
	ClassDB::bind_method(D_METHOD("is_running"), &MCPWebSocketServer::is_running);
	ClassDB::bind_method(D_METHOD("get_port"), &MCPWebSocketServer::get_port);
	ClassDB::bind_method(D_METHOD("broadcast", "message"), &MCPWebSocketServer::broadcast);
	ClassDB::bind_method(D_METHOD("send_to", "client_id", "message"), &MCPWebSocketServer::send_to);
	ClassDB::bind_method(D_METHOD("get_client_count"), &MCPWebSocketServer::get_client_count);
}

MCPWebSocketServer::~MCPWebSocketServer() {
	stop();
}

bool MCPWebSocketServer::start(int p_port) {
	if (_running) {
		WARN_PRINT("MCPWebSocketServer: 服务器已在运行");
		return true;
	}

	_port = p_port;

	// TODO: 使用 Godot 内置 WebSocketServer API 启动
	// _server_id = WebSocketServer::create();
	// _server_id->listen(_port);
	_running = true;

	print_line(vformat("MCPWebSocketServer: 启动监听端口 %d", _port));
	return true;
}

void MCPWebSocketServer::stop() {
	if (!_running) {
		return;
	}

	// TODO: 关闭 WebSocket 服务器
	_running = false;
	print_line("MCPWebSocketServer: 已停止");
}

void MCPWebSocketServer::poll() {
	if (!_running) {
		return;
	}

	// TODO: 轮询 WebSocket 事件
	// WebSocketServer::get_singleton()->poll();
	// 处理连接/断开/消息事件
}

void MCPWebSocketServer::broadcast(const String &p_message) {
	if (!_running) {
		return;
	}
	// TODO: 向所有已连接客户端广播
	print_line(vformat("MCPWebSocketServer: 广播消息 (%d 字节)", p_message.length()));
}

void MCPWebSocketServer::send_to(int p_client_id, const String &p_message) {
	if (!_running) {
		return;
	}
	// TODO: 向指定客户端发送
	print_line(vformat("MCPWebSocketServer: 发送给客户端 %d (%d 字节)", p_client_id, p_message.length()));
}

int MCPWebSocketServer::get_client_count() const {
	// TODO: 返回实际连接数
	return 0;
}

void MCPWebSocketServer::_on_client_connected(int p_client_id) {
	print_line(vformat("MCPWebSocketServer: 客户端 %d 已连接", p_client_id));
}

void MCPWebSocketServer::_on_client_disconnected(int p_client_id) {
	print_line(vformat("MCPWebSocketServer: 客户端 %d 已断开", p_client_id));
}

void MCPWebSocketServer::_on_message_received(int p_client_id, const PackedByteArray &p_data) {
	String json_str = String::utf8((const char *)p_data.ptr(), p_data.size());

	// 解析 JSON
	Ref<JSON> json;
	json.instantiate();
	Error err = json->parse(json_str);
	if (err != OK) {
		Dictionary error_resp;
		error_resp["jsonrpc"] = "2.0";
		error_resp["error"] = Dictionary{ { "code", -32700 }, { "message", "Parse error" } };
		send_to(p_client_id, JSON::stringify(error_resp));
		return;
	}

	Dictionary request = json->get_data();
	_handle_request(p_client_id, request);
}

void MCPWebSocketServer::_handle_request(int p_client_id, const Dictionary &p_request) {
	String method = p_request.get("method", "");
	int id = p_request.get("id", 0);
	Dictionary params = p_request.get("params", Dictionary());

	// TODO: 将请求转发到 MCPCommandQueue 或 ToolDispatcher
	print_line(vformat("MCPWebSocketServer: 收到请求 method=%s id=%d", method, id));
}
