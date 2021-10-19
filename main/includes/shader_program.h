#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "ers/typedefs.h"
#include "ers/macros.h"
#include "ers/common.h"
#include "ers/vec.h"
#include <type_traits>

// Helper struct used for interpolating varyings post clipping and automating barycentric intepolation in the interface/base class.
// Not meant to be public.
struct VaryingsInfo
{
    f32* data; // pointer to the first float in the Varyings structure of a shader.
    f32* data_interpolated; // pointer to the first float in the Varyings structure of a shader, for communicating with the basic class.
    s32* indices; // pointer to array containing the indices defining which vertices make up which tringle.
    s32 count; // size of the Varyings struct in multiples of sizeof(float), i.e. how many floating number are contained in a Varyings struct.

    // Returns pointer to the 1st floating point element for the varyings of a vertex.
    // @param triangle: triangle index, it can be either 0 for the 1st triangle or 1 for the 2nd.
    // @param vert: vertex index, it can be 0, 1 or 2
    f32* GetVars(s32 triangle, s32 vert)
    {
        ERS_ASSERT(triangle == 0 || triangle == 1);
        ERS_ASSERT(vert == 0 || vert == 1 || vert == 2);
        return data + indices[3 * triangle + vert] * count;
    }

    void SetSecondTriangleIndices(s32 idx0, s32 idx1, s32 idx2)
    {
        indices[3] = idx0; indices[4] = idx1; indices[5] = idx2;
    }
};

struct VertexAttributes1
{       
    ers::vec3 aPos;
    ers::vec3 aNormal;
    ers::vec2 aTexcoord;
};

struct VertexAttributes2
{       
    ers::vec2 aPos;
    ers::vec2 aTexcoord;
};

struct VertexAttributes3
{       
    ers::vec3 aPos;
    ers::vec3 aColor;
};

// Interface/base class for shader programs for the software renderer.
class IShaderProgram 
{
public:
    ers::vec3 m_barNoPerspective;
    ers::vec3 m_bar;
    ers::vec4 m_ndcTri[3];
    
    // Calculates the current triangle's normal device coordinates.
    // @param in0, in1, in2: pointers to structures containing the vertex attributes, e.g. position, normal, texture coordinates, color etc. 
    // See above for examples VertexAttributes1. 
    // @param p0, p1, p2: outputs for the current triangle's normal device coordinates calculated in the vertex shader.
    virtual void VertexShader(const void* in0, const void* in1, const void* in2, ers::vec4& p0, ers::vec4& p1, ers::vec4& p2) 
    { 
        ERS_UNUSED(in0); 
        ERS_UNUSED(in1); 
        ERS_UNUSED(in2); 
        ERS_UNUSED(p0); 
        ERS_UNUSED(p1); 
        ERS_UNUSED(p2); 
    } 

    // Calculates the fragment's color.
    // @param out: the color calculated in the fragment shader.
    virtual bool FragmentShader(ers::vec4& out) { ERS_UNUSED(out); return false; } 

    // Produces a helper struct containing pointers to and the size of the shaders Varyings struct.
    virtual VaryingsInfo GetVaryingsInfo() { return { nullptr, nullptr, nullptr, 0 }; }

    // Performs perspective correct interpolation of the varyings in the shader.
    void InterpolateVaryings(const ers::vec3& bar_coords, const ers::vec3& bar_coords_correct, s32 tri_idx_after_clipping) 
    {         
        m_barNoPerspective = bar_coords;
        m_bar = bar_coords_correct;

        VaryingsInfo vars_info = GetVaryingsInfo();
        if (vars_info.data != nullptr)
        {
            f32* vars0 = vars_info.GetVars(tri_idx_after_clipping, 0);
            f32* vars1 = vars_info.GetVars(tri_idx_after_clipping, 1);
            f32* vars2 = vars_info.GetVars(tri_idx_after_clipping, 2);
            f32* vars_interpolated = vars_info.data_interpolated;
            
            const s32 count = vars_info.count;
            for (s32 i = 0; i < count; ++i)
            {
                vars_interpolated[i] = m_bar.x() * vars0[i] + m_bar.y() * vars1[i] + m_bar.z() * vars2[i];
            }
        }       
    }   
};

// Helper macro for defining varyings correctly for the shader programs that inherit the above class.
// Note: We use an array of 4 varying structs instead of 3 or 6, since we can only ever produce one new vertex when clipping.
#define ERS_SHADER_DEFINE_VARYINGS(varyings_name_per_vertex, varyings_name_interpolated, definition) \
public: \
struct Varyings definition; \
static_assert(std::is_trivially_copyable<Varyings>::value == true, "Varyings are not trivially copyable."); \
static_assert(sizeof(Varyings) % sizeof(float) == 0, "sizeof(Varyings) isn't a multiple of 4. \nNOTE: Varyings are interpreted as being bags of floats."); \
private:  \
    Varyings varyings_name_per_vertex[4]; \
    Varyings varyings_name_interpolated; \
    s32 vars_indices[6] = { 0, 1, 2, 3, 4, 5 }; \
public: \
    VaryingsInfo GetVaryingsInfo() override  \
    {  \
        VaryingsInfo result; \
        result.data = reinterpret_cast<f32*>(&((varyings_name_per_vertex)[0])); \
        result.data_interpolated = reinterpret_cast<f32*>(&(varyings_name_interpolated)); \
        result.indices = reinterpret_cast<s32*>(&((vars_indices)[0])); \
        result.count = sizeof(Varyings) / sizeof(f32); \
        return result;  \
    } \

#endif // SHADER_PROGRAM_H