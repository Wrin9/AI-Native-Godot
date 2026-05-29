/**************************************************************************/
/*  mcp_websocket_server.h                                                */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* WebSocket 服务器 - 替代 HTTP 实现双向实时通信                            */
/* 支持多客户端连接、JSON-RPC 2.0（兼容 MCP 协议）                         */
/**************************************************************************/

#ifndef MCP_WEBSOCKET_SERVER_H
#define MCP_WEBSOCKET_SERVER_H

#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/templates/hash_map.h"

// 前向声明，避免直接包含 WebSocket 头文件
class WebSocketServer;
class WebSocketPeer;

class MCPWebSocketServer : public RefCounted {
	GDCLASS(MCPWebSocketServer, RefCounted);

public:
	// 客户端状态
	enum ClientState {
		CLIENT_CONNECTING,
		CLIENT_CONNECTED,
		CLIENT_AUTHENTICATED,
		CLIENT_DISCONNECTED,
	};

	// 客户端信息
	struct ClientInfo {
		int peer_id = -1;
		ClientState state = CLIENT_CONNECTING;
		String name;
		String client_type; // "ai_bridge", "debug_tool", "unknown"
		uint64_t connected_at_msec = 0;
		HashMap<int, String> pending_requests; // request_id -> tool_name
		int messages_received = 0;
		int messages_sent = 0;
	};

	// 构造 / 析构
	MCPWebSocketServer();
	~MCPWebSocketServer();

	// ---- 服务器生命周期 ----

	// 启动服务器
	Error start(int p_port = 8765);

	// 停止服务器
	void stop();

	// 轮询（必须在 _process 中每帧调用）
	void poll();

	// 服务器是否正在运行
	bool is_running() const;

	// 获取监听端口
	int get_port() const;

	// ---- 消息发送 ----

	// 广播消息给所有已连接的 AI 客户端
	void broadcast(const String &p_type, const Dictionary &p_data);

	// 发送消息给特定客户端
	void send_to(int p_peer_id, const String &p_type, const Dictionary &p_data);

	// 发送 JSON-RPC 响应给特定客户端
	void send_response(int p_peer_id, const Variant &p_request_id, const Dictionary &p_result);

	// 发送 JSON-RPC 错误响应
	void send_error_response(int p_peer_id, const Variant &p_request_id, int p_code, const String &p_message);

	// 发送 JSON-RPC 通知（无 id）
	void send_notification(int p_peer_id, const String &p_method, const Dictionary &p_params = Dictionary());

	// ---- 客户端管理 ----

	// 获取已连接客户端数量
	int get_client_count() const;

	// 获取所有已连接客户端信息
	Array get_clients_info() const;

	// 断开特定客户端
	void disconnect_client(int p_peer_id, int p_code = 1000, const String &p_reason = "");

	// 获取特定客户端信息
	Dictionary get_client_info(int p_peer_id) const;

	// ---- 配置 ----

	// 设置最大客户端数
	void set_max_clients(int p_max);
	int get_max_clients() const;

	// 设置心跳间隔（毫秒，0 禁用）
	void set_ping_interval_msec(uint64_t p_msec);
	uint64_t get_ping_interval_msec() const;

	// 设置消息处理回调
	void set_message_handler(const Callable &p_handler);

protected:
	static void _bind_methods();

private:
	// WebSocket 服务器实例
	WebSocketServer *_server = nullptr;

	// 已连接的客户端列表
	Vector<int> _connected_peers;

	// 客户端详细信息
	HashMap<int, ClientInfo> _clients;

	// 配置
	int _port = 8765;
	bool _is_running = false;
	int _max_clients = 4;
	uint64_t _ping_interval_msec = 30000;
	uint64_t _last_ping_msec = 0;

	// 消息处理回调
	Callable _message_handler;

	// 内部方法
	void _on_client_connected(int p_peer_id);
	void _on_client_disconnected(int p_peer_id, int p_code, const String &p_reason);
	void _on_data_received(int p_peer_id);

	// 处理接收到的消息
	void _handle_message(int p_peer_id, const Dictionary &p_message);

	// 解析 JSON 消息
	bool _parse_json_message(const String &p_json, Dictionary &r_out) const;

	// 序列化消息为 JSON
	String _serialize_message(const Dictionary &p_message) const;

	// 定期心跳
	void _send_ping();

	// 序列号（用于通知 ID）
	uint64_t _notification_seq = 0;
};

#endif // MCP_WEBSOCKET_SERVER_H
