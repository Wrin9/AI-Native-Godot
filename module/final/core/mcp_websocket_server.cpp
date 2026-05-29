/**************************************************************************/
/*  mcp_websocket_server.cpp                                              */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* WebSocket 服务器实现 - 替代 HTTP 实现双向实时通信                        */
/* 支持多客户端连接、JSON-RPC 2.0（兼容 MCP 协议）                         */
/**************************************************************************/

#include "mcp_websocket_server.h"

#include "core/os/os.h"
#include "core/json.h"
#include "modules/websocket/websocket_server.h"
#include "modules/websocket/websocket_peer.h"

// ============================================================
// 构造 / 析构
// ============================================================

MCPWebSocketServer::MCPWebSocketServer() {}

MCPWebSocketServer::~MCPWebSocketServer() { stop(); }

// ============================================================
// 服务器生命周期
// ============================================================

Error MCPWebSocketServer::start(int p_port) {
	if (_is_running) {
		WARN_PRINT("MCPWebSocketServer: 服务器已在运行");
		return OK;
	}

	_port = p_port;
	if (!_server) {
		_server = memnew(WebSocketServer);
	}

	Dictionary supported_protocols;
	supported_protocols["mcp"] = true;

	Error err = _server->listen(p_port, supported_protocols);
	if (err != OK) {
		memdelete(_server);
		_server = nullptr;
		WARN_PRINT(vformat("MCPWebSocketServer: 无法在端口 %d 启动（错误码 %d）", p_port, err));
		return err;
	}

	_is_running = true;
	_last_ping_msec = OS::get_singleton()->get_ticks_msec();

	_server->connect("client_connected", callable_mp(this, &MCPWebSocketServer::_on_client_connected));
	_server->connect("client_disconnected", callable_mp(this, &MCPWebSocketServer::_on_client_disconnected));
	_server->connect("data_received", callable_mp(this, &MCPWebSocketServer::_on_data_received));

	return OK;
}

void MCPWebSocketServer::stop() {
	if (!_is_running) return;

	for (int i = 0; i < _connected_peers.size(); i++) {
		if (_server) {
			_server->disconnect_peer(_connected_peers[i], 1001, "Server shutting down");
		}
	}

	_connected_peers.clear();
	_clients.clear();

	if (_server) {
		_server->stop();
		memdelete(_server);
		_server = nullptr;
	}
	_is_running = false;
}

void MCPWebSocketServer::poll() {
	if (!_is_running || !_server) return;

	_server->poll();

	uint64_t now = OS::get_singleton()->get_ticks_msec();
	if (_ping_interval_msec > 0 && now - _last_ping_msec >= _ping_interval_msec) {
		_send_ping();
		_last_ping_msec = now;
	}
}

bool MCPWebSocketServer::is_running() const { return _is_running; }
int MCPWebSocketServer::get_port() const { return _port; }

// ============================================================
// 消息发送
// ============================================================

void MCPWebSocketServer::broadcast(const String &p_type, const Dictionary &p_data) {
	if (!_is_running || !_server) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_type;
	message["params"] = p_data;

	String json = _serialize_message(message);
	PackedByteArray data = json.utf8();

	for (int i = 0; i < _connected_peers.size(); i++) {
		WebSocketPeer *peer = _server->get_peer(_connected_peers[i]);
		if (peer) {
			peer->put_packet(data.ptr(), data.size());
			if (_clients.has(_connected_peers[i])) {
				_clients[_connected_peers[i]].messages_sent++;
			}
		}
	}
}

void MCPWebSocketServer::send_to(int p_peer_id, const String &p_type, const Dictionary &p_data) {
	if (!_is_running || !_server) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_type;
	message["params"] = p_data;

	String json = _serialize_message(message);
	PackedByteArray data = json.utf8();

	WebSocketPeer *peer = _server->get_peer(p_peer_id);
	if (peer) {
		peer->put_packet(data.ptr(), data.size());
		if (_clients.has(p_peer_id)) {
			_clients[p_peer_id].messages_sent++;
		}
	}
}

