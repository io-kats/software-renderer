#include "software_renderer.h"
#include "stb_image_write.h"

Renderer::Renderer(int width, int height, ers::IAllocator* alloc)
    : 
    m_width(width),
    m_height(height),
    m_colorBuffer(nullptr),
    m_zBuffer(nullptr),
    m_state(State::DEFAULT),
    m_alloc(alloc),
    m_shader(nullptr)
{
    ERS_ASSERT(width >= 2 && width <= ERS_RENDERER_MAX_WIDTH);
    ERS_ASSERT(height >= 2 && height <= ERS_RENDERER_MAX_HEIGHT);
    m_colorBuffer = (u8*)m_alloc->Allocate(sizeof(u8) * ERS_RENDERER_MAX_WIDTH * ERS_RENDERER_MAX_HEIGHT * 4, alignof(u8)); 
    m_zBuffer = (f32*)m_alloc->Allocate(sizeof(f32) * ERS_RENDERER_MAX_WIDTH * ERS_RENDERER_MAX_HEIGHT, alignof(f32));     
    Clear(); 
}

Renderer::~Renderer()
{
    m_alloc->Deallocate(m_colorBuffer);
    m_alloc->Deallocate(m_zBuffer);    
}

void Renderer::Enable(State state)
{
    m_state |= state;
}

void Renderer::Disable(State state)
{
    m_state &= ~state;
}

void Renderer::Toggle(State state)
{
    m_state ^= state;
}

bool Renderer::IsEnabled(State state)
{
    return (m_state & state) > 0;
}

void Renderer::SetPixel(s32 x, s32 y, ers::vec4& color)
{
    ERS_ASSERT(x >= 0 && x < m_width);
    ERS_ASSERT(y >= 0 && y < m_height);
    size_t position = 4 * (m_width * y + x);
    m_colorBuffer[position] = (u8)(ers::clamp(color.x(), 0.0f, 1.0f) * 255.999f);
    m_colorBuffer[position + 1] = (u8)(ers::clamp(color.y(), 0.0f, 1.0f) * 255.999f);
    m_colorBuffer[position + 2] = (u8)(ers::clamp(color.z(), 0.0f, 1.0f) * 255.999f);
    m_colorBuffer[position + 3] = (u8)(ers::clamp(color.w(), 0.0f, 1.0f) * 255.999f);
}

void Renderer::SetZValue(s32 x, s32 y, f32 z_val)
{
    ERS_ASSERT(x >= 0 && x < m_width);
    ERS_ASSERT(y >= 0 && y < m_height);
    m_zBuffer[y * m_width + x] = z_val;
}

f32 Renderer::GetZValue(s32 x, s32 y)
{
    ERS_ASSERT(x >= 0 && x < m_width);
    ERS_ASSERT(y >= 0 && y < m_height);
    return m_zBuffer[y * m_width + x];
}

void Renderer::SetViewport(s32 width, s32 height)
{ 
    ERS_ASSERT(width >= 2 && width <= ERS_RENDERER_MAX_WIDTH);
    ERS_ASSERT(height >= 2 && height <= ERS_RENDERER_MAX_HEIGHT);
    m_width = width; 
    m_height = height; 
}

void Renderer::SetShaderProgram(IShaderProgram* shader)
{
    m_shader = shader;
}

u8* Renderer::GetColorBuffer()
{
    return m_colorBuffer;
}

f32* Renderer::GetZBuffer()
{
    return m_zBuffer;
}

s32 Renderer::GetWidth()
{
    return m_width;
}

s32 Renderer::GetHeight()
{
    return m_height;
}

const ers::vec4* Renderer::GetNdcVertices()
{
    return &m_ndcTri[0];
}

void Renderer::Clear(f32 r, f32 g, f32 b, f32 a)
{
    for (s32 i = 0; i < m_width * m_height; ++i)
    {
        size_t position = 4 * i;
        m_colorBuffer[position] = (u8)(r * 255.999f);
        m_colorBuffer[position + 1] = (u8)(g * 255.999f);
        m_colorBuffer[position + 2] = (u8)(b * 255.999f);
        m_colorBuffer[position + 3] = (u8)(a * 255.999f);
    }
    for (s32 i = 0; i < m_width * m_height; ++i) m_zBuffer[i] = FLT_MAX;
}

void Renderer::RenderTriangle(const void* in0, const void* in1, const void* in2)
{
    ERS_ASSERT(m_shader != nullptr);
    m_shader->VertexShader(in0, in1, in2, m_ndcTri[0], m_ndcTri[1], m_ndcTri[2]);
    s32 count_tris_after_clipping;
    clipTriangle(count_tris_after_clipping);
    for (s32 tri_idx = 0; tri_idx < count_tris_after_clipping; ++tri_idx) 
        rasterizeTriangle(tri_idx);
}

