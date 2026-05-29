/**************************************************************************/
/*  file_tools.h                                                          */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* FileTools — 文件系统 MCP 工具组                                         */
/*                                                                        */
/* 提供：文件读写、目录列表、资源导入等工具。                              */
/**************************************************************************/

#ifndef FILE_TOOLS_H
#define FILE_TOOLS_H

#include "../mcp_tool_base.h"

class FileTools : public MCPToolBase {
	GDCLASS(FileTools, MCPToolBase);

public:
	Array get_tool_definitions() const override;

	String read_file(const Dictionary &p_args);
	String write_file(const Dictionary &p_args);
	String list_dir(const Dictionary &p_args);
	String delete_file(const Dictionary &p_args);
	String make_dir(const Dictionary &p_args);
	String file_exists(const Dictionary &p_args);

protected:
	static void _bind_methods();
};

#endif // FILE_TOOLS_H
