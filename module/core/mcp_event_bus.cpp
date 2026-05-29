/**************************************************************************/
/*  mcp_event_bus.cpp                                                     */
/**************************************************************************/
/*                         AI-Native Godot Module                         */
/**************************************************************************/
/* 编辑器事件总线实现                                                       */
/**************************************************************************/

#include "mcp_event_bus.h"

#include "core/os/os.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_interface.h"

// ============================================================
// 事件类型常量定义
// ============================================================

const String MCPEventBus::EVENT_SCENE_CHANGED = "scene_changed";
const String MCPEventBus::EVENT_NODE_CREATED = "node_created";
const String MCPEventBus::EVENT_NODE_REMOVED = "node_removed";
const String MCPEventBus::EVENT_NODE_RENAMED = "node_renamed";
const String MCPEventBus::EVENT_NODE_MOVED = "node_moved";
const String MCPEventBus::EVENT_PROPERTY_CHANGED = "property_changed";
const String MCPEventBus::EVENT_SCRIPT_COMPILED = "script_compiled";
const String MCPEventBus::EVENT_SCRIPT_ERROR = "script_error";
const String MCPEventBus::EVENT_SCRIPT_OPENED = "script_opened";
const String MCPEventBus::EVENT_SAVE_COMPLETED = "save_completed";
const String MCPEventBus::EVENT_UNDO_REDO = "undo_redo";
const String MCPEventBus::EVENT_PLAY_STATE_CHANGED = "play_state_changed";
const String MCPEventBus::EVENT_CONSOLE_MESSAGE = "console_message";
const String MCPEventBus::EVENT_FRAME_TICK = "frame_tick";
const String MCPEventBus::EVENT_PROJECT_SETTING_CHANGED = "project_setting_changed";
const String MCPEventBus::EVENT_FILE_CHANGED = "file_changed";

// ============================================================
// 构造 / 析构
// ============================================================

MCPEventBus::MCPEventBus() {
}

MCPEventBus::~MCPEventBus() {
}

// ============================================================
// 公开 API
// ============================================================

void MCPEventBus::setup_editor_listeners(EditorPlugin *p_plugin) {
	if (_listeners_setup) {
		return; // 避免重复绑定
	}

	_plugin = p_plugin;
	if (!_plugin) {
		WARN_PRINT("MCPEventBus: 插件引用为空，无法设置编辑器监听");
		return;
	}

	// 监听 SceneTree 信号
	SceneTree *tree = SceneTree::get_singleton();
	if (tree) {
		// 节点添加/移除
		tree->connect("node_added", callable_mp(this, &MCPEventBus::_on_node_added));
		tree->connect("node_removed", callable_mp(this, &MCPEventBus::_on_node_removed));

		// 场景变更
		tree->connect("tree_changed", callable_mp(this, &MCPEventBus::_on_scene_changed));
	}

	_listeners_setup = true;
}

void MCPEventBus::push_event(const String &p_type, const Dictionary &p_data) {
	if (!_enabled) {
		return;
	}

	if (!_is_event_type_allowed(p_type)) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_msec();

	Event evt;
	evt.type = p_type;
	evt.data = p_data;
	evt.timestamp_msec = now;

	_event_buffer.push_back(evt);
	_total_event_count++;

	// 更新类型计数
	if (_event_type_counts.has(p_type)) {
		_event_type_counts[p_type]++;
	} else {
		_event_type_counts[p_type] = 1;
	}

	// 更新合并映射
	int idx = _event_buffer.size() - 1;
	if (_merge_map.has(p_type)) {
		_merge_map[p_type].push_back(idx);
	} else {
		Vector<int> indices;
		indices.push_back(idx);
		_merge_map[p_type] = indices;
	}

	// 立即分发 CRITICAL 级别的事件（play_state_changed 等）
	if (p_type == EVENT_PLAY_STATE_CHANGED || p_type == EVENT_SCRIPT_ERROR) {
		flush_events();
	}
}

