#ifndef SHADER_H
#define SHADER_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "ers/common.h"
#include "ers/matrix.h"
#include "ers/vec.h"
#include "ers/macros.h"

// Wrapper class for loading and interacting with shader programs for OpenGL.
class GLShaderProgram
{
public:
	GLuint id;

	GLShaderProgram() = default;
	GLShaderProgram(const char* vertex_filename, const char* fragment_filename);
#ifndef __EMSCRIPTEN__
	GLShaderProgram(const char* vertex_filename, const char* geometry_filename, const char* fragment_filename);
#endif
	~GLShaderProgram();
	
	void Use() const;
	void Destroy();

	void Set(const char* name, bool b) const;
	void Set(const char* name, int n) const;
	void Set(const char* name, float f) const;
	void Set(const char* name, const ers::vec2& vec) const;
	void Set(const char* name, const ers::vec3& vec) const;
	void Set(const char* name, const ers::vec4& vec) const;
	void Set(const char* name, const ers::mat4& m) const;

	void LoadFromSource(const char* vertex_source, const char* geometry_source, const char* fragment_source);

private:
	GLuint compileShader(const char* shader_source, GLenum type);
};

#endif // SHADER_H