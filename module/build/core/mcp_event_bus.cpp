/**************************************************************************/
/*  mcp_event_bus.cpp                                                     */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* MCPEventBus 实现 — 编辑器事件的收集与批量推送。                          */
/**************************************************************************/

#include "core/mcp_event_bus.h"

#include "core/os/time.h"

void MCPEventBus::_bind_methods() {
	ClassDB::bind_method(D_METHOD("emit", "event_type", "data"), &MCPEventBus::emit, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("flush_events"), &MCPEventBus::flush_events);
	ClassDB::bind_method(D_METHOD("get_pending_event_count"), &MCPEventBus::get_pending_event_count);
	ClassDB::bind_method(D_METHOD("clear"), &MCPEventBus::clear);
	ClassDB::bind_method(D_METHOD("add_listener", "callback"), &MCPEventBus::add_listener);
	ClassDB::bind_method(D_METHOD("remove_listener", "callback"), &MCPEventBus::remove_listener);
}

void MCPEventBus::emit(const String &p_event_type, const Dictionary &p_data) {
	Event evt;
	evt.type = p_event_type;
	evt.data = p_data;
	evt.timestamp = Time::get_singleton()->get_ticks_msec();
	_pending.push_back(evt);
}

void MCPEventBus::flush_events() {
	if (_pending.is_empty()) {
		return;
	}

	// 将所有待发事件打包为批量通知
	Array events;
	for (const Event &evt : _pending) {
		Dictionary evt_dict;
		evt_dict["type"] = evt.type;
		evt_dict["data"] = evt.data;
		evt_dict["timestamp"] = evt.timestamp;
		events.append(evt_dict);
	}
	_pending.clear();

	// 通知所有监听者
	for (const Callable &cb : _listeners) {
		Variant args[] = { events };
		Callable::CallError err;
		cb.callp(args, 1, err);
		if (err.error != Callable::CallError::CALL_OK) {
			WARN_PRINT("MCPEventBus: 监听回调调用失败");
		}
	}
}

int MCPEventBus::get_pending_event_count() const {
	return _pending.size();
}

void MCPEventBus::clear() {
	_pending.clear();
}

void MCPEventBus::add_listener(const Callable &p_callback) {
	_listeners.append(p_callback);
}

void MCPEventBus::remove_listener(const Callable &p_callback) {
	_listeners.erase(p_callback);
}