void MCPEventBus::subscribe(const String &p_event_type, const Callable &p_callback) {
	if (!_subscribers.has(p_event_type)) {
		_subscribers[p_event_type] = Vector<Callable>();
	}
	_subscribers[p_event_type].push_back(p_callback);
}

void MCPEventBus::unsubscribe(const String &p_event_type, const Callable &p_callback) {
	if (!_subscribers.has(p_event_type)) {
		return;
	}

	Vector<Callable> &callbacks = _subscribers[p_event_type];
	int idx = callbacks.find(p_callback);
	if (idx >= 0) {
		callbacks.remove_at(idx);
	}

	// 清理空的订阅列表
	if (callbacks.is_empty()) {
		_subscribers.erase(p_event_type);
	}
}

void MCPEventBus::set_flush_interval_msec(uint64_t p_msec) {
	_flush_interval_msec = p_msec;
}

uint64_t MCPEventBus::get_flush_interval_msec() const {
	return _flush_interval_msec;
}

void MCPEventBus::flush_events() {
	if (_event_buffer.is_empty()) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_msec();

	// 按事件类型合并
	Vector<String> types_to_merge;
	for (const KeyValue<String, Vector<int>> &E : _merge_map) {
		// 只合并缓冲区中超过合并间隔的事件
		if (E.value.size() > 0) {
			const Event &first_evt = _event_buffer[E.value[0]];
			if (now - first_evt.timestamp_msec >= _flush_interval_msec || E.value.size() >= 10) {
				types_to_merge.push_back(E.key);
			}
		}
	}

	// 对每个需要合并的类型进行处理
	for (const String &type : types_to_merge) {
		_merge_events(type);
	}

	// 分发不在合并映射中的独立事件
	for (int i = 0; i < _event_buffer.size(); i++) {
		if (_event_buffer[i].type.is_empty()) {
			continue; // 已被合并标记
		}

		// 检查是否在合并映射中（未到合并时间的）
		bool in_merge_map = false;
		for (const KeyValue<String, Vector<int>> &E : _merge_map) {
			for (int idx : E.value) {
				if (idx == i) {
					in_merge_map = true;
					break;
				}
			}
			if (in_merge_map) break;
		}

		if (!in_merge_map) {
			_dispatch_event(_event_buffer[i]);
		}
	}

	// 清空缓冲区和合并映射
	_event_buffer.clear();
	_merge_map.clear();
	_last_flush_msec = now;
}

void MCPEventBus::process() {
	if (!_enabled) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_msec();

	// 检查是否到刷新间隔
	if (now - _last_flush_msec >= _flush_interval_msec) {
		flush_events();
	}
}

int MCPEventBus::get_buffer_size() const {
	return _event_buffer.size();
}

void MCPEventBus::clear_buffer() {
	_event_buffer.clear();
	_merge_map.clear();
}

int MCPEventBus::get_subscriber_count(const String &p_event_type) const {
	if (!_subscribers.has(p_event_type)) {
		return 0;
	}
	return _subscribers[p_event_type].size();
}

uint64_t MCPEventBus::get_total_event_count() const {
	return _total_event_count;
}

void MCPEventBus::set_enabled(bool p_enabled) {
	_enabled = p_enabled;
}

bool MCPEventBus::is_enabled() const {
	return _enabled;
}

void MCPEventBus::set_event_filter(const Vector<String> &p_types) {
	_event_filter.clear();
	_has_filter = false;
	for (int i = 0; i < p_types.size(); i++) {
		_event_filter.insert(p_types[i]);
	}
	_has_filter = !p_types.is_empty();
}

Vector<String> MCPEventBus::get_event_filter() const {
	Vector<String> types;
	for (const String &type : _event_filter) {
		types.push_back(type);
	}
	return types;
}

void MCPEventBus::clear_event_filter() {
	_event_filter.clear();
	_has_filter = false;
}

