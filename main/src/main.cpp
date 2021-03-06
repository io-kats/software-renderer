// Copyright 2021 Ioannis Katsios <ioannis.katsios1@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining 
// a copy of this software and associated documentation files (the 
// "Software"), to deal in the Software without restriction, including 
// without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to 
// permit persons to whom the Software is furnished to do so, subject to 
// the following conditions:

// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "ers/typedefs.h"
#include "ers/common.h"
#include "window.h"
#include "ers/vec.h"
#include "ers/matrix.h"
#include "image.h"
#include "mesh.h"
#include "software_renderer.h"
#include "gl_shader_program.h"
#include "gl_surface.h"
#include "camera.h"
#include "transform.h"

#include "simple_shader.h"
#include "debug_light_shader.h"
#include "blinn_phong_shader.h"
#include "shadowmap_shader.h"

#define RESOURCES "./resources/"

#define RAND_F32 (f32)ers::random_frac()
#define RAND_V3F32 ers::vec3((f32)ers::random_frac(), (f32)ers::random_frac(), (f32)ers::random_frac())
#define RAND_V3F32_2 ers::vec3(2.0f * (f32)ers::random_frac() - 1.0f, 2.0f * (f32)ers::random_frac() - 1.0f, 2.0f * (f32)ers::random_frac() - 1.0f)

class App : public Window
{
private:
	Renderer* m_renderer;

	ers::String m_stringBuf;

	f32 m_triangle[3][6] = 
	{
		{ -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f },
	};

	Mesh m_quadMesh; 
	Mesh m_cubeMesh; 
	Mesh m_monkeyMesh; 
	Mesh m_arrowMesh; 

	Image* m_modelDiffuse;	

	Image* m_floorDiffuse;	
	Image* m_floorSpecular;	
	Image* m_floorNormal;		

	Image* m_shadowmap;

	SimpleShader m_simpleShader;
	DebugLightShader m_debugLightShader;
	ShadowmapShader m_shadowmapShader;
	BlinnPhongShader m_blinnPhongShader;	

	Camera* m_playerCamera;

	GLSurface m_surface;

	struct MeshInstance
	{
		const Mesh* mesh;
		Transform transform;
		ers::vec3 color;
		ers::vec3 rotation_axis;
	};

	MeshInstance m_lightCube;
	MeshInstance m_arrowInstance;
	MeshInstance m_monkeyInstance;
	MeshInstance m_floorInstance;

	ers::Vector<MeshInstance> m_cubes;

	enum Scene
	{
		HELLO_TRIANGLE = 0,
		CUBES,
		TEXTURE,
		INVALID
	};

	s32 m_whichScene;
	s32 m_numOfImages;
	s32 m_numOfPcfDims;

public:
	App(const char* title_, int width_, int height_, int windowpos_x, int windowpos_y)
		: 
		Window(title_, width_, height_, windowpos_x, windowpos_y),
		m_stringBuf(ers::String(1024)),
		m_surface(width_, height_)
	{ 
		
	}

	MeshInstance MakeRandomCube()
	{
		MeshInstance result;
		result.mesh = &m_cubeMesh;
		result.transform.Translate(RAND_V3F32_2 * 4.0f);
		result.rotation_axis = ers::normalize(RAND_V3F32_2);
		result.transform.Rotate(ers::radians(360.0f * RAND_F32), result.rotation_axis);
		result.transform.Scale(RAND_V3F32 * 0.5f);
		result.color = RAND_V3F32;
		return result;
	}

