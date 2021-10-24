#ifndef BLINNPHONG_SHADER_H
#define BLINNPHONG_SHADER_H

#include "shader_program.h"
#include "ers/matrix.h"
#include "image.h"

class BlinnPhongShader : public IShaderProgram
{

ERS_SHADER_DEFINE_VARYINGS
(m_vars, m_varsInterpolated, 
{       
    ers::vec3 fragpos;
    ers::vec3 normal;
    ers::vec2 texcoord;
    ers::vec4 lightspace_fragpos;
})

private:    
    ers::vec3 m_randomColor;
    ers::vec3 m_d01;
    ers::vec3 m_d02;
    ers::vec3 m_du;
    ers::vec3 m_dv;
    
public:
    ers::vec3 uniform_light_pos;
    ers::vec3 uniform_light_dir;
    ers::vec3 uniform_view_pos;
    ers::vec3 uniform_color;

    Image* sampler2d_diffuse_map;
    Image* sampler2d_normal_map;
    Image* sampler2d_specular_map;
    Image* sampler2d_shadow_map;

    ers::mat4 uniform_mvp_mat;
    ers::mat4 uniform_model;
    ers::mat3 uniform_model_it;
    ers::mat4 uniform_lightspace_mat;

    f32 uniform_zFar;

    bool uniform_do_random_color;
    bool uniform_do_specific_color;
    bool uniform_do_point_light;
    const f32 shadow_bias = 0.05f;

    f32 calculate_shadow_value(const ers::vec4& lightspace_fragpos)
    {
        f32 shadow_value = 0.0f;
        if (sampler2d_shadow_map != nullptr)
        {
            ers::vec3 shadow_ndc(lightspace_fragpos / lightspace_fragpos.w());
            shadow_ndc.x() = 0.5f * shadow_ndc.x() + 0.5f;
            shadow_ndc.y() = 0.5f * shadow_ndc.y() + 0.5f;

            f32 dist;
            sampler2d_shadow_map->Get(shadow_ndc.x(), shadow_ndc.y(), dist);

            const f32 closest_depth = dist;
            const f32 current_depth = shadow_ndc.z();
            shadow_value = (current_depth - shadow_bias > closest_depth) ? 1.0f : 0.0f;
        }
        return shadow_value;
    }

    ers::vec3 get_normal()
    {
        const ers::vec3 n = ers::normalize(m_varsInterpolated.normal);

        ers::vec3 normal;
        if (sampler2d_normal_map == nullptr) 
        {
            normal = n;
        }
        else
        {
            const ers::mat3 Ainv = ers::inverse(ers::transpose(ers::mat3(m_d01, m_d02, n)));
            const ers::vec3 T = ers::normalize(Ainv * m_du);
            const ers::vec3 B = ers::normalize(Ainv * m_dv);
            const ers::mat3 tbn(T, B, n);
            sampler2d_normal_map->Get(m_varsInterpolated.texcoord.x(), m_varsInterpolated.texcoord.y(), normal);
            normal = 2.0f * normal - ers::vec3(1.0f);
            normal = ers::normalize(tbn * normal);
        } 	

        return normal;	
    }

    f32 get_shininess()
    {
        f32 shininess; 
        if (sampler2d_specular_map != nullptr)     
        {
            sampler2d_specular_map->Get(m_varsInterpolated.texcoord.x(), m_varsInterpolated.texcoord.y(), shininess);  
            shininess *= 255.0f;
        }
        else // default value
        {
            shininess = 32.0f;
        }
        return shininess;
    }

    ers::vec3 get_diffuse_sample()
    {
        ers::vec3 diffuse_sample;
        if (uniform_do_random_color) 
        {
            diffuse_sample = m_randomColor;
        }
        else if (uniform_do_specific_color || sampler2d_diffuse_map == nullptr)
        {
            diffuse_sample = uniform_color;
        }
        else
        {
            sampler2d_diffuse_map->Get(m_varsInterpolated.texcoord.x(), m_varsInterpolated.texcoord.y(), diffuse_sample);
        }
        return diffuse_sample;
    }