// ============================================================
// 编辑器信号回调
// ============================================================

void MCPEventBus::_on_node_added(Node *p_node) {
	if (!p_node) return;

	// 过滤内部节点
	if (p_node->is_internal()) return;

	Dictionary data = _node_summary(p_node);
	data["action"] = "created";
	push_event(EVENT_NODE_CREATED, data);
}

void MCPEventBus::_on_node_removed(Node *p_node) {
	if (!p_node) return;

	if (p_node->is_internal()) return;

	Dictionary data = _node_summary(p_node);
	data["action"] = "removed";
	push_event(EVENT_NODE_REMOVED, data);
}

void MCPEventBus::_on_node_renamed(Node *p_node) {
	if (!p_node) return;

	Dictionary data = _node_summary(p_node);
	data["action"] = "renamed";
	push_event(EVENT_NODE_RENAMED, data);
}

void MCPEventBus::_on_script_changed(Ref<RefCounted> p_resource) {
	Dictionary data;
	data["resource_type"] = "script";
	push_event(EVENT_SCRIPT_COMPILED, data);
}

void MCPEventBus::_on_scene_changed() {
	Dictionary data;
	data["timestamp"] = OS::get_singleton()->get_ticks_msec();
	push_event(EVENT_SCENE_CHANGED, data);
}

void MCPEventBus::_on_play_state_changed() {
	Dictionary data;
	SceneTree *tree = SceneTree::get_singleton();
	if (tree) {
		data["is_playing"] = EditorInterface::get_singleton() && EditorInterface::get_singleton()->is_playing_scene();
	}
	push_event(EVENT_PLAY_STATE_CHANGED, data);
}

// ============================================================
// 内部辅助方法
// ============================================================

void MCPEventBus::_merge_events(const String &p_type) {
	if (!_merge_map.has(p_type)) {
		return;
	}

	const Vector<int> &indices = _merge_map[p_type];
	if (indices.is_empty()) {
		return;
	}

	// 收集同类型的所有事件
	Vector<Event> same_type_events;
	for (int idx : indices) {
		if (idx >= 0 && idx < _event_buffer.size()) {
			same_type_events.push_back(_event_buffer[idx]);
			// 标记为已合并
			_event_buffer.write[idx].type = "";
		}
	}

	if (same_type_events.size() <= 1) {
		// 只有一个事件，直接分发
		if (same_type_events.size() == 1) {
			_dispatch_event(same_type_events[0]);
		}
		return;
	}

	// 合并事件数据
	Dictionary merged = _merge_event_data(same_type_events);
	_dispatch_merged_event(p_type, merged);
}

Dictionary MCPEventBus::_merge_event_data(const Vector<Event> &p_events) const {
	Dictionary merged;
	Array items;

	for (const Event &evt : p_events) {
		items.push_back(evt.data);
	}

	merged["count"] = items.size();
	merged["items"] = items;

	// 使用第一个事件的时间戳和最后一个事件的时间戳
	if (!p_events.is_empty()) {
		merged["first_timestamp"] = (int64_t)p_events[0].timestamp_msec;
		merged["last_timestamp"] = (int64_t)p_events[p_events.size() - 1].timestamp_msec;
	}

	return merged;
}

void MCPEventBus::_dispatch_event(const Event &p_event) {
	if (!_subscribers.has(p_event.type)) {
		// 即使没有订阅者，也发出通用信号
		emit_signal("event_dispatched", p_event.type, p_event.data);
		return;
	}

	const Vector<Callable> &callbacks = _subscribers[p_event.type];
	for (int i = 0; i < callbacks.size(); i++) {
		const Callable &cb = callbacks[i];
		if (cb.is_valid()) {
			cb.call(p_event.type, p_event.data);
		}
	}

	// 同时发出通用信号
	emit_signal("event_dispatched", p_event.type, p_event.data);
}

