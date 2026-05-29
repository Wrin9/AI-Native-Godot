/**************************************************************************/
/*  mcp_http_server.cpp                                                   */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPHTTPServer 实现 — HTTP JSON-RPC 服务器                              */
/* 使用 TCPServer + StreamPeerTCP 实现 HTTP 传输层                        */
/* 支持 JSON-RPC 2.0 协议（兼容 MCP）                                    */
/**************************************************************************/

#include "mcp_http_server.h"

#include "core/os/os.h"
#include "core/io/json.h"

// ============================================================
// 构造 / 析构
// ============================================================

MCPHTTPServer::MCPHTTPServer() {}

MCPHTTPServer::~MCPHTTPServer() { stop(); }

// ============================================================
// 服务器生命周期
// ============================================================

Error MCPHTTPServer::start(int p_port) {
	if (_is_running) {
		WARN_PRINT("MCPHTTPServer: 服务器已在运行");
		return OK;
	}

	_port = p_port;
	if (_server.is_null()) {
		_server.instantiate();
	}

	Error err = _server->listen(p_port);
	if (err != OK) {
		_server.unref();
		WARN_PRINT(vformat("MCPHTTPServer: 无法在端口 %d 启动（错误码 %d）", p_port, err));
		return err;
	}

	_is_running = true;
	print_line(vformat("MCPHTTPServer: HTTP MCP 服务器已在端口 %d 启动", p_port));
	return OK;
}

void MCPHTTPServer::stop() {
	if (!_is_running) return;

	// 断开所有客户端
	for (const KeyValue<int, ClientInfo> &E : _clients) {
		if (E.value.connection.is_valid()) {
			E.value.connection->disconnect_from_host();
		}
	}

	_clients.clear();
	_connected_ids.clear();

	if (_server.is_valid()) {
		_server->stop();
		_server.unref();
	}
	_is_running = false;
}

void MCPHTTPServer::poll() {
	if (!_is_running || _server.is_null()) return;

	_accept_new_connections();
	_poll_clients();
}

bool MCPHTTPServer::is_running() const { return _is_running; }
int MCPHTTPServer::get_port() const { return _port; }

// ============================================================
// 消息发送
// ============================================================

void MCPHTTPServer::broadcast(const String &p_type, const Dictionary &p_data) {
	if (!_is_running) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_type;
	message["params"] = p_data;

	for (int i = 0; i < _connected_ids.size(); i++) {
		_send_json_response(_connected_ids[i], message);
	}
}

void MCPHTTPServer::send_to(int p_client_id, const String &p_type, const Dictionary &p_data) {
	if (!_is_running) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_type;
	message["params"] = p_data;

	_send_json_response(p_client_id, message);
}

void MCPHTTPServer::send_response(int p_client_id, const Variant &p_request_id, const Dictionary &p_result) {
	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["id"] = p_request_id;
	message["result"] = p_result;

	_send_json_response(p_client_id, message);
}

void MCPHTTPServer::send_error_response(int p_client_id, const Variant &p_request_id, int p_code, const String &p_message) {
	Dictionary error_obj;
	error_obj["code"] = p_code;
	error_obj["message"] = p_message;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["id"] = p_request_id;
	message["error"] = error_obj;

	_send_json_response(p_client_id, message);
}

void MCPHTTPServer::send_notification(int p_client_id, const String &p_method, const Dictionary &p_params) {
	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_method;
	if (!p_params.is_empty()) {
		message["params"] = p_params;
	}

	_send_json_response(p_client_id, message);
}

// ============================================================
// 客户端管理
// ============================================================

int MCPHTTPServer::get_client_count() const { return _connected_ids.size(); }

Array MCPHTTPServer::get_clients_info() const {
	Array result;
	for (int i = 0; i < _connected_ids.size(); i++) {
		result.push_back(get_client_info(_connected_ids[i]));
	}
	return result;
}

void MCPHTTPServer::disconnect_client(int p_client_id) {
	if (_clients.has(p_client_id)) {
		if (_clients[p_client_id].connection.is_valid()) {
			_clients[p_client_id].connection->disconnect_from_host();
		}
		_clients[p_client_id].state = CLIENT_DISCONNECTED;
		_clients.erase(p_client_id);
		_connected_ids.remove_at(_connected_ids.find(p_client_id));
	}
}