void Renderer::clipTriangle(s32& count_tris)
{
    bool should_clip = true;
    count_tris = -1; 

    ers::vec4& p0 = m_ndcTri[0];
    ers::vec4& p1 = m_ndcTri[1];
    ers::vec4& p2 = m_ndcTri[2];
    
    // Lazy early discarding of triangles that are completely behind (one of) the clip planes.
    // TODO: Actual full frustum culling.
    if (
        (p0.x() < -p0.w() && p1.x() < -p1.w() && p2.x() < -p2.w())
     || (p0.x() >  p0.w() && p1.x() >  p1.w() && p2.x() >  p2.w())
     || (p0.y() < -p0.w() && p1.y() < -p1.w() && p2.y() < -p2.w())
     || (p0.y() >  p0.w() && p1.y() >  p1.w() && p2.y() >  p2.w())
     || (p0.z() < -p0.w() && p1.z() < -p1.w() && p2.z() < -p2.w())
     || (p0.z() >  p0.w() && p1.z() >  p1.w() && p2.z() >  p2.w())
    ) 
    {
        should_clip = false;
        count_tris = 0;
    }
    
    // No need to clip if in front of z plane, just discard fragments/triangles in the rasterizer.
    if (should_clip && p0.z() > -p0.w() && p1.z() > -p1.w() && p2.z() > -p2.w()) 
    {
        should_clip = false;
        count_tris = 1;
    }

    // At least one vertex is behind the near z-plane, so we clip the triangle.
    if (should_clip) 
    {
        bool is_behind[3] = { false };
        s32 how_many_behind = 0;
        for (s32 i = 0; i < 3; ++i)
        {
            if (m_ndcTri[i].z() < -m_ndcTri[i].w()) 
            { 
                is_behind[i] = true; 
                ++how_many_behind; 
            }
        }
        ERS_ASSERT(how_many_behind == 1 || how_many_behind == 2);

        VaryingsInfo vars_info = m_shader->GetVaryingsInfo();
        const s32 vars_count = vars_info.count;
        s32 idx0, idx1, idx2;    
        if (how_many_behind == 1) // one point behind clip-plane = quad case/2 new triangles
        {                 
            if (is_behind[0])
            {               
                idx0 = 0; idx1 = 1; idx2 = 2;
            }
            else if (is_behind[1])
            {
                idx0 = 1; idx1 = 2; idx2 = 0;
            }
            else if (is_behind[2])
            {                
                idx0 = 2; idx1 = 0; idx2 = 1;
            }

            if (vars_info.data != nullptr)
            {             
                f32 t0, t1; 
                clipGetTwoTriangles(m_ndcTri[idx0], m_ndcTri[idx1], m_ndcTri[idx2], m_ndcTri[3], m_ndcTri[4], m_ndcTri[5], t0, t1);
                f32* vars00 = vars_info.GetVars(0, idx0);
                f32* vars01 = vars_info.GetVars(0, idx1);
                f32* vars02 = vars_info.GetVars(0, idx2);
                f32* vars10 = vars_info.GetVars(1, 0);
                lerpVaryings(vars10, vars00, vars01, t1, vars_count);
                lerpVaryings(vars00, vars00, vars02, t0, vars_count);
                vars_info.SetSecondTriangleIndices(3, idx1, idx0);
            }
        }
        else if (how_many_behind == 2) // two points behind clip-plane = 1 new triangle
        {         
            if (is_behind[0] && is_behind[1])
            {  
                idx0 = 0; idx1 = 1; idx2 = 2;
            }
            else if (is_behind[1] && is_behind[2])
            {
                idx0 = 1; idx1 = 2; idx2 = 0;
            }
            else if (is_behind[2] && is_behind[0])
            {
                idx0 = 2; idx1 = 0; idx2 = 1;
            }

            if (vars_info.data != nullptr)
            {
                f32 t0, t1;
                clipGetOneTriangle(m_ndcTri[idx0], m_ndcTri[idx1], m_ndcTri[idx2], t0, t1);
                f32* vars00 = vars_info.GetVars(0, idx0);
                f32* vars01 = vars_info.GetVars(0, idx1);
                f32* vars02 = vars_info.GetVars(0, idx2);
                lerpVaryings(vars00, vars00, vars02, t0, vars_count);
                lerpVaryings(vars01, vars01, vars02, t1, vars_count);
            }
        }

        count_tris = (how_many_behind == 2) ? 1 : 2;
    }
}

