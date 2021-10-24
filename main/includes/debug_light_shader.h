#ifndef DEBUG_LIGHT_SHADER_H
#define DEBUG_LIGHT_SHADER_H

#include "shader_program.h"
#include "ers/matrix.h"
#include "image.h"

class DebugLightShader : public IShaderProgram
{

ERS_SHADER_DEFINE_VARYINGS(m_vars, m_varsInterpolated, { ers::vec3 fragpos; })

public:
    ers::mat4 uniform_mvp_mat;
    ers::mat4 uniform_model;
    ers::vec3 uniform_color;
    ers::vec3 uniform_light_pos;
    f32 uniform_scale;
    bool uniform_wireframe;

    void VertexShader(
        const void* in0, const void* in1, const void* in2,
        ers::vec4& p0, ers::vec4& p1, ers::vec4& p2        
    ) override
    {
        p0 = VertexShaderPerVertex(in0, 0);
		p1 = VertexShaderPerVertex(in1, 1);
		p2 = VertexShaderPerVertex(in2, 2);
    }

    ers::vec4 VertexShaderPerVertex(const void* in, s32 which_vert)
    {       
        const VertexAttributes1* vert = (const VertexAttributes1*)in;
        m_vars[which_vert].fragpos = ers::vec3(uniform_model * ers::vec4(vert->aPos, 1.0f));		

		const ers::vec4 position = uniform_mvp_mat * ers::vec4(vert->aPos, 1.0f);       
        return position;
    }

    bool FragmentShader(ers::vec4& out) override
    {        
        const Varyings& vars = m_varsInterpolated; 	
        const f32 dist = ers::length(vars.fragpos - uniform_light_pos);
        f32 t = 1.0f - ers::smoothstep(uniform_scale * 0.6f, uniform_scale * 0.90f, dist);

        f32 s = 0.0f;
        if (uniform_wireframe && (m_bar.x() < 0.1f || m_bar.y() < 0.1f || m_bar.z() < 0.1f))
        {
            t = 0.0f;
            s = 1.0f;
        }
        out = ers::vec4(t * uniform_color + s * (1.0f - t) * (ers::vec3(1.0f) - uniform_color), 1.0f);     
        return false;
    }  
};

#endif // DEBUG_LIGHT_SHADER_H