Dictionary MCPHTTPServer::get_client_info(int p_client_id) const {
	Dictionary info;
	if (!_clients.has(p_client_id)) {
		info["found"] = false;
		return info;
	}

	const ClientInfo &client = _clients[p_client_id];
	info["found"] = true;
	info["client_id"] = p_client_id;
	info["state"] = client.state;
	info["name"] = client.name;
	info["client_type"] = client.client_type;
	info["connected_at_msec"] = (int64_t)client.connected_at_msec;
	info["messages_received"] = client.messages_received;
	info["messages_sent"] = client.messages_sent;
	return info;
}

void MCPHTTPServer::set_max_clients(int p_max) { _max_clients = MAX(p_max, 1); }
int MCPHTTPServer::get_max_clients() const { return _max_clients; }
void MCPHTTPServer::set_message_handler(const Callable &p_handler) { _message_handler = p_handler; }

// ============================================================
// 内部方法
// ============================================================

void MCPHTTPServer::_accept_new_connections() {
	if (_server.is_null()) return;

	while (_server->is_connection_available()) {
		if ((int)_clients.size() >= _max_clients) {
			// 超过最大客户端数，接受后立即关闭
			Ref<StreamPeerTCP> conn = _server->take_connection();
			if (conn.is_valid()) {
				conn->disconnect_from_host();
			}
			return;
		}

		Ref<StreamPeerTCP> conn = _server->take_connection();
		if (conn.is_null()) {
			break;
		}

		int client_id = _next_client_id++;
		ClientInfo info;
		info.connection = conn;
		info.state = CLIENT_CONNECTED;
		info.connected_at_msec = OS::get_singleton()->get_ticks_msec();
		info.client_type = "unknown";
		_clients[client_id] = info;
		_connected_ids.push_back(client_id);

		emit_signal("client_connected", client_id);
	}
}

void MCPHTTPServer::_poll_clients() {
	Vector<int> to_remove;

	for (int i = 0; i < _connected_ids.size(); i++) {
		int cid = _connected_ids[i];
		if (!_clients.has(cid)) continue;

		ClientInfo &client = _clients[cid];
		if (client.connection.is_null() || client.connection->get_status() != StreamPeerSocket::STATUS_CONNECTED) {
			to_remove.push_back(cid);
			continue;
		}

		// 读取可用数据
		int available = client.connection->get_available_bytes();
		if (available > 0) {
			PackedByteArray data;
			data.resize(available);
			Error err = client.connection->get_data(data.ptrw(), available);
			if (err == OK && data.size() > 0) {
				String chunk = String::utf8((const char *)data.ptr(), data.size());
				client.recv_buffer += chunk;
			}
		}

		// 尝试解析完整的 HTTP 请求
		int header_end = client.recv_buffer.find("\r\n\r\n");
		if (header_end >= 0) {
			// 解析 Content-Length
			int content_length = 0;
			String header_section = client.recv_buffer.substr(0, header_end);
			int cl_pos = header_section.find("Content-Length:");
			if (cl_pos < 0) {
				cl_pos = header_section.find("content-length:");
			}
			if (cl_pos >= 0) {
				String cl_line = header_section.substr(cl_pos);
				// 最后一行可能不含换行（header_end 在 \r\n\r\n 处截断）
				int line_end = cl_line.find("\r\n");
				if (line_end < 0) { line_end = cl_line.find("\n"); }
				if (line_end >= 0) { cl_line = cl_line.substr(0, line_end); }
				// cl_line 现在是 "Content-Length: 166"
				int colon_pos = cl_line.find(":");
				if (colon_pos >= 0) {
					content_length = cl_line.substr(colon_pos + 1).strip_edges().to_int();
				}
			}

			int body_start = header_end + 4;
			int total_request_length = body_start + content_length;


			if ((int)client.recv_buffer.length() >= total_request_length) {
				// 完整请求已到达
				String body = client.recv_buffer.substr(body_start, content_length);
				client.recv_buffer = client.recv_buffer.substr(total_request_length);
				client.messages_received++;

				_handle_request(cid, body);
			}
		}

		// 防止缓冲区无限增长
		if (client.recv_buffer.length() > 1048576) {
			WARN_PRINT(vformat("MCPHTTPServer: 客户端 %d 缓冲区溢出，断开连接", cid));
			to_remove.push_back(cid);
		}
	}

	// 清理断开的客户端
	for (int i = 0; i < to_remove.size(); i++) {
		int cid = to_remove[i];
		if (_clients.has(cid)) {
			_clients[cid].state = CLIENT_DISCONNECTED;
			_clients.erase(cid);
		}
		int idx = _connected_ids.find(cid);
		if (idx >= 0) {
			_connected_ids.remove_at(idx);
		}
		emit_signal("client_disconnected", cid);
	}
}

