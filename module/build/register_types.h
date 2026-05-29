/**************************************************************************/
/*  register_types.h                                                      */
/**************************************************************************/
/*                        MCP Editor Module                               */
/*                                                                        */
/* Godot 模块注册头文件 — 声明模块的初始化/反初始化函数                    */
/* 编译时由 SCons 自动收集，引擎启动时按级别调用                           */
/**************************************************************************/

#ifndef MCP_EDITOR_REGISTER_TYPES_H
#define MCP_EDITOR_REGISTER_TYPES_H

#include "modules/register_module_types.h"

void initialize_mcp_editor_module(ModuleInitializationLevel p_level);
void uninitialize_mcp_editor_module(ModuleInitializationLevel p_level);

#endif // MCP_EDITOR_REGISTER_TYPES_H