	void HelloTriangleSceneUpdateAndDraw()
	{
		m_renderer->Clear();

		VertexAttributes3 v0, v1, v2;
		v0.aPos = ers::vec3(m_triangle[0][0], m_triangle[0][1], m_triangle[0][2]);
		v0.aColor = ers::vec3(m_triangle[0][3], m_triangle[0][4], m_triangle[0][5]);

		v1.aPos = ers::vec3(m_triangle[1][0], m_triangle[1][1], m_triangle[1][2]);
		v1.aColor = ers::vec3(m_triangle[1][3], m_triangle[1][4], m_triangle[1][5]);

		v2.aPos = ers::vec3(m_triangle[2][0], m_triangle[2][1], m_triangle[2][2]);
		v2.aColor = ers::vec3(m_triangle[2][3], m_triangle[2][4], m_triangle[2][5]);
		
		m_renderer->SetShaderProgram(&m_simpleShader);
		m_renderer->RenderTriangle(&v0, &v1, &v2);
	}

	void CubesSceneInit()
	{
		const s32 count_cubes = 10;
		m_cubes.Reserve(count_cubes);
		for (s32 i = 0; i < count_cubes; ++i)
			m_cubes.PushBack(MakeRandomCube());
	}

	void CubesSceneUpdateAndDraw()
	{
		f32 current_time = (f32)GetCurrentFrameTime();
		ers::vec3 light_pos = ers::vec3(cosf(current_time), 0.0f, sinf(current_time));
		const s32 count_cubes = (s32)m_cubes.GetSize();

		for (s32 i = 0; i < count_cubes; ++i)
			m_cubes[i].transform.SetRotation(5.0f * current_time, m_cubes[i].rotation_axis);

		m_renderer->SetViewport(GetWindowWidth(), GetWindowHeight());
		m_renderer->Clear(0.2f, 0.2f, 0.3f);

		m_blinnPhongShader.uniform_do_random_color = false;
		m_blinnPhongShader.uniform_do_specific_color = true;
		m_blinnPhongShader.uniform_do_point_light = true;
		m_blinnPhongShader.uniform_light_pos = light_pos;
		m_blinnPhongShader.uniform_view_pos = m_playerCamera->GetPosition();

		const ers::mat4 proj = m_playerCamera->GetProjectionMatrix();
		const ers::mat4 view = m_playerCamera->GetViewMatrix();
		const ers::mat4 vp = proj * view;

		m_blinnPhongShader.sampler2d_diffuse_map = nullptr;
		m_blinnPhongShader.sampler2d_normal_map = nullptr;
		m_blinnPhongShader.sampler2d_specular_map = nullptr;	
		m_blinnPhongShader.sampler2d_shadow_map = nullptr;	

		m_blinnPhongShader.uniform_lightspace_mat = m_shadowmapShader.uniform_lightspace_mat;
		m_renderer->SetShaderProgram(&m_blinnPhongShader);
		
		for (s32 i = 0; i < count_cubes; ++i)
		{
			ers::mat4 tr = m_cubes[i].transform.GetModelMatrix();
			m_blinnPhongShader.uniform_mvp_mat = vp * tr; 	
			m_blinnPhongShader.uniform_model = tr;
			m_blinnPhongShader.uniform_model_it = ers::mat3(ers::transpose(ers::inverse(tr)));
			m_blinnPhongShader.uniform_color = m_cubes[i].color;
			m_cubes[i].mesh->Draw(m_renderer);
		}

		m_lightCube.transform.SetTranslation(light_pos);
		ers::mat4 tr_cube = m_lightCube.transform.GetModelMatrix();	
		m_debugLightShader.uniform_wireframe = false;
		m_debugLightShader.uniform_scale = m_lightCube.transform.GetScaleX() * ers::sin_norm(current_time, 0.8f, 1.0f, 2.0f);
		m_debugLightShader.uniform_mvp_mat = vp * tr_cube;
		m_debugLightShader.uniform_model = tr_cube;
		m_debugLightShader.uniform_color = m_lightCube.color;
		m_debugLightShader.uniform_light_pos = light_pos;
		m_renderer->SetShaderProgram(&m_debugLightShader);
		m_lightCube.mesh->Draw(m_renderer);
	}

