/**
 * AI Bridge 共享类型定义
 * 所有公开接口的类型都在此文件中定义
 */
/** 判断是否为 JSON-RPC 请求（有 id 的消息） */
export function isJsonRpcRequest(msg) {
    return msg !== null
        && typeof msg === 'object'
        && !Array.isArray(msg)
        && 'method' in msg
        && Object.prototype.hasOwnProperty.call(msg, 'id');
}
/** 判断是否为 JSON-RPC 响应 */
export function isJsonRpcResponse(msg) {
    return msg !== null
        && typeof msg === 'object'
        && !Array.isArray(msg)
        && ('result' in msg || 'error' in msg);
}
/** 判断是否为 JSON-RPC 通知（无 id） */
export function isJsonRpcNotification(msg) {
    return msg !== null
        && typeof msg === 'object'
        && !Array.isArray(msg)
        && 'method' in msg
        && !Object.prototype.hasOwnProperty.call(msg, 'id');
}
//# sourceMappingURL=types.js.map