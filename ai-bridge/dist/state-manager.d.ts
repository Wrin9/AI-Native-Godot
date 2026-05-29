/**
 * 状态缓存管理器
 * 缓存 Godot 的工具列表、场景树等数据
 * 支持增量更新和事件驱动的缓存失效
 */
import type { DeltaUpdate, GodotEvent, ToolDefinition } from './types.js';
export declare class StateManager {
    private cache;
    private lastUpdate;
    private deltaBuffer;
    /** 缓存键名常量 */
    static readonly KEYS: {
        readonly TOOLS: "tools";
        readonly SCENE_TREE: "scene_tree";
        readonly PROJECT_INFO: "project_info";
        readonly EDITOR_STATE: "editor_state";
        readonly SELECTION: "selection";
    };
    /** 获取缓存值 */
    get<T = unknown>(key: string): T | null;
    /** 设置缓存值 */
    set<T = unknown>(key: string, value: T, ttl?: number): void;
    /** 获取缓存的工具列表 */
    getCachedTools(): ToolDefinition[] | null;
    /** 设置缓存的工具列表 */
    setCachedTools(tools: ToolDefinition[]): void;
    /** 获取缓存的场景树 */
    getCachedSceneTree(): unknown | null;
    /** 设置缓存的场景树 */
    setCachedSceneTree(tree: unknown): void;
    /** 应用 Godot 推送的事件更新缓存 */
    applyEvent(event: GodotEvent): void;
    /** 标记缓存过期 */
    invalidate(key: string): void;
    /** 标记所有缓存过期 */
    invalidateAll(): void;
    /** 获取增量更新（自上次获取以来的变化） */
    getDelta(): DeltaUpdate;
    /** 获取最后更新时间 */
    getLastUpdate(): number;
    /** 获取快照 */
    getSnapshot(): Record<string, unknown> | null;
    /** 检查缓存是否存在且未过期 */
    has(key: string): boolean;
    /** 获取缓存统计信息 */
    getStats(): {
        size: number;
        keys: string[];
        lastUpdate: number;
    };
}
//# sourceMappingURL=state-manager.d.ts.map