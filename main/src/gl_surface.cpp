#include "gl_surface.h"

GLSurface::GLSurface(s32 width, s32 height)
{
    generateVertexBuffers();
    generateTexture(width, height);
    createProgram();    
}

GLSurface::~GLSurface()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteTextures(1, &m_texId);
    m_shader.Destroy();   
}

void GLSurface::Draw(u8* data) const
{
    Draw(data, 0, 0, m_width, m_height);
}

void GLSurface::Draw(u8* data, s32 xpos, s32 ypos, s32 width, s32 height) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data); 
        
    m_shader.Use();
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GLSurface::Resize(s32 new_width, s32 new_height)
{
    glDeleteTextures(1, &m_texId);
    m_texId = 0;
    generateTexture(new_width, new_height);
}

void GLSurface::generateVertexBuffers()
{
    // Quad vertex attributes.
    GLfloat vertex_attributes[] = // x,y pos + texture coords
    {
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
    };

    GLuint indices[] = // triangles
    {
        0, 1, 2,
        2, 3, 0,
    };

    // Buffers.
    glGenVertexArrays(1, &m_vao); GL_CHECK();
    glGenBuffers(1, &m_vbo); GL_CHECK();
    glGenBuffers(1, &m_ebo); GL_CHECK();

    glBindVertexArray(m_vao); GL_CHECK();
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo); GL_CHECK();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_attributes), vertex_attributes, GL_STATIC_DRAW); GL_CHECK();	
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo); GL_CHECK();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); GL_CHECK();	

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0); GL_CHECK();
    glEnableVertexAttribArray(0); GL_CHECK();		

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat))); GL_CHECK();
    glEnableVertexAttribArray(1); GL_CHECK();
}

void GLSurface::generateTexture(s32 width, s32 height)
{
    m_width = GLsizei(width);
    m_height = GLsizei(height);

    glGenTextures(1, &m_texId); GL_CHECK();
    glBindTexture(GL_TEXTURE_2D, m_texId); GL_CHECK();
    glPixelStorei(GL_PACK_ALIGNMENT, 1); GL_CHECK();	
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); GL_CHECK();	
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); GL_CHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); GL_CHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GL_CHECK();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GL_CHECK();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); GL_CHECK();
}

void GLSurface::createProgram()
{
    // Simple shader program to render the quad, onto which the software renderer framebuffer gets rendered.
    const char* vertex_shader_source =    
#ifdef __EMSCRIPTEN__
    "#version 300 es\n"
#else
    "#version 330 core\n"
#endif
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoords;\n"
    "out vec2 tex_coords;\n"
    "uniform mat4 proj;\n"
    "void main()\n"
    "{\n"
    "	tex_coords = aTexCoords;\n"
    "	gl_Position = proj * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

    const char* fragment_shader_source = 
#ifdef __EMSCRIPTEN__
    "#version 300 es\n"
    "precision mediump float;\n"
#else
    "#version 330 core\n"
#endif
    "in vec2 tex_coords;\n"
    "out vec4 final_color;\n"
    "uniform sampler2D image;\n"
    "void main()\n"
    "{\n"
    "	final_color = texture(image, tex_coords);\n"
    "}\n";

    m_shader.LoadFromSource(vertex_shader_source, nullptr, fragment_shader_source);
    m_shader.Use();
    m_shader.Set("proj", ers::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
    m_shader.Set("image", 0);
}