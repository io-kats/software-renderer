#ifndef SHADOWMAP_SHADER_H
#define SHADOWMAP_SHADER_H

#include "shader_program.h"
#include "ers/matrix.h"
#include "image.h"

class ShadowmapShader : public IShaderProgram
{

// ERS_SHADER_DEFINE_VARYINGS({ ers::vec3 fragpos; })
    
public:
    ers::vec3 uniform_light_pos;
    ers::mat4 uniform_lightspace_mat;
    ers::mat4 uniform_model;
    f32 uniform_zFar;

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
        //m_vars[which_vert].fragpos = ers::vec3(uniform_model * ers::vec4(vert->aPos, 1.0f));		

		const ers::vec4 position = uniform_lightspace_mat * uniform_model * ers::vec4(vert->aPos, 1.0f);       
        return position;
    }


    bool FragmentShader(ers::vec4& out) override
    {            
        // const Varyings& vars = m_varsIntepolated; 	
        // const f32 distance = ers::length(vars.fragpos - uniform_light_pos) / uniform_zFar;
        out = ers::vec4(1.0f);     
        return false;
    }  
};

#endif // SHADOWMAP_SHADER_H