"""
MCP Editor Module — SCons 构建配置
Godot 模块构建入口文件，告诉 SCons 这个模块是否可构建/启用，
以及需要生成文档的类列表。
"""


def can_build(env, platform):
    """所有平台都可构建"""
    return True


def configure(env):
    """模块级编译配置（当前无需特殊配置）"""
    pass


def get_doc_classes():
    """需要生成 Godot 文档的类列表"""
    return [
        "MCPEditorPlugin",
        "MCPCommandQueue",
        "MCPSandbox",
        "MCPEventBus",
        "MCPWebSocketServer",
        "MCPSnapshot",
        "ToolDispatcher",
    ]


def get_doc_path():
    """文档类 XML 所在目录"""
    return "doc_classes"


def is_enabled():
    """默认启用"""
    return True
