/**
 * 状态缓存管理器
 * 缓存 Godot 的工具列表、场景树等数据
 * 支持增量更新和事件驱动的缓存失效
 */

import type { CacheEntry, DeltaUpdate, GodotEvent, ToolDefinition } from './types.js';

/** 默认缓存生存时间（毫秒） */
const DEFAULT_TTL = 60_000; // 1 分钟
/** 工具列表缓存生存时间 */
const TOOLS_TTL = 300_000; // 5 分钟
/** 场景树缓存生存时间 */
const SCENE_TREE_TTL = 10_000; // 10 秒

export class StateManager {
  private cache: Map<string, CacheEntry> = new Map();
  private lastUpdate = 0;
  private deltaBuffer: DeltaUpdate = {
    added: {},
    modified: {},
    removed: [],
    timestamp: 0,
  };

  /** 缓存键名常量 */
  static readonly KEYS = {
    TOOLS: 'tools',
    SCENE_TREE: 'scene_tree',
    PROJECT_INFO: 'project_info',
    EDITOR_STATE: 'editor_state',
    SELECTION: 'selection',
  } as const;

  /** 获取缓存值 */
  get<T = unknown>(key: string): T | null {
    const entry = this.cache.get(key);
    if (entry === undefined) {
      return null;
    }

    // 检查是否过期
    if (entry.ttl > 0 && Date.now() - entry.timestamp > entry.ttl) {
      this.cache.delete(key);
      return null;
    }

    return entry.value as T;
  }

  /** 设置缓存值 */
  set<T = unknown>(key: string, value: T, ttl: number = DEFAULT_TTL): void {
    this.cache.set(key, {
      value,
      timestamp: Date.now(),
      ttl,
    });
    this.lastUpdate = Date.now();
  }

  /** 获取缓存的工具列表 */
  getCachedTools(): ToolDefinition[] | null {
    return this.get<ToolDefinition[]>(StateManager.KEYS.TOOLS);
  }

  /** 设置缓存的工具列表 */
  setCachedTools(tools: ToolDefinition[]): void {
    this.set(StateManager.KEYS.TOOLS, tools, TOOLS_TTL);
  }

  /** 获取缓存的场景树 */
  getCachedSceneTree(): unknown | null {
    return this.get(StateManager.KEYS.SCENE_TREE);
  }

  /** 设置缓存的场景树 */
  setCachedSceneTree(tree: unknown): void {
    this.set(StateManager.KEYS.SCENE_TREE, tree, SCENE_TREE_TTL);
  }

  /** 应用 Godot 推送的事件更新缓存 */
  applyEvent(event: GodotEvent): void {
    const now = Date.now();

    switch (event.type) {
      case 'scene_tree_changed': {
        // 场景树变更，使缓存失效
        this.invalidate(StateManager.KEYS.SCENE_TREE);
        // 记录增量
        this.deltaBuffer.modified['scene_tree'] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'tools_changed': {
        // 工具列表变更，使缓存失效
        this.invalidate(StateManager.KEYS.TOOLS);
        this.deltaBuffer.modified['tools'] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'selection_changed': {
        // 选择变更，更新缓存
        this.set(StateManager.KEYS.SELECTION, event.data, SCENE_TREE_TTL);
        this.deltaBuffer.modified['selection'] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'node_property_changed': {
        // 节点属性变更，使场景树缓存失效
        this.invalidate(StateManager.KEYS.SCENE_TREE);
        this.deltaBuffer.modified[`node:${event.data.node as string}`] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'node_created':
      case 'node_deleted': {
        // 节点创建/删除，使场景树缓存失效
        this.invalidate(StateManager.KEYS.SCENE_TREE);
        if (event.type === 'node_created') {
          this.deltaBuffer.added[`node:${event.data.node as string}`] = event.data;
        } else {
          this.deltaBuffer.removed.push(`node:${event.data.node as string}`);
        }
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'project_settings_changed': {
        // 项目设置变更
        this.invalidate(StateManager.KEYS.PROJECT_INFO);
        this.deltaBuffer.modified['project_info'] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }

      case 'state_snapshot': {
        // 完整状态快照
        // 存储快照数据到各个缓存项
        if (event.data.tools) {
          this.setCachedTools(event.data.tools as ToolDefinition[]);
        }
        if (event.data.scene_tree) {
          this.setCachedSceneTree(event.data.scene_tree);
        }
        this.deltaBuffer = {
          added: {},
          modified: {},
          removed: [],
          timestamp: now,
        };
        break;
      }

      default: {
        // 未知事件类型，保存到增量缓冲
        this.deltaBuffer.modified[event.type] = event.data;
        this.deltaBuffer.timestamp = now;
        break;
      }
    }
  }

  /** 标记缓存过期 */
  invalidate(key: string): void {
    this.cache.delete(key);
  }

  /** 标记所有缓存过期 */
  invalidateAll(): void {
    this.cache.clear();

    this.deltaBuffer = {
      added: {},
      modified: {},
      removed: [],
      timestamp: 0,
    };
  }

  /** 获取增量更新（自上次获取以来的变化） */
  getDelta(): DeltaUpdate {
    const delta = { ...this.deltaBuffer };
    // 重置增量缓冲
    this.deltaBuffer = {
      added: {},
      modified: {},
      removed: [],
      timestamp: Date.now(),
    };
    return delta;
  }

  /** 获取最后更新时间 */
  getLastUpdate(): number {
    return this.lastUpdate;
  }

  /** 获取快照 */
  getSnapshot(): Record<string, unknown> | null {
    const result: Record<string, unknown> = {};
    for (const [key, entry] of this.cache) {
      if (entry.ttl === 0 || Date.now() - entry.timestamp <= entry.ttl) {
        result[key] = entry.value;
      }
    }
    return Object.keys(result).length > 0 ? result : null;
  }

  /** 检查缓存是否存在且未过期 */
  has(key: string): boolean {
    return this.get(key) !== null;
  }

  /** 获取缓存统计信息 */
  getStats(): { size: number; keys: string[]; lastUpdate: number } {
    return {
      size: this.cache.size,
      keys: Array.from(this.cache.keys()),
      lastUpdate: this.lastUpdate,
    };
  }
}