void MCPHTTPServer::_handle_request(int p_client_id, const String &p_body) {
	if (p_body.strip_edges().is_empty()) {
		// 空 body，可能是简单的 GET 请求
		Dictionary result;
		result["status"] = "ok";
		result["server"] = "MCP HTTP Server";
		_send_http_response(p_client_id, 200, "application/json", _serialize_message(result));
		return;
	}

	Dictionary message;
	if (_parse_json_message(p_body, message)) {
		_handle_message(p_client_id, message);
	} else {
		send_error_response(p_client_id, Variant(), -32700, "Parse error: invalid JSON");
	}
}

void MCPHTTPServer::_handle_message(int p_client_id, const Dictionary &p_message) {
	String jsonrpc = p_message.get("jsonrpc", "");
	if (jsonrpc != "2.0") {
		send_error_response(p_client_id, p_message.get("id", Variant()), -32600, "Invalid Request: jsonrpc must be '2.0'");
		return;
	}

	String method = p_message.get("method", "");
	bool has_id = p_message.has("id");

	// 通知消息（无 id）
	if (!has_id && !method.is_empty()) {
		if (method == "notifications/initialized") {
			if (_clients.has(p_client_id)) {
				_clients[p_client_id].state = CLIENT_AUTHENTICATED;
			}
			emit_signal("client_initialized", p_client_id);
			return;
		}

		if (method == "notifications/cancelled") {
			Dictionary params = p_message.get("params", Dictionary());
			int req_id = params.get("requestId", -1);
			emit_signal("request_cancelled", p_client_id, req_id);
			return;
		}

		if (_message_handler.is_valid()) {
			_message_handler.call(p_client_id, p_message);
		}
		return;
	}

	// 请求消息（有 id）
	if (method.is_empty()) {
		send_error_response(p_client_id, p_message.get("id", Variant()), -32600, "Invalid Request: method is required");
		return;
	}

	Dictionary params = p_message.get("params", Dictionary());

	// MCP 协议握手 - initialize
	if (method == "initialize") {
		Dictionary capabilities;
		capabilities["tools"] = Dictionary();
		capabilities["resources"] = Dictionary();
		capabilities["prompts"] = Dictionary();

		Dictionary server_info;
		server_info["name"] = "AI-Native Godot MCP Server";
		server_info["version"] = "0.1.0";

		Dictionary result;
		result["protocolVersion"] = "2025-11-25";
		result["serverInfo"] = server_info;
		result["capabilities"] = capabilities;

		send_response(p_client_id, p_message["id"], result);

		if (_clients.has(p_client_id)) {
			_clients[p_client_id].state = CLIENT_AUTHENTICATED;
			Dictionary client_info_dict = params.get("clientInfo", Dictionary());
			String client_name = client_info_dict.get("name", "");
			if (client_name.find("bridge") >= 0 || client_name.find("Bridge") >= 0) {
				_clients[p_client_id].client_type = "ai_bridge";
			} else if (client_name.find("debug") >= 0 || client_name.find("Debug") >= 0) {
				_clients[p_client_id].client_type = "debug_tool";
			}
			_clients[p_client_id].name = client_name;
		}
		return;
	}

	// 转发给外部处理器
	if (_message_handler.is_valid()) {
		_message_handler.call(p_client_id, p_message);
	} else {
		send_error_response(p_client_id, p_message["id"], -32601, vformat("Method not found: %s", method));
	}
}

void MCPHTTPServer::_send_http_response(int p_client_id, int p_status_code, const String &p_content_type, const String &p_body) {
	if (!_clients.has(p_client_id)) return;

	ClientInfo &client = _clients[p_client_id];
	if (client.connection.is_null() || client.connection->get_status() != StreamPeerSocket::STATUS_CONNECTED) {
		return;
	}

	String status_text;
	switch (p_status_code) {
		case 200: status_text = "OK"; break;
		case 400: status_text = "Bad Request"; break;
		case 404: status_text = "Not Found"; break;
		case 500: status_text = "Internal Server Error"; break;
		default: status_text = "OK"; break;
	}

			// Build response as UTF-8, precise Content-Length
	CharString body_utf8 = p_body.utf8();
	int body_len = body_utf8.length();

	String header;
	header += vformat("HTTP/1.1 %d %s\r\n", p_status_code, status_text);
	header += vformat("Content-Type: %s\r\n", p_content_type);
	header += vformat("Content-Length: %d\r\n", body_len);
	header += "Access-Control-Allow-Origin: *\r\n";
	header += "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
	header += "Access-Control-Allow-Headers: Content-Type\r\n";
	header += "Connection: close\r\n";
	header += "\r\n";

	// Send header and body as separate writes from pre-converted UTF-8
	CharString header_utf8 = header.utf8();
	client.connection->put_data((const uint8_t *)header_utf8.ptr(), header_utf8.length());
	client.connection->put_data((const uint8_t *)body_utf8.ptr(), body_utf8.length());
	client.messages_sent++;
	// Flush and close connection (Connection: close)
	client.connection->poll();
	client.connection->disconnect_from_host();
}