    ers::vec3 get_light_direction()
    {
        return (uniform_do_point_light) ? ers::normalize(uniform_light_pos - m_varsInterpolated.fragpos) : (-uniform_light_dir);
    }

    void VertexShader(
        const void* in0, const void* in1, const void* in2,
        ers::vec4& p0, ers::vec4& p1, ers::vec4& p2        
    ) override
    {
        p0 = VertexShaderPerVertex(in0, 0);
		p1 = VertexShaderPerVertex(in1, 1);
		p2 = VertexShaderPerVertex(in2, 2);

        m_randomColor = ers::vec3((f32)ers::random_frac(), (f32)ers::random_frac(), (f32)ers::random_frac());

        m_d01 = m_vars[1].fragpos - m_vars[0].fragpos;
        m_d02 = m_vars[2].fragpos - m_vars[0].fragpos;
        m_du = ers::vec3(m_vars[1].texcoord.x() - m_vars[0].texcoord.x(), m_vars[2].texcoord.x() - m_vars[0].texcoord.x(), 0.0f);
        m_dv = ers::vec3(m_vars[1].texcoord.y() - m_vars[0].texcoord.y(), m_vars[2].texcoord.y() - m_vars[0].texcoord.y(), 0.0f);
    }

    ers::vec4 VertexShaderPerVertex(const void* in, s32 which_vert)
    {       
        const VertexAttributes1* vert = (const VertexAttributes1*)in;
        m_vars[which_vert].fragpos = ers::vec3(uniform_model * ers::vec4(vert->aPos, 1.0f));		
		m_vars[which_vert].normal = uniform_model_it * vert->aNormal;		
        m_vars[which_vert].texcoord = vert->aTexcoord;
        m_vars[which_vert].lightspace_fragpos = uniform_lightspace_mat * uniform_model * ers::vec4(vert->aPos, 1.0f);	

		const ers::vec4 position = uniform_mvp_mat * ers::vec4(vert->aPos, 1.0f);       
        return position;
    }

    bool FragmentShader(ers::vec4& out) override
    {                
        ers::vec3 normal = get_normal();

        const f32 amb_val = 0.3f;

        const ers::vec3 light_dir = get_light_direction();
        const f32 diff_val = ers::max(0.0f, ers::dot(normal, light_dir));    
        
        const f32 shininess = get_shininess(); 
        const ers::vec3 view_dir = ers::normalize(uniform_view_pos - m_varsInterpolated.fragpos); 
        const ers::vec3 half_dir = ers::normalize(light_dir + view_dir); 
        const f32 spec_val = pow(ers::max(0.0f, ers::dot(normal, half_dir)), shininess); 

        const f32 shadow_val = calculate_shadow_value(m_varsInterpolated.lightspace_fragpos);

        ers::vec3 diffuse_color = get_diffuse_sample(); 

        ers::vec3 final_color;  
        const f32 light_specular_intensity = 0.5f;

        if (sampler2d_specular_map != nullptr) 
            final_color = diffuse_color * (amb_val + (1.0f - shadow_val) * (diff_val + light_specular_intensity * spec_val));
        else 
            final_color = diffuse_color * (amb_val + (1.0f - shadow_val) * diff_val) + ers::vec3((1.0f - shadow_val) * light_specular_intensity * spec_val);

        // f32 intensity = 0.6f * diff_val + 0.4f * spec_val;
        // if (intensity > 0.9f) 
        // {
        //     intensity = 1.1f;
        // }
        // else if (intensity > 0.5f) 
        // {
        //     intensity = 0.7f;
        // }
        // else 
        // {
        //     intensity = 0.5f;
        // }

        // final_color = diffuse_color * (amb_val + (1.0f - shadow_val) * intensity);

        out = ers::vec4(final_color, 1.0f);     
        return false;
    }  
};

#endif // BLINNPHONG_SHADER_H