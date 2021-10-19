#ifndef SOFTWARE_RENDERER_H
#define SOFTWARE_RENDERER_H

#include "ers/typedefs.h"
#include "ers/macros.h"
#include "ers/common.h"
#include "ers/vec.h"
#include "ers/allocators.h"
#include "image.h"
#include "shader_program.h"

#define ERS_RENDERER_EPSILON 5.0e-5f
#define ERS_RENDERER_MAX_WIDTH 2048
#define ERS_RENDERER_MAX_HEIGHT 2048

class Renderer
{
public:
    enum State
    {
        DEFAULT = 0,
        CULL_FACE = 1 << 0,
        WIREFRAME = 1 << 1,
        DEPTH_TEST = 1 << 2
    };

    Renderer(s32 width, s32 height, ers::IAllocator* alloc = &ers::default_alloc);
    ~Renderer();

    void Enable(State state);
    void Disable(State state);
    void Toggle(State state);
    bool IsEnabled(State state);

    void SetPixel(s32 x, s32 y, ers::vec4& color);
    void SetZValue(s32 x, s32 y, f32 z_val);
    f32 GetZValue(s32 x, s32 y);

    void SetViewport(s32 width, s32 height);
    void SetShaderProgram(IShaderProgram* shader);
    void Clear(f32 r = 0.0f, f32 g = 0.0f, f32 b = 0.0f, f32 a = 1.0f);

    u8* GetColorBuffer();
    f32* GetZBuffer();
    s32 GetWidth();
    s32 GetHeight();
    const ers::vec4* GetNdcVertices();
    
    void ProcessTriangle(const void* in0, const void* in1, const void* in2);  
    void WriteToFile(const char* filename, bool flip = true);

private:
    struct Bbox
    {
        s32 x_min; s32 y_min;
        s32 x_max; s32 y_max;
    };

    struct NdcTriCoords
    {
        s32 x0, y0;
        s32 x1, y1;
        s32 x2, y2;
        s32 surface;
        ers::ivec2 d01;
        ers::ivec2 d12;
        ers::ivec2 d20;
    };

    ers::vec4 m_ndcTri[6];
    
    s32 m_width;
    s32 m_height;
    u8* m_colorBuffer;
    f32* m_zBuffer;
    u32 m_state;
    ers::IAllocator* m_alloc;

    IShaderProgram* m_shader;

    void clipTriangle(s32& count_tris);
    void rasterizeTriangle(s32 tri_idx);   

    void lerpVaryings(f32* out, f32* in1, f32* in2, f32 t, s32 count);
    void clipGetOneTriangle(ers::vec4& p0, ers::vec4& p1, ers::vec4& p2, f32& t0, f32& t1);
    void clipGetTwoTriangles(
        ers::vec4& p0, 
        ers::vec4& p1, 
        ers::vec4& p2,
        ers::vec4& q0, 
        ers::vec4& q1, 
        ers::vec4& q2,
        f32& t0,
        f32& t1
    );
    void normalizeCoordinates(ers::vec4& p);

    NdcTriCoords getNdcTriCoords(ers::vec4& p0, ers::vec4& p1, ers::vec4& p2);
    Bbox getTriangleBoundingBox(const NdcTriCoords& tri);
    ers::ivec3 getWeights0(const NdcTriCoords& tri, s32 x0, s32 y0);
};

#endif // SOFTWARE_RENDERER_H