void MCPHTTPServer::_send_json_response(int p_client_id, const Dictionary &p_message) {
	String json = _serialize_message(p_message);
	_send_http_response(p_client_id, 200, "application/json", json);
}

bool MCPHTTPServer::_parse_json_message(const String &p_json, Dictionary &r_out) const {
	if (p_json.strip_edges().is_empty()) return false;

	// Use JSON::parse_string (Godot 4.6 static API)
	Variant parsed = JSON::parse_string(p_json);
	if (parsed.get_type() != Variant::DICTIONARY) return false;

	r_out = parsed;
	return true;
}

String MCPHTTPServer::_serialize_message(const Dictionary &p_message) const {
	return JSON::stringify(p_message, "\t", false);
}

bool MCPHTTPServer::_parse_http_request(const String &p_raw, String &r_method, String &r_path, String &r_body) const {
	int header_end = p_raw.find("\r\n\r\n");
	if (header_end < 0) return false;

	String header_section = p_raw.substr(0, header_end);
	r_body = p_raw.substr(header_end + 4);

	// 解析第一行
	int first_line_end = header_section.find("\n");
	if (first_line_end < 0) return false;

	String first_line = header_section.substr(0, first_line_end);
	Vector<String> parts = first_line.split(" ");
	if (parts.size() < 2) return false;

	r_method = parts[0];
	r_path = parts[1];
	return true;
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPHTTPServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start", "port"), &MCPHTTPServer::start, DEFVAL(9877));
	ClassDB::bind_method(D_METHOD("stop"), &MCPHTTPServer::stop);
	ClassDB::bind_method(D_METHOD("poll"), &MCPHTTPServer::poll);
	ClassDB::bind_method(D_METHOD("is_running"), &MCPHTTPServer::is_running);
	ClassDB::bind_method(D_METHOD("get_port"), &MCPHTTPServer::get_port);

	ClassDB::bind_method(D_METHOD("broadcast", "type", "data"), &MCPHTTPServer::broadcast);
	ClassDB::bind_method(D_METHOD("send_to", "client_id", "type", "data"), &MCPHTTPServer::send_to);
	ClassDB::bind_method(D_METHOD("send_response", "client_id", "request_id", "result"), &MCPHTTPServer::send_response);
	ClassDB::bind_method(D_METHOD("send_error_response", "client_id", "request_id", "code", "message"), &MCPHTTPServer::send_error_response);
	ClassDB::bind_method(D_METHOD("send_notification", "client_id", "method", "params"), &MCPHTTPServer::send_notification, DEFVAL(Dictionary()));

	ClassDB::bind_method(D_METHOD("get_client_count"), &MCPHTTPServer::get_client_count);
	ClassDB::bind_method(D_METHOD("get_clients_info"), &MCPHTTPServer::get_clients_info);
	ClassDB::bind_method(D_METHOD("disconnect_client", "client_id"), &MCPHTTPServer::disconnect_client);
	ClassDB::bind_method(D_METHOD("get_client_info", "client_id"), &MCPHTTPServer::get_client_info);

	ClassDB::bind_method(D_METHOD("set_max_clients", "max"), &MCPHTTPServer::set_max_clients);
	ClassDB::bind_method(D_METHOD("get_max_clients"), &MCPHTTPServer::get_max_clients);
	ClassDB::bind_method(D_METHOD("set_message_handler", "handler"), &MCPHTTPServer::set_message_handler);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_clients", PROPERTY_HINT_RANGE, "1,16,1"), "set_max_clients", "get_max_clients");

	ADD_SIGNAL(MethodInfo("client_connected", PropertyInfo(Variant::INT, "client_id")));
	ADD_SIGNAL(MethodInfo("client_disconnected", PropertyInfo(Variant::INT, "client_id")));
	ADD_SIGNAL(MethodInfo("client_initialized", PropertyInfo(Variant::INT, "client_id")));
	ADD_SIGNAL(MethodInfo("request_cancelled", PropertyInfo(Variant::INT, "client_id"), PropertyInfo(Variant::INT, "request_id")));
	ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::INT, "client_id"), PropertyInfo(Variant::DICTIONARY, "message")));
}