void MCPWebSocketServer::send_response(int p_peer_id, const Variant &p_request_id, const Dictionary &p_result) {
	if (!_is_running || !_server) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["id"] = p_request_id;
	message["result"] = p_result;

	String json = _serialize_message(message);
	PackedByteArray data = json.utf8();

	WebSocketPeer *peer = _server->get_peer(p_peer_id);
	if (peer) {
		peer->put_packet(data.ptr(), data.size());
		if (_clients.has(p_peer_id)) {
			_clients[p_peer_id].messages_sent++;
		}
	}
}

void MCPWebSocketServer::send_error_response(int p_peer_id, const Variant &p_request_id, int p_code, const String &p_message) {
	if (!_is_running || !_server) return;

	Dictionary error_obj;
	error_obj["code"] = p_code;
	error_obj["message"] = p_message;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["id"] = p_request_id;
	message["error"] = error_obj;

	String json = _serialize_message(message);
	PackedByteArray data = json.utf8();

	WebSocketPeer *peer = _server->get_peer(p_peer_id);
	if (peer) {
		peer->put_packet(data.ptr(), data.size());
		if (_clients.has(p_peer_id)) {
			_clients[p_peer_id].messages_sent++;
		}
	}
}

void MCPWebSocketServer::send_notification(int p_peer_id, const String &p_method, const Dictionary &p_params) {
	if (!_is_running || !_server) return;

	Dictionary message;
	message["jsonrpc"] = "2.0";
	message["method"] = p_method;
	if (!p_params.is_empty()) {
		message["params"] = p_params;
	}

	String json = _serialize_message(message);
	PackedByteArray data = json.utf8();

	WebSocketPeer *peer = _server->get_peer(p_peer_id);
	if (peer) {
		peer->put_packet(data.ptr(), data.size());
		if (_clients.has(p_peer_id)) {
			_clients[p_peer_id].messages_sent++;
		}
	}
}

// ============================================================
// 客户端管理
// ============================================================

int MCPWebSocketServer::get_client_count() const { return _connected_peers.size(); }

Array MCPWebSocketServer::get_clients_info() const {
	Array result;
	for (int i = 0; i < _connected_peers.size(); i++) {
		result.push_back(get_client_info(_connected_peers[i]));
	}
	return result;
}

void MCPWebSocketServer::disconnect_client(int p_peer_id, int p_code, const String &p_reason) {
	if (!_server) return;
	_server->disconnect_peer(p_peer_id, p_code, p_reason);
}

Dictionary MCPWebSocketServer::get_client_info(int p_peer_id) const {
	Dictionary info;
	if (!_clients.has(p_peer_id)) {
		info["found"] = false;
		return info;
	}

	const ClientInfo &client = _clients[p_peer_id];
	info["found"] = true;
	info["peer_id"] = client.peer_id;
	info["state"] = client.state;
	info["name"] = client.name;
	info["client_type"] = client.client_type;
	info["connected_at_msec"] = (int64_t)client.connected_at_msec;
	info["messages_received"] = client.messages_received;
	info["messages_sent"] = client.messages_sent;
	info["pending_request_count"] = client.pending_requests.size();
	return info;
}

void MCPWebSocketServer::set_max_clients(int p_max) { _max_clients = MAX(p_max, 1); }
int MCPWebSocketServer::get_max_clients() const { return _max_clients; }
void MCPWebSocketServer::set_ping_interval_msec(uint64_t p_msec) { _ping_interval_msec = p_msec; }
uint64_t MCPWebSocketServer::get_ping_interval_msec() const { return _ping_interval_msec; }
void MCPWebSocketServer::set_message_handler(const Callable &p_handler) { _message_handler = p_handler; }

// ============================================================
// 内部回调
// ============================================================

void MCPWebSocketServer::_on_client_connected(int p_peer_id) {
	if ((int)_connected_peers.size() >= _max_clients) {
		if (_server) {
			_server->disconnect_peer(p_peer_id, 1008, "Max clients reached");
		}
		return;
	}

	ClientInfo info;
	info.peer_id = p_peer_id;
	info.state = CLIENT_CONNECTED;
	info.connected_at_msec = OS::get_singleton()->get_ticks_msec();
	info.client_type = "unknown";
	_clients[p_peer_id] = info;
	_connected_peers.push_back(p_peer_id);

	emit_signal("client_connected", p_peer_id);
}

void MCPWebSocketServer::_on_client_disconnected(int p_peer_id, int p_code, const String &p_reason) {
	int idx = _connected_peers.find(p_peer_id);
	if (idx >= 0) {
		_connected_peers.remove_at(idx);
	}

	if (_clients.has(p_peer_id)) {
		_clients[p_peer_id].state = CLIENT_DISCONNECTED;
	}

	emit_signal("client_disconnected", p_peer_id, p_code, p_reason);
}

void MCPWebSocketServer::_on_data_received(int p_peer_id) {
	if (!_server) return;

	WebSocketPeer *peer = _server->get_peer(p_peer_id);
	if (!peer) return;

	while (peer->get_available_packet_count() > 0) {
		const uint8_t *data = nullptr;
		int size = 0;
		Error err = peer->get_packet(&data, size);
		if (err != OK || !data || size <= 0) continue;

		String json_str;
		json_str.parse_utf8((const char *)data, size);

		Dictionary message;
		if (_parse_json_message(json_str, message)) {
			if (_clients.has(p_peer_id)) {
				_clients[p_peer_id].messages_received++;
			}
			_handle_message(p_peer_id, message);
		} else {
			send_error_response(p_peer_id, Variant(), -32700, "Parse error: invalid JSON");
		}
	}
}

void MCPWebSocketServer::_handle_message(int p_peer_id, const Dictionary &p_message) {
	String jsonrpc = p_message.get("jsonrpc", "");
	if (jsonrpc != "2.0") {
		send_error_response(p_peer_id, p_message.get("id", Variant()), -32600, "Invalid Request: jsonrpc must be '2.0'");
		return;
	}

	String method = p_message.get("method", "");
	bool has_id = p_message.has("id");

	// 通知消息（无 id）
	if (!has_id && !method.is_empty()) {
		if (method == "notifications/initialized") {
			if (_clients.has(p_peer_id)) {
				_clients[p_peer_id].state = CLIENT_AUTHENTICATED;
			}
			emit_signal("client_initialized", p_peer_id);
			return;
		}

		if (method == "notifications/cancelled") {
			Dictionary params = p_message.get("params", Dictionary());
			int req_id = params.get("requestId", -1);
			if (_clients.has(p_peer_id) && _clients[p_peer_id].pending_requests.has(req_id)) {
				_clients[p_peer_id].pending_requests.erase(req_id);
			}
			emit_signal("request_cancelled", p_peer_id, req_id);
			return;
		}

		if (_message_handler.is_valid()) {
			_message_handler.call(p_peer_id, p_message);
		}
		return;
	}

	// 请求消息（有 id）
	if (method.is_empty()) {
		send_error_response(p_peer_id, p_message.get("id", Variant()), -32600, "Invalid Request: method is required");
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

		send_response(p_peer_id, p_message["id"], result);

		if (_clients.has(p_peer_id)) {
			_clients[p_peer_id].state = CLIENT_AUTHENTICATED;
			Dictionary client_info_dict = params.get("clientInfo", Dictionary());
			String client_name = client_info_dict.get("name", "");
			if (client_name.find("bridge") >= 0 || client_name.find("Bridge") >= 0) {
				_clients[p_peer_id].client_type = "ai_bridge";
			} else if (client_name.find("debug") >= 0 || client_name.find("Debug") >= 0) {
				_clients[p_peer_id].client_type = "debug_tool";
			}
			_clients[p_peer_id].name = client_name;
		}
		return;
	}

	// 记录待处理请求
	if (_clients.has(p_peer_id)) {
		Variant id_var = p_message["id"];
		if (id_var.get_type() == Variant::INT) {
			_clients[p_peer_id].pending_requests[(int)id_var] = method;
		}
	}

	// 转发给外部处理器
	if (_message_handler.is_valid()) {
		_message_handler.call(p_peer_id, p_message);
	} else {
		send_error_response(p_peer_id, p_message["id"], -32601, vformat("Method not found: %s", method));
	}
}

