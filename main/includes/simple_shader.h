#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "shader_program.h"
#include "ers/vec.h"

class SimpleShader : public IShaderProgram
{

ERS_SHADER_DEFINE_VARYINGS(m_vars, m_varsInterpolated, { ers::vec3 color; })

public:
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
        const VertexAttributes3* vert = (const VertexAttributes3*)in;
        m_vars[which_vert].color = vert->aColor;		

		const ers::vec4 position = ers::vec4(vert->aPos, 1.0f);       
        return position;
    }

    bool FragmentShader(ers::vec4& out) override
    {        
        out = ers::vec4(m_varsInterpolated.color, 1.0f);     
        return false;
    }  
};

#endif // SIMPLE_SHADER_H