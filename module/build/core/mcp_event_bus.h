/**************************************************************************/
/*  mcp_event_bus.h                                                       */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPEventBus — 事件总线                                                  */
/*                                                                        */
/* 收集编辑器状态变更事件，批量推送给 MCP 客户端。                          */
/* 避免逐事件推送造成网络洪水，攒到帧结束时统一发送。                      */
/**************************************************************************/

#ifndef MCP_EVENT_BUS_H
#define MCP_EVENT_BUS_H

#include "core/object/object.h"
#include "core/variant/dictionary.h"

class MCPEventBus : public Object {
	GDCLASS(MCPEventBus, Object);

public:
	/** 发布一个事件 */
	void emit(const String &p_event_type, const Dictionary &p_data = Dictionary());

	/** 刷新所有待发事件（由 MCPEditorPlugin::_process 调用） */
	void flush_events();

	/** 获取待发事件数量 */
	int get_pending_event_count() const;

	/** 清空待发事件 */
	void clear();

	/** 注册事件监听回调 */
	void add_listener(const Callable &p_callback);

	/** 移除事件监听回调 */
	void remove_listener(const Callable &p_callback);

	MCPEventBus() = default;
	~MCPEventBus() = default;

protected:
	static void _bind_methods();

private:
	/** 事件条目 */
	struct Event {
		String type;
		Dictionary data;
		uint64_t timestamp = 0;
	};

	/** 待刷新事件 */
	List<Event> _pending;

	/** 监听回调列表 */
	Vector<Callable> _listeners;
};

#endif // MCP_EVENT_BUS_H