	void MakeFloorTextures()
	{
		const s32 dim = 512;
		const s32 step = dim / 8;		

		m_floorDiffuse  = new Image(dim, dim, Image::Format::RGB, Image::Range::LDR);			
		m_floorSpecular  = new Image(dim, dim, Image::Format::GRAYSCALE, Image::Range::LDR);	
		m_floorNormal  = new Image(dim, dim, Image::Format::RGB, Image::Range::LDR);

		Image* height_map = new Image(dim, dim, Image::Format::GRAYSCALE, Image::Range::HDR);	

		const f32 half_step = 0.5f * (f32)step;
		const f32 half_dim = 0.5f * (f32)dim;
		const f32 inv_dim2 = 1.0f / (f32)(dim * dim);
		const f32 inv_dim = 1.0f / (f32)dim;
		for (s32 y = 0; y < dim; y += step)
		{
			for (s32 x = 0; x < dim; x += step)
			{
				const ers::vec3 color = RAND_V3F32;
				for (s32 yy = y; yy < y + step; ++yy)
				{
					const f32 ry2 = ((f32)yy - half_dim) * ((f32)yy - half_dim);
					for (s32 xx = x; xx < x + step; ++xx)
					{				
						const f32 r2 = ((f32)xx - half_dim) * ((f32)xx - half_dim) + ry2;
						const f32 m = ers::sin_norm(r2 * inv_dim2, 0.3f, 1.0f, 0.1f);
						m_floorDiffuse->Set(xx, yy, m * color);						
						m_floorSpecular->Set(xx, yy, sqrtf(m));
						height_map->Set(xx, yy, m);
					}
				}
			}
		}
		
		for (s32 y = 0; y < dim; ++y)
		{
			const f32 t0 = (f32)(y - 1) * inv_dim;
			const f32 t1 = (f32)(y + 1) * inv_dim;
			for (s32 x = 0; x < dim; ++x)
			{
				const f32 s0 = (f32)(x - 1) * inv_dim;
				const f32 s1 = (f32)(x + 1) * inv_dim;

				ers::vec3 m00(s0, t0, 0.0f); height_map->Get(s0, t0, m00.e[2]);
				ers::vec3 m01(s1, t1, 0.0f); height_map->Get(s1, t1, m01.e[2]);
				ers::vec3 m10(s0, t1, 0.0f); height_map->Get(s0, t1, m10.e[2]);
				ers::vec3 m11(s1, t0, 0.0f); height_map->Get(s1, t0, m11.e[2]);

				ers::vec3 m0 = m01 - m00;
				ers::vec3 m1 = m11 - m10;
				ers::vec3 result = ers::normalize(ers::cross(m1, m0));
				m_floorNormal->Set(x, y, result * 0.5f + ers::vec3(0.5f));
			}
		}

		delete height_map;
	}

	void TextureSceneInit()
	{
		load_object_file(RESOURCES"arrow.obj", m_arrowMesh);
		m_arrowInstance.mesh = &m_arrowMesh;
		m_arrowInstance.color = ers::vec3(0.4f);
		m_arrowInstance.transform.Reset();
		m_arrowInstance.transform.Scale(ers::vec3(0.05f));

		m_modelDiffuse  = new Image(RESOURCES"test.png");	 		
		load_object_file(RESOURCES"monkey.obj", m_monkeyMesh);
		m_monkeyInstance.mesh = &m_monkeyMesh;
		m_monkeyInstance.color = ers::vec3(1.0f);
		m_monkeyInstance.transform.Reset();
		m_monkeyInstance.transform.Translate(ers::vec3(0.0f, 0.0f, -4.0f));
		m_monkeyInstance.transform.Scale(ers::vec3(1.5f));
		m_monkeyInstance.transform.Rotate(ers::radians(-90.0f), ers::vec3(0.0f, 1.0f, 0.0f));

		MakeFloorTextures();
		m_floorInstance.mesh = &m_quadMesh;
		m_floorInstance.color = ers::vec3(0.2f, 0.2f, 0.3f);
		m_floorInstance.transform.Reset();
		m_floorInstance.transform.Translate(ers::vec3(0.0f, -1.5f, -4.0f));
		m_floorInstance.transform.Scale(ers::vec3(10.0f));
		m_floorInstance.transform.Rotate(ers::radians(-90.0f), ers::vec3(1.0f, 0.0f, 0.0f));		
	}

	void TextureSceneUpdateAndDraw()
	{
		const f32 current_time = (f32)GetCurrentFrameTime();
		const f32 dt = (f32)GetDeltaTime();

		const ers::vec3 pos_texture_cube = m_monkeyInstance.transform.GetTranslation();
		m_monkeyInstance.transform.Rotate(dt * ers::radians(30.0f), ers::vec3(0.0f, 1.0f, 0.0f));
		const ers::mat4 tr_texture_cube = m_monkeyInstance.transform.GetModelMatrix();

		const ers::vec3 light_pos = m_monkeyInstance.transform.GetTranslation() 
			+ ers::vec3(4.0f * cosf(current_time * 0.5f), ers::sin_norm(current_time * 0.5f, 1.0f, 5.0f, 6.0f), 4.0f * sinf(current_time * 0.5f));
		const f32 zFar = 15.0f;
		const ers::mat4 light_proj = ers::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, zFar);
		const ers::mat4 light_view = ers::lookAt(light_pos, pos_texture_cube + ers::vec3(0.0f, 0.0f, -1.0f), ers::vec3(0.0f, 1.0f, 0.0f));

		// Do a renderpass for shadows.
		m_renderer->SetViewport(m_shadowmap->GetWidth(), m_shadowmap->GetHeight());
		m_renderer->Clear();
		
		m_shadowmapShader.uniform_light_pos = light_pos;		
		m_shadowmapShader.uniform_lightspace_mat = light_proj * light_view;
		m_shadowmapShader.uniform_zFar = zFar;
		m_shadowmapShader.uniform_model = tr_texture_cube;
		m_renderer->SetShaderProgram(&m_shadowmapShader);
		m_monkeyInstance.mesh->Draw(m_renderer);

		const ers::mat4 tr_floor = m_floorInstance.transform.GetModelMatrix();	
		m_shadowmapShader.uniform_model = tr_floor;
		m_floorInstance.mesh->Draw(m_renderer);

		// Copy z_buffer to shadowmap.
		void* p_to = m_shadowmap->GetData();
		void* p_from = m_renderer->GetZBuffer();
		memcpy(p_to, p_from, m_shadowmap->GetSize() * sizeof(f32));

		// Render normally.
		m_renderer->SetViewport(GetWindowWidth(), GetWindowHeight());
		m_renderer->Clear();

		m_blinnPhongShader.uniform_do_random_color = false;
		m_blinnPhongShader.uniform_do_specific_color = false;
		m_blinnPhongShader.uniform_do_point_light = false;
		m_blinnPhongShader.uniform_pcf_dims = m_numOfPcfDims;
		m_blinnPhongShader.uniform_light_dir = ers::normalize(pos_texture_cube - light_pos);
		m_blinnPhongShader.uniform_view_pos = m_playerCamera->GetPosition();

		const ers::mat4 proj = m_playerCamera->GetProjectionMatrix();
		const ers::mat4 view = m_playerCamera->GetViewMatrix();
		const ers::mat4 vp = proj * view;

		m_blinnPhongShader.sampler2d_diffuse_map = m_modelDiffuse;
		m_blinnPhongShader.sampler2d_normal_map = nullptr;
		m_blinnPhongShader.sampler2d_specular_map = nullptr;	
		m_blinnPhongShader.sampler2d_shadow_map = m_shadowmap;	

		m_blinnPhongShader.uniform_lightspace_mat = m_shadowmapShader.uniform_lightspace_mat;
		m_blinnPhongShader.uniform_mvp_mat = vp * tr_texture_cube; 	
		m_blinnPhongShader.uniform_model = tr_texture_cube;
		m_blinnPhongShader.uniform_model_it = ers::mat3(ers::transpose(ers::inverse(tr_texture_cube)));
		m_blinnPhongShader.uniform_color = ers::vec3(0.1f, 0.5f, 0.2f);

		m_renderer->SetShaderProgram(&m_blinnPhongShader);
		m_monkeyInstance.mesh->Draw(m_renderer);

		m_blinnPhongShader.uniform_do_specific_color = false;
		m_blinnPhongShader.uniform_color = m_floorInstance.color;
		m_blinnPhongShader.uniform_model = tr_floor;
		m_blinnPhongShader.uniform_model_it = ers::mat3(ers::transpose(ers::inverse(tr_floor)));
		m_blinnPhongShader.uniform_mvp_mat = vp * tr_floor; 
		m_blinnPhongShader.sampler2d_diffuse_map = m_floorDiffuse;
		m_blinnPhongShader.sampler2d_normal_map = m_floorNormal;
		m_blinnPhongShader.sampler2d_specular_map = m_floorSpecular;	
		m_blinnPhongShader.sampler2d_shadow_map = m_shadowmap;	
		m_floorInstance.mesh->Draw(m_renderer);

		m_arrowInstance.transform.SetTranslation(light_pos);
		m_arrowInstance.transform.SetRotation(acosf(m_blinnPhongShader.uniform_light_dir.y()), ers::cross(ers::vec3(0.0f, 1.0f, 0.0f), m_blinnPhongShader.uniform_light_dir));
		ers::mat4 tr_cube = m_arrowInstance.transform.GetModelMatrix();	
		m_debugLightShader.uniform_scale = 0.7f;
		m_debugLightShader.uniform_wireframe = true;
		m_debugLightShader.uniform_mvp_mat = vp * tr_cube;
		m_debugLightShader.uniform_model = tr_cube;
		m_debugLightShader.uniform_color = m_arrowInstance.color;
		m_debugLightShader.uniform_light_pos = light_pos;
		m_renderer->SetShaderProgram(&m_debugLightShader);
		m_arrowInstance.mesh->Draw(m_renderer);
	}

	void TextureSceneCleanup()
	{
		delete m_modelDiffuse; 
		delete m_floorDiffuse; 
		delete m_floorSpecular; 
		delete m_floorNormal; 
	}

	void Init() override
	{		
		ers::init_rand();

		m_whichScene = Scene::HELLO_TRIANGLE;
		m_numOfImages = 0;
		m_numOfPcfDims = 1;

		m_shadowmap = new Image(512, 512, Image::Format::GRAYSCALE, Image::Range::HDR);	

		make_cube(m_cubeMesh);
		make_quad(m_quadMesh);

		m_lightCube.mesh = &m_cubeMesh;
		m_lightCube.color = ers::vec4(1.0f);
		m_lightCube.transform.Reset();
		m_lightCube.transform.Scale(ers::vec3(0.4f));

		const s32 w = GetWindowWidth();
		const s32 h = GetWindowHeight();
		
		m_playerCamera = new Camera(
			ers::vec3(0.0f, 0.5f, 6.0f), 
			(f32)w, 
			(f32)h,
			ers::radians(45.0f),
			0.0f,
			0.2f
		);

		m_renderer = new Renderer(w, h);	
		m_renderer->Enable(Renderer::DEPTH_TEST);
		m_renderer->Enable(Renderer::CULL_FACE); 

		CubesSceneInit();
		TextureSceneInit();
	}

	void ProcessInput()
	{
		const f32 dt = (f32)GetDeltaTime();
		if (KeyHeld(GLFW_KEY_W))
			m_playerCamera->KeyboardControls(Camera_Movement::FORWARD, dt);

		if (KeyHeld(GLFW_KEY_D))
			m_playerCamera->KeyboardControls(Camera_Movement::RIGHT, dt);

		if (KeyHeld(GLFW_KEY_S))
			m_playerCamera->KeyboardControls(Camera_Movement::BACKWARD, dt);

		if (KeyHeld(GLFW_KEY_A))
			m_playerCamera->KeyboardControls(Camera_Movement::LEFT, dt);

		if (KeyPressed(GLFW_KEY_C))
			m_playerCamera->ToggleMode();

		if (KeyPressed(GLFW_KEY_V))
			m_renderer->Toggle(Renderer::WIREFRAME);

		if (KeyPressed(GLFW_KEY_F))
		{
			s32 n = m_numOfImages;
			m_stringBuf.Sprintf("image%d", n / 100); n %= 100;
			m_stringBuf.AppendSprintf("%d", n / 10); n %= 10;
			m_stringBuf.AppendSprintf("%d.png", n);			
			m_renderer->WriteToFile(m_stringBuf.GetCstr());
			++m_numOfImages;
			if (m_numOfImages > 999)
				m_numOfImages = 0;
		}
			
		if (KeyPressed(GLFW_KEY_RIGHT))
		{
			++m_whichScene;
			if (m_whichScene == Scene::INVALID) 
				m_whichScene = 0;
		}

		if (KeyPressed(GLFW_KEY_LEFT))
		{
			--m_whichScene;
			if (m_whichScene < 0) 
				m_whichScene = (s32)Scene::INVALID - 1;
		}

		if (KeyPressed(GLFW_KEY_UP))
		{
			if (m_whichScene == Scene::CUBES)
			{
				for (s32 i = 0; i < 10; ++i)
					m_cubes.PushBack(MakeRandomCube());
				printf("Number of parallepipeds on screen: %d\n", (s32)m_cubes.GetSize());
			}
			else if (m_whichScene == Scene::TEXTURE)
			{	
				m_numOfPcfDims += 2;
				printf("Number of pcf samples: %d\n", m_numOfPcfDims * m_numOfPcfDims);
			}
		}

		if (KeyPressed(GLFW_KEY_DOWN))
		{
			if (m_whichScene == Scene::CUBES && m_cubes.GetSize() > 0)
			{
				m_cubes.Resize(m_cubes.GetSize() - 10);
				printf("Number of parallepipeds on screen: %d\n", (s32)m_cubes.GetSize());
			}
			else if (m_whichScene == Scene::TEXTURE)
			{	
				m_numOfPcfDims -= 2;
				if (m_numOfPcfDims < 1)
					m_numOfPcfDims = 1;
				else
					printf("Number of pcf samples: %d\n", m_numOfPcfDims * m_numOfPcfDims);
			}
		}

		if (MouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT))
		{
			ers::vec2 mouse_dr = GetMouseDelta();
			m_playerCamera->MouseControls(mouse_dr.x(), -mouse_dr.y());
		}
	}

    void Update() override
	{			
		if (WindowSizeChanged()) 
		{
			const s32 w = GetWindowWidth();
			const s32 h = GetWindowHeight();
			m_renderer->SetViewport(w, h);
			m_playerCamera->UpdateProjection((f32)w, (f32)h);
			m_surface.Resize(w, h);
		}

		ProcessInput();

		switch (m_whichScene)
		{
			case Scene::HELLO_TRIANGLE:
				HelloTriangleSceneUpdateAndDraw();
				break;
			case Scene::CUBES:
				CubesSceneUpdateAndDraw();
				break;
			case Scene::TEXTURE:
				TextureSceneUpdateAndDraw();
				break;
			default:
				ERS_UNREACHABLE();
		}

		m_surface.Draw(m_renderer->GetColorBuffer());
	}

	void Cleanup() override
	{
		TextureSceneCleanup();
		delete m_renderer;
		delete m_playerCamera;	
		delete m_shadowmap;
	}
};


int main()
{		
	const s32 w = 800;
	const s32 h = 600;

	App app(
		"Software Renderer", 
		w, 
		h, 
		960 - w / 2, 
		540 - h / 2
	);
	app.Run();
	app.Cleanup();
	return 0;
}