void Renderer::rasterizeTriangle(s32 tri_idx)
{   
    s32 base = 3 * tri_idx;
    ers::vec4& p0 = m_ndcTri[base];
    ers::vec4& p1 = m_ndcTri[base + 1];
    ers::vec4& p2 = m_ndcTri[base + 2];
    normalizeCoordinates(p0);
    normalizeCoordinates(p1);
    normalizeCoordinates(p2);        

    NdcTriCoords tri = getNdcTriCoords(p0, p1, p2);

    if (tri.surface == 0) return; // degenerate triangle. Ignore.
    if (IsEnabled(CULL_FACE) && tri.surface < 0) return; // Backface culling.
    
    Bbox bbox = getTriangleBoundingBox(tri);

    // ******************************************************
    // Rasterizing kernel. Should be easily(?) parallelizable.
    // TODO: Parallelize it!

    // Initialize barycentric coordinates at the center of the 1st position in the bounding box, 
    // normalized to double the above calculated signed surface (4 times the triangle surface),...    
    ers::ivec3 weights0 = getWeights0(tri, bbox.x_min, bbox.y_min);

    // ...and calculate step values, for rows and columns respectively.  
    const ers::ivec3 wstepx(2 * tri.d12.y(), 2 * tri.d20.y(), 2 * tri.d01.y());
    const ers::ivec3 wstepy(2 * tri.d12.x(), 2 * tri.d20.x(), 2 * tri.d01.x());
 
    const f32 tri_surface_inv = 1.0f / (f32)(2 * tri.surface); 
    ers::ivec3 weights;
    for (s32 y = bbox.y_min; y <= bbox.y_max; ++y)
    {
        weights = weights0;       
        for (s32 x = bbox.x_min; x <= bbox.x_max; ++x)
        {
            // Check if point is in triangle.
            // Calculate normalized barycentric coordinates... 
            ers::vec3 bar;
            bar.x() = (f32)weights.x() * tri_surface_inv;
            bar.y() = (f32)weights.y() * tri_surface_inv;
            bar.z() = (f32)weights.z() * tri_surface_inv;
            
            // Increment the barycentric coordinates before possibly leaving the inner loop.
            weights += wstepx;			      

            // Negative barycentric coordinates <=> point is outside triangle.
            if (bar.x() < -ERS_RENDERER_EPSILON || bar.y() < -ERS_RENDERER_EPSILON || bar.z() < -ERS_RENDERER_EPSILON) 
                continue;

            ers::vec3 bar_correct; // ... also calculate the perspective correct barycentric coordinates.
            bar_correct.x() = bar.x() * p0.w();
            bar_correct.y() = bar.y() * p1.w();
            bar_correct.z() = bar.z() * p2.w();
            bar_correct /= (bar_correct.x() + bar_correct.y() + bar_correct.z());
       
            // Low effort wireframe.
            if (IsEnabled(WIREFRAME) && bar_correct.y() > 0.01f && bar_correct.z() > 0.01f && bar_correct.x() > 0.01f) continue;
          
            // Interpolate the z coordinate (in ndc-space). 
            const f32 z_curr = bar.x() * p0.z() + bar.y() * p1.z() + bar.z() * p2.z();

            // Clip in the z-axis. We already clip against the near z-plane so we don't need z_curr < -1.0f, 
            // but leaving it in in case I mess around with clipping again.
            if (z_curr < -1.0f || z_curr > 1.0f) continue; 

            f32 buf_z = GetZValue(x, y);
            if (!IsEnabled(DEPTH_TEST) || (z_curr <= buf_z)) // early depth test. more negative z is "in front".
            {              
                ers::vec4 col;               
                m_shader->InterpolateVaryings(bar, bar_correct, tri_idx);                     	
                bool discard = m_shader->FragmentShader(col);           
                if (!discard)
                {                  
                    SetPixel(x, y, col);                   
                    SetZValue(x, y, z_curr);
                }
            }           
        }
        weights0 += wstepy;
    }
}

Renderer::NdcTriCoords Renderer::getNdcTriCoords(ers::vec4& p0, ers::vec4& p1, ers::vec4& p2)
{
    NdcTriCoords tri;

    // Normalize to screen/image coordinates. [-1, 1] -> [0, 1] -> [0, width-1 or height-1]
    const f32 w = (f32)m_width - 0.001f;
    const f32 h = (f32)m_height - 0.001f;

    tri.x0 = (s32)((0.5f + 0.5f * p0.x()) * w);
    tri.y0 = (s32)((0.5f + 0.5f * p0.y()) * h);

    tri.x1 = (s32)((0.5f + 0.5f * p1.x()) * w);
    tri.y1 = (s32)((0.5f + 0.5f * p1.y()) * h);

    tri.x2 = (s32)((0.5f + 0.5f * p2.x()) * w);
    tri.y2 = (s32)((0.5f + 0.5f * p2.y()) * h);

    tri.d01 = ers::ivec2(tri.x1 - tri.x0, tri.y0 - tri.y1); // <- opposite sign for the y coordinates of the difference vectors to avoid minus-ing them later.
    tri.d12 = ers::ivec2(tri.x2 - tri.x1, tri.y1 - tri.y2);
    tri.d20 = ers::ivec2(tri.x0 - tri.x2, tri.y2 - tri.y0);

    // Calculate signed surface of triangle (actually two times that) for backface culling, barycentric coordinates and checking for degeneracy.
    tri.surface = tri.d01.y() * tri.x2 + tri.d01.x() * tri.y2 + tri.x0 * tri.y1 - tri.y0 * tri.x1;

    return tri;
}

