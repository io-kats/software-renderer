#include "gl_shader_program.h"

#ifdef __EMSCRIPTEN__
#define ERS_GEOMETRY_SHADER GL_FRAGMENT_SHADER
static const ers::StringView glsl_version_vertex("#version 300 es\n");
static const ers::StringView glsl_version_fragment("#version 300 es\nprecision mediump float;\n");
#else
#define ERS_GEOMETRY_SHADER GL_GEOMETRY_SHADER
static const ers::StringView glsl_version_vertex("#version 330\n");
static const ers::StringView glsl_version_fragment = glsl_version_vertex;
static const ers::StringView glsl_version_geometry = glsl_version_vertex;
#endif

GLShaderProgram::GLShaderProgram(const char* vertex_filename, const char* fragment_filename)
{
	ers::String vertex_source(glsl_version_vertex);
	FILE* vertex_file = fopen(vertex_filename, "rb");
	ERS_ASSERTF(vertex_file != nullptr, "String::AppendFile: Couldn't read file: %s", vertex_filename);
	vertex_source.AppendFile(vertex_file);
	fclose(vertex_file);

	ers::String fragment_source(glsl_version_fragment);
	FILE* fragment_file = fopen(fragment_filename, "rb");
	ERS_ASSERTF(vertex_file != nullptr, "String::AppendFile: Couldn't read file: %s", fragment_filename);
	fragment_source.AppendFile(fragment_file);
	fclose(fragment_file);

	LoadFromSource(vertex_source.GetCstr(), nullptr, fragment_source.GetCstr());
}

#ifndef __EMSCRIPTEN__
GLShaderProgram::GLShaderProgram(const char* vertex_filename, const char* geometry_filename, const char* fragment_filename)
{
	ers::String vertex_source(glsl_version_vertex);
	FILE* vertex_file = fopen(vertex_filename, "rb");
	ERS_ASSERTF(vertex_file != nullptr, "String::AppendFile: Couldn't read file: %s", vertex_filename);
	vertex_source.AppendFile(vertex_file);
	fclose(vertex_file);

	ers::String geometry_source(glsl_version_geometry);
	FILE* geometry_file = fopen(geometry_filename, "rb");
	ERS_ASSERTF(vertex_file != nullptr, "String::AppendFile: Couldn't read file: %s", geometry_filename);
	geometry_source.AppendFile(geometry_file);
	fclose(geometry_file);

	ers::String fragment_source(glsl_version_fragment);
	FILE* fragment_file = fopen(fragment_filename, "rb");
	ERS_ASSERTF(vertex_file != nullptr, "String::AppendFile: Couldn't read file: %s", fragment_filename);
	fragment_source.AppendFile(fragment_file);
	fclose(fragment_file);

	LoadFromSource(vertex_source.GetCstr(), geometry_source.GetCstr(), fragment_source.GetCstr());
}
#endif

GLShaderProgram::~GLShaderProgram() { }

void GLShaderProgram::Use() const
{
	glUseProgram(id);
}

void GLShaderProgram::Destroy()
{
	glDeleteProgram(id);
}

void GLShaderProgram::Set(const char* name, bool b) const
{
	glUniform1i(glGetUniformLocation(id, name), b ? int(1) : int(0));
}

void GLShaderProgram::Set(const char* name, int n) const
{
	glUniform1i(glGetUniformLocation(id, name), n);
}

void GLShaderProgram::Set(const char* name, float f) const
{
	glUniform1f(glGetUniformLocation(id, name), f);
}

void GLShaderProgram::Set(const char* name, const ers::vec2& v) const
{
	glUniform2f(glGetUniformLocation(id, name), v.x(), v.y());
}

void GLShaderProgram::Set(const char* name, const ers::vec3& v) const
{
	glUniform3f(glGetUniformLocation(id, name), v.x(), v.y(), v.z());
}

void GLShaderProgram::Set(const char* name, const ers::vec4& v) const
{
	glUniform4f(glGetUniformLocation(id, name), v.x(), v.y(), v.z(), v.w());
}

void GLShaderProgram::Set(const char* name, const ers::mat4& m) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &(m.v[0].e[0]));
}

void GLShaderProgram::LoadFromSource(const char* vertex_source, const char* geometry_source, const char* fragment_source)
{
	id = glCreateProgram(); GL_CHECK();
	
	GLuint vertex_shader = compileShader(vertex_source, GL_VERTEX_SHADER);
	GLuint fragment_shader = compileShader(fragment_source, GL_FRAGMENT_SHADER);

	GLuint geometry_shader;
	if (geometry_source != nullptr) geometry_shader = compileShader(geometry_source, ERS_GEOMETRY_SHADER); // see above for ERS_GEOMETRY_SHADER
		
	glAttachShader(id, vertex_shader); GL_CHECK();
	if (geometry_source != nullptr) { glAttachShader(id, geometry_shader); GL_CHECK(); }
	glAttachShader(id, fragment_shader); GL_CHECK();
	
	glLinkProgram(id); GL_CHECK();

	GLint success;
	GLchar info_log[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success); GL_CHECK();
	if (!success)
	{
		glGetProgramInfoLog(id, 512, nullptr, info_log); GL_CHECK();
		ERS_PANICF(false, "[ERROR] Shader::Shader: Failed to link program.\n%s\n", info_log);
	}

	glDeleteShader(vertex_shader); GL_CHECK();
	if (geometry_source != nullptr) { glDeleteShader(geometry_shader); GL_CHECK(); }	
	glDeleteShader(fragment_shader); GL_CHECK();
}

enum Shader_Type
{
	VERTEX = 0,
	FRAGMENT = 1,
	GEOMETRY = 2,
};

GLuint GLShaderProgram::compileShader(const char* shader_source, GLenum type)
{	
	static const ers::StringView type_shader[] =
	{
		ers::StringView("vertex"),
		ers::StringView("fragment"),
		ers::StringView("geometry")
	};

	GLuint shader = glCreateShader(type); GL_CHECK();
	glShaderSource(shader, 1, &shader_source, nullptr); GL_CHECK();
	glCompileShader(shader); GL_CHECK();

	GLint success;
	GLchar info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success); GL_CHECK();
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, info_log); GL_CHECK();
		Shader_Type type_shader_index;
		switch (type) 
		{
		case GL_VERTEX_SHADER:
			type_shader_index = Shader_Type::VERTEX;
			break;
		case GL_FRAGMENT_SHADER:
			type_shader_index = Shader_Type::FRAGMENT;
			break;
#ifndef __EMSCRIPTEN__
		case GL_GEOMETRY_SHADER:
			type_shader_index = Shader_Type::GEOMETRY;
			break;
#endif
		default: {
			ERS_UNUSED(type_shader_index);
			ERS_ASSERTF(0 != 0, "%s", "Unreachable.");
		}		
		}		

		ERS_PANICF(
			false, 
			"[ERROR] Shader::compileShader:\nFailed to compile %.*s shader.\n%s\n", 
			(int)type_shader[type_shader_index].size, 
			type_shader[type_shader_index].data, 
			info_log
		);
	}

	return shader;
}