bool MCPWebSocketServer::_parse_json_message(const String &p_json, Dictionary &r_out) const {
	if (p_json.strip_edges().is_empty()) return false;

	Variant parsed;
	String err_str;
	int err_line;
	Error err = JSON::parse(p_json, parsed, err_str, err_line);
	if (err != OK) return false;
	if (parsed.get_type() != Variant::DICTIONARY) return false;

	r_out = parsed;
	return true;
}

String MCPWebSocketServer::_serialize_message(const Dictionary &p_message) const {
	return JSON::stringify(p_message, "\t", false);
}

void MCPWebSocketServer::_send_ping() {
	if (!_is_running || !_server) return;

	for (int i = 0; i < _connected_peers.size(); i++) {
		WebSocketPeer *peer = _server->get_peer(_connected_peers[i]);
		if (peer && peer->is_connected_to_host()) {
			peer->ping();
		}
	}
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPWebSocketServer::_bind_methods() {
	BIND_ENUM_CONSTANT(CLIENT_CONNECTING);
	BIND_ENUM_CONSTANT(CLIENT_CONNECTED);
	BIND_ENUM_CONSTANT(CLIENT_AUTHENTICATED);
	BIND_ENUM_CONSTANT(CLIENT_DISCONNECTED);

	ClassDB::bind_method(D_METHOD("start", "port"), &MCPWebSocketServer::start, DEFVAL(8765));
	ClassDB::bind_method(D_METHOD("stop"), &MCPWebSocketServer::stop);
	ClassDB::bind_method(D_METHOD("poll"), &MCPWebSocketServer::poll);
	ClassDB::bind_method(D_METHOD("is_running"), &MCPWebSocketServer::is_running);
	ClassDB::bind_method(D_METHOD("get_port"), &MCPWebSocketServer::get_port);

	ClassDB::bind_method(D_METHOD("broadcast", "type", "data"), &MCPWebSocketServer::broadcast);
	ClassDB::bind_method(D_METHOD("send_to", "peer_id", "type", "data"), &MCPWebSocketServer::send_to);
	ClassDB::bind_method(D_METHOD("send_response", "peer_id", "request_id", "result"), &MCPWebSocketServer::send_response);
	ClassDB::bind_method(D_METHOD("send_error_response", "peer_id", "request_id", "code", "message"), &MCPWebSocketServer::send_error_response);
	ClassDB::bind_method(D_METHOD("send_notification", "peer_id", "method", "params"), &MCPWebSocketServer::send_notification, DEFVAL(Dictionary()));

	ClassDB::bind_method(D_METHOD("get_client_count"), &MCPWebSocketServer::get_client_count);
	ClassDB::bind_method(D_METHOD("get_clients_info"), &MCPWebSocketServer::get_clients_info);
	ClassDB::bind_method(D_METHOD("disconnect_client", "peer_id", "code", "reason"), &MCPWebSocketServer::disconnect_client, DEFVAL(1000), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_client_info", "peer_id"), &MCPWebSocketServer::get_client_info);

	ClassDB::bind_method(D_METHOD("set_max_clients", "max"), &MCPWebSocketServer::set_max_clients);
	ClassDB::bind_method(D_METHOD("get_max_clients"), &MCPWebSocketServer::get_max_clients);
	ClassDB::bind_method(D_METHOD("set_ping_interval_msec", "msec"), &MCPWebSocketServer::set_ping_interval_msec);
	ClassDB::bind_method(D_METHOD("get_ping_interval_msec"), &MCPWebSocketServer::get_ping_interval_msec);
	ClassDB::bind_method(D_METHOD("set_message_handler", "handler"), &MCPWebSocketServer::set_message_handler);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_clients", PROPERTY_HINT_RANGE, "1,16,1"), "set_max_clients", "get_max_clients");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ping_interval_msec", PROPERTY_HINT_RANGE, "0,60000,1000"), "set_ping_interval_msec", "get_ping_interval_msec");

	ADD_SIGNAL(MethodInfo("client_connected", PropertyInfo(Variant::INT, "peer_id")));
	ADD_SIGNAL(MethodInfo("client_disconnected", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::INT, "code"), PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("client_initialized", PropertyInfo(Variant::INT, "peer_id")));
	ADD_SIGNAL(MethodInfo("request_cancelled", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::INT, "request_id")));
	ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::DICTIONARY, "message")));
}