Renderer::Bbox Renderer::getTriangleBoundingBox(const NdcTriCoords& tri)
{
    Bbox bbox;

    // Find bounding box.
    bbox.x_min = tri.x0; 
    bbox.y_min = tri.y0;

    bbox.x_max = tri.x0; 
    bbox.y_max = tri.y0;

    if (tri.x1 > bbox.x_max) bbox.x_max = tri.x1;
    if (tri.y1 > bbox.y_max) bbox.y_max = tri.y1;
    if (tri.x1 < bbox.x_min) bbox.x_min = tri.x1;
    if (tri.y1 < bbox.y_min) bbox.y_min = tri.y1;

    if (tri.x2 > bbox.x_max) bbox.x_max = tri.x2;
    if (tri.y2 > bbox.y_max) bbox.y_max = tri.y2;
    if (tri.x2 < bbox.x_min) bbox.x_min = tri.x2;
    if (tri.y2 < bbox.y_min) bbox.y_min = tri.y2;

    // Clip in the x- and y-axes.
    bbox.x_min = ers::clamp(bbox.x_min, 0, m_width - 1);
    bbox.x_max = ers::clamp(bbox.x_max, 0, m_width - 1);

    bbox.y_min = ers::clamp(bbox.y_min, 0, m_height - 1);
    bbox.y_max = ers::clamp(bbox.y_max, 0, m_height - 1);

    return bbox;
}

ers::ivec3 Renderer::getWeights0(const NdcTriCoords& tri, s32 x0, s32 y0)
{
    ers::ivec3 weights0(
        2 * (tri.d12.y() * x0 + tri.d12.x() * y0 + tri.y2 * tri.x1 - tri.x2 * tri.y1) + tri.d12.x() + tri.d12.y(), 
        2 * (tri.d20.y() * x0 + tri.d20.x() * y0 + tri.y0 * tri.x2 - tri.x0 * tri.y2) + tri.d20.x() + tri.d20.y(), 
        2 * (tri.d01.y() * x0 + tri.d01.x() * y0 + tri.y1 * tri.x0 - tri.x1 * tri.y0) + tri.d01.x() + tri.d01.y() 
    );
    return weights0;
}

void Renderer::WriteToFile(const char* filename, bool flip)
{
	stbi_flip_vertically_on_write(flip);
	s32 rc = stbi_write_png(
        filename, 
        m_width, 
        m_height, 
        4, 
        reinterpret_cast<const void*>(m_colorBuffer), 
        4 * m_width
    );
	ERS_ASSERT(rc != 0);
}

void Renderer::lerpVaryings(f32* out, f32* in1, f32* in2, f32 t, s32 count)
{
    const f32 tm = 1.0f - t;
    for (s32 i = 0; i < count; ++i)
        out[i] = tm * in1[i] + t * in2[i];
}

void Renderer::clipGetOneTriangle(
    ers::vec4& p0, 
    ers::vec4& p1, 
    ers::vec4& p2, // <-- is inside
    f32& t0,
    f32& t1)
{
    t0 = (-p0.w() - p0.z()) / (p2.w() + p2.z() - p0.w() - p0.z());
    t1 = (-p1.w() - p1.z()) / (p2.w() + p2.z() - p1.w() - p1.z());
    p0 = (1.0f - t0) * p0 + t0 * p2;
    p1 = (1.0f - t1) * p1 + t1 * p2;
}

void Renderer::clipGetTwoTriangles(
    ers::vec4& p0, 
    ers::vec4& p1, 
    ers::vec4& p2,
    ers::vec4& q0, 
    ers::vec4& q1, 
    ers::vec4& q2,
    f32& t0,
    f32& t1)
{
    t0 = (-p0.w() - p0.z()) / (p2.w() + p2.z() - p0.w() - p0.z());
    t1 = (-p0.w() - p0.z()) / (p1.w() + p1.z() - p0.w() - p0.z());
    q0 = (1.0f - t1) * p0 + t1 * p1;
    p0 = (1.0f - t0) * p0 + t0 * p2;
    q1 = p1;
    q2 = p0;
}

void Renderer::normalizeCoordinates(ers::vec4& p)
{
    p.w() = 1.0f / p.w();
    p.x() *= p.w();
    p.y() *= p.w();
    p.z() *= p.w();
}