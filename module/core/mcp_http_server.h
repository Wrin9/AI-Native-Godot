/**************************************************************************/
/*  mcp_http_server.h                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPHTTPServer — HTTP JSON-RPC 服务器替代 WebSocket                     */
/* 使用 Godot 内置 TCPServer + StreamPeerTCP 实现 HTTP 传输层             */
/* 支持 JSON-RPC 2.0 协议（兼容 MCP）                                    */
/**************************************************************************/

#ifndef MCP_HTTP_SERVER_H
#define MCP_HTTP_SERVER_H

#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/templates/hash_map.h"
#include "core/io/tcp_server.h"
#include "core/io/stream_peer_tcp.h"
#include "core/io/stream_peer_socket.h"

class MCPHTTPServer : public RefCounted {
	GDCLASS(MCPHTTPServer, RefCounted);

public:
	// 客户端状态
	enum ClientState {
		CLIENT_CONNECTED,
		CLIENT_AUTHENTICATED,
		CLIENT_DISCONNECTED,
	};

	// 客户端信息
	struct ClientInfo {
		Ref<StreamPeerTCP> connection;
		ClientState state = CLIENT_CONNECTED;
		String name;
		String client_type;
		uint64_t connected_at_msec = 0;
		int messages_received = 0;
		int messages_sent = 0;
		String recv_buffer; // 不完整 HTTP body 的缓冲
	};

	// 构造 / 析构
	MCPHTTPServer();
	~MCPHTTPServer();

	// ---- 服务器生命周期 ----
	Error start(int p_port = 9877);
	void stop();
	void poll();
	bool is_running() const;
	int get_port() const;

	// ---- 消息发送 ----
	void broadcast(const String &p_type, const Dictionary &p_data);
	void send_to(int p_client_id, const String &p_type, const Dictionary &p_data);
	void send_response(int p_client_id, const Variant &p_request_id, const Dictionary &p_result);
	void send_error_response(int p_client_id, const Variant &p_request_id, int p_code, const String &p_message);
	void send_notification(int p_client_id, const String &p_method, const Dictionary &p_params = Dictionary());

	// ---- 客户端管理 ----
	int get_client_count() const;
	Array get_clients_info() const;
	void disconnect_client(int p_client_id);
	Dictionary get_client_info(int p_client_id) const;

	// ---- 配置 ----
	void set_max_clients(int p_max);
	int get_max_clients() const;
	void set_message_handler(const Callable &p_handler);

protected:
	static void _bind_methods();

private:
	// TCP 服务器
	Ref<TCPServer> _server;

	// 已连接的客户端列表 (client_id → ClientInfo)
	HashMap<int, ClientInfo> _clients;
	Vector<int> _connected_ids;
	int _next_client_id = 1;

	// 配置
	int _port = 9877;
	bool _is_running = false;
	int _max_clients = 4;

	// 消息处理回调
	Callable _message_handler;

	// 通知序号
	uint64_t _notification_seq = 0;

	// 内部方法
	void _accept_new_connections();
	void _poll_clients();
	void _handle_request(int p_client_id, const String &p_body);
	void _send_http_response(int p_client_id, int p_status_code, const String &p_content_type, const String &p_body);
	void _send_json_response(int p_client_id, const Dictionary &p_message);
	void _handle_message(int p_client_id, const Dictionary &p_message);

	// JSON 解析/序列化
	bool _parse_json_message(const String &p_json, Dictionary &r_out) const;
	String _serialize_message(const Dictionary &p_message) const;

	// HTTP 请求解析
	bool _parse_http_request(const String &p_raw, String &r_method, String &r_path, String &r_body) const;
};

#endif // MCP_HTTP_SERVER_H
