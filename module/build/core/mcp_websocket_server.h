/**************************************************************************/
/*  mcp_websocket_server.h                                                */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPWebSocketServer — WebSocket 服务器                                   */
/*                                                                        */
/* 监听 MCP 客户端的 WebSocket 连接，接收 JSON-RPC 请求，                   */
/* 将响应和事件推送回客户端。                                              */
/**************************************************************************/

#ifndef MCP_WEBSOCKET_SERVER_H
#define MCP_WEBSOCKET_SERVER_H

#include "core/object/object.h"
#include "core/variant/dictionary.h"

class MCPWebSocketServer : public Object {
	GDCLASS(MCPWebSocketServer, Object);

public:
	/** 启动服务器 */
	bool start(int p_port = 9877);

	/** 停止服务器 */
	void stop();

	/** 轮询网络事件（由 MCPEditorPlugin::_process 调用） */
	void poll();

	/** 服务器是否正在运行 */
	bool is_running() const { return _running; }

	/** 获取监听端口 */
	int get_port() const { return _port; }

	/** 向所有客户端发送消息 */
	void broadcast(const String &p_message);

	/** 向指定客户端发送消息 */
	void send_to(int p_client_id, const String &p_message);

	/** 获取当前连接数 */
	int get_client_count() const;

	MCPWebSocketServer() = default;
	~MCPWebSocketServer();

protected:
	static void _bind_methods();

private:
	bool _running = false;
	int _port = 9877;
	int _server_id = -1; // WebSocket 服务端 ID

	/** 处理新客户端连接 */
	void _on_client_connected(int p_client_id);
	/** 处理客户端断开 */
	void _on_client_disconnected(int p_client_id);
	/** 处理客户端消息 */
	void _on_message_received(int p_client_id, const PackedByteArray &p_data);

	/** 解析并处理 JSON-RPC 请求 */
	void _handle_request(int p_client_id, const Dictionary &p_request);
};

#endif // MCP_WEBSOCKET_SERVER_H