void MCPEventBus::_dispatch_merged_event(const String &p_type, const Dictionary &p_merged_data) {
	// 分发给订阅者
	if (_subscribers.has(p_type)) {
		const Vector<Callable> &callbacks = _subscribers[p_type];
		for (int i = 0; i < callbacks.size(); i++) {
			const Callable &cb = callbacks[i];
			if (cb.is_valid()) {
				cb.call(p_type, p_merged_data);
			}
		}
	}

	// 发出合并事件信号
	emit_signal("events_merged", p_type, p_merged_data);
	emit_signal("event_dispatched", p_type, p_merged_data);
}

Dictionary MCPEventBus::_node_summary(Node *p_node) const {
	Dictionary summary;
	if (!p_node) {
		return summary;
	}

	summary["name"] = p_node->get_name();
	summary["type"] = p_node->get_class();
	summary["path"] = String(p_node->get_path());
	summary["instance_id"] = p_node->get_instance_id();

	// 如果有父节点，包含父节点信息
	Node *parent = p_node->get_parent();
	if (parent) {
		summary["parent_name"] = parent->get_name();
		summary["parent_type"] = parent->get_class();
	}

	return summary;
}

bool MCPEventBus::_is_event_type_allowed(const String &p_type) const {
	if (!_has_filter) {
		return true; // 无过滤，允许所有
	}
	return _event_filter.has(p_type);
}

// ============================================================
// Godot 绑定
// ============================================================

void MCPEventBus::_bind_methods() {
	// 绑定方法
	ClassDB::bind_method(D_METHOD("setup_editor_listeners", "plugin"), &MCPEventBus::setup_editor_listeners);
	ClassDB::bind_method(D_METHOD("push_event", "type", "data"), &MCPEventBus::push_event, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("subscribe", "event_type", "callback"), &MCPEventBus::subscribe);
	ClassDB::bind_method(D_METHOD("unsubscribe", "event_type", "callback"), &MCPEventBus::unsubscribe);
	ClassDB::bind_method(D_METHOD("set_flush_interval_msec", "msec"), &MCPEventBus::set_flush_interval_msec);
	ClassDB::bind_method(D_METHOD("get_flush_interval_msec"), &MCPEventBus::get_flush_interval_msec);
	ClassDB::bind_method(D_METHOD("flush_events"), &MCPEventBus::flush_events);
	ClassDB::bind_method(D_METHOD("process"), &MCPEventBus::process);
	ClassDB::bind_method(D_METHOD("get_buffer_size"), &MCPEventBus::get_buffer_size);
	ClassDB::bind_method(D_METHOD("clear_buffer"), &MCPEventBus::clear_buffer);
	ClassDB::bind_method(D_METHOD("get_subscriber_count", "event_type"), &MCPEventBus::get_subscriber_count);
	ClassDB::bind_method(D_METHOD("get_total_event_count"), &MCPEventBus::get_total_event_count);
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &MCPEventBus::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &MCPEventBus::is_enabled);
	ClassDB::bind_method(D_METHOD("set_event_filter", "types"), &MCPEventBus::set_event_filter);
	ClassDB::bind_method(D_METHOD("get_event_filter"), &MCPEventBus::get_event_filter);
	ClassDB::bind_method(D_METHOD("clear_event_filter"), &MCPEventBus::clear_event_filter);

	// 属性
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flush_interval_msec", PROPERTY_HINT_RANGE, "10,1000,10"), "set_flush_interval_msec", "get_flush_interval_msec");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");

	// 信号
	ADD_SIGNAL(MethodInfo("event_dispatched",
			PropertyInfo(Variant::STRING, "type"),
			PropertyInfo(Variant::DICTIONARY, "data")));

	ADD_SIGNAL(MethodInfo("events_merged",
			PropertyInfo(Variant::STRING, "type"),
			PropertyInfo(Variant::DICTIONARY, "merged_data")));
}
