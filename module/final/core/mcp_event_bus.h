/**************************************************************************/
/*  mcp_event_bus.h                                                       */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 编辑器事件总线 - 推送编辑器状态变更给 AI                                 */
/* 监听编辑器信号、事件合并、订阅分发                                       */
/**************************************************************************/

#ifndef MCP_EVENT_BUS_H
#define MCP_EVENT_BUS_H

#include "core/object/object.h"
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"

class EditorPlugin;
class Node;

class MCPEventBus : public Object {
	GDCLASS(MCPEventBus, Object);

public:
	// 事件类型常量
	// 场景相关
	static const String EVENT_SCENE_CHANGED;
	static const String EVENT_NODE_CREATED;
	static const String EVENT_NODE_REMOVED;
	static const String EVENT_NODE_RENAMED;
	static const String EVENT_NODE_MOVED;
	static const String EVENT_PROPERTY_CHANGED;

	// 脚本相关
	static const String EVENT_SCRIPT_COMPILED;
	static const String EVENT_SCRIPT_ERROR;
	static const String EVENT_SCRIPT_OPENED;

	// 编辑器操作
	static const String EVENT_SAVE_COMPLETED;
	static const String EVENT_UNDO_REDO;

	// 运行状态
	static const String EVENT_PLAY_STATE_CHANGED;
	static const String EVENT_CONSOLE_MESSAGE;
	static const String EVENT_FRAME_TICK;

	// 项目相关
	static const String EVENT_PROJECT_SETTING_CHANGED;
	static const String EVENT_FILE_CHANGED;

	// 构造 / 析构
	MCPEventBus();
	~MCPEventBus();

	// ---- 公开 API ----

	// 设置编辑器监听（在插件初始化时调用）
	void setup_editor_listeners(EditorPlugin *p_plugin);

	// 手动推送事件
	void push_event(const String &p_type, const Dictionary &p_data = Dictionary());

	// 订阅特定类型的事件
	void subscribe(const String &p_event_type, const Callable &p_callback);

	// 取消订阅
	void unsubscribe(const String &p_event_type, const Callable &p_callback);

	// 设置事件合并刷新间隔（毫秒）
	void set_flush_interval_msec(uint64_t p_msec);

	// 获取事件合并刷新间隔
	uint64_t get_flush_interval_msec() const;

	// 刷新缓冲区（在 _process 中调用或手动触发）
	void flush_events();

	// 处理编辑器轮询（在 _process 中调用）
	void process();

	// 获取事件缓冲区大小
	int get_buffer_size() const;

	// 清空事件缓冲区
	void clear_buffer();

	// 获取订阅者数量
	int get_subscriber_count(const String &p_event_type) const;

	// 获取总事件计数
	uint64_t get_total_event_count() const;

	// 启用/禁用事件总线
	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	// 设置事件过滤（只推送指定类型的事件）
	void set_event_filter(const Vector<String> &p_types);
	Vector<String> get_event_filter() const;
	void clear_event_filter();

protected:
	static void _bind_methods();

private:
	// 事件结构体
	struct Event {
		String type;
		Dictionary data;
		uint64_t timestamp_msec = 0;

		Event() = default;
		Event(const String &p_type, const Dictionary &p_data, uint64_t p_ts) :
				type(p_type), data(p_data), timestamp_msec(p_ts) {}
	};

	// 事件缓冲区
	Vector<Event> _event_buffer;

	// 订阅者映射（事件类型 → 回调列表）
	HashMap<String, Vector<Callable>> _subscribers;

	// 事件过滤
	HashSet<String> _event_filter;
	bool _has_filter = false;

	// 合并间隔
	uint64_t _flush_interval_msec = 100;
	uint64_t _last_flush_msec = 0;

	// 计数统计
	uint64_t _total_event_count = 0;
	HashMap<String, uint64_t> _event_type_counts;

	// 状态
	bool _enabled = true;
	bool _listeners_setup = false;

	// 编辑器引用
	EditorPlugin *_plugin = nullptr;

	// 合并事件的辅助映射（类型 → 缓冲区中的索引列表）
	HashMap<String, Vector<int>> _merge_map;

	// ---- 编辑器信号回调 ----
	void _on_node_added(Node *p_node);
	void _on_node_removed(Node *p_node);
	void _on_node_renamed(Node *p_node);
	void _on_script_changed(Ref<RefCounted> p_resource);
	void _on_scene_changed();
	void _on_play_state_changed();

	// ---- 内部辅助 ----
	void _merge_events(const String &p_type);
	Dictionary _merge_event_data(const Vector<Event> &p_events) const;
	void _dispatch_event(const Event &p_event);
	void _dispatch_merged_event(const String &p_type, const Dictionary &p_merged_data);

	// 节点摘要（轻量，不含完整树）
	Dictionary _node_summary(Node *p_node) const;

	// 检查事件类型是否通过过滤
	bool _is_event_type_allowed(const String &p_type) const;
};

#endif // MCP_EVENT_BUS_H
