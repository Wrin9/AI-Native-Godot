/**
 * shader_tools.h - Shader 工具
 *
 * Shader 资源的创建、代码编辑和参数设置。
 */
#ifndef SHADER_TOOLS_H
#define SHADER_TOOLS_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class EditorPlugin;

class ShaderTools : public RefCounted {
	GDCLASS(ShaderTools, RefCounted);

public:
	void set_editor_plugin(EditorPlugin *p_plugin);

	// 创建 Shader 资源
	String create_shader(const Dictionary &p_args);

	// 设置 Shader 代码
	String set_shader_code(const Dictionary &p_args);

	// 设置 Shader 参数（通过 ShaderMaterial）
	String set_shader_param(const Dictionary &p_args);

protected:
	static void _bind_methods();

private:
	EditorPlugin *_plugin = nullptr;
};

#endif // SHADER_TOOLS_H
