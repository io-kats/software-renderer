#ifndef GL_SURFACE_H
#define GL_SURFACE_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "ers/common.h"
#include "ers/matrix.h"
#include "gl_shader_program.h"

// Simple class for creating a "surface" for real-time software rendering
// by making use of a texture.
// Always has 4 compontents (RGBA).
class GLSurface
{
public:
    GLSurface(s32 width, s32 height);
    ~GLSurface();

    // "data" is a pointer to a texture to be rendered. 
    // Assummes it has 4 components, just like the underlying texture.
    void Draw(u8* data) const;
    void Draw(u8* data, s32 xpos, s32 ypos, s32 width, s32 height) const;
    void Resize(s32 new_width, s32 new_height);

private:
    void generateVertexBuffers();
    void generateTexture(s32 width, s32 height);
    void createProgram();

    GLsizei m_width, m_height;

    GLuint m_vbo, m_vao, m_ebo;
	GLuint m_texId;
	GLShaderProgram m_shader;
};

#endif // GL_SURFACE_H