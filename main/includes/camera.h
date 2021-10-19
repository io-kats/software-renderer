#ifndef CAMERA_H
#define CAMERA_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "ers/common.h"
#include "ers/matrix.h"
#include "ers/vec.h"
#include "ers/macros.h"
#include "ray.h"

constexpr f32 MAX_PITCH = 89.0f * ers::PI<f32>::val() / 180.0f;
constexpr f32 DEFAULT_YAW = -ers::HALF_PI<f32>::val();
constexpr f32 DEFAULT_PITCH = 0.0f;
constexpr s32 DEFAULT_ZOOM = 100;
constexpr f32 DEFAULT_MOUSE_SENSITIVITY = 0.0025f;
constexpr f32 DEFAULT_MOVEMENT_SENSITIVITY = 4.0f;

enum class Camera_Movement {
	FORWARD,
	RIGHT,
	BACKWARD,
	LEFT,
	UPWARD,
	DOWNWARD
};

enum class Camera_Mode {
	FLY,
	FPS
};

class Camera
{
public:
	Camera();
	Camera(ers::vec3 position_,
		f32 width = 1280.0f,
		f32 height = 720.f,
		f32 fov = ers::HALF_PI<f32>::val() * 0.5f,
		f32 aperture = 0.0f,
		f32 zNear = 0.1f,
		f32 zFar = 100.0f,		
		f32 yaw = DEFAULT_YAW,	
		f32 pitch = DEFAULT_PITCH,		 
		ers::vec3 world_up = ers::vec3(0.0f, 1.0f, 0.0f),
		f32 movement_sensitivity = DEFAULT_MOVEMENT_SENSITIVITY,
		f32 mouse_sensitivity = DEFAULT_MOUSE_SENSITIVITY,
		s32 zoom = DEFAULT_ZOOM,	   
		Camera_Mode mode = Camera_Mode::FPS
	);
	~Camera();

	// TODO: Abstract controls away?
	void KeyboardControls(Camera_Movement mov, f32 delta_time);
	void MouseControls(f32 xoffset, f32 yoffset);
	ers::mat4 GetViewMatrix();
	ers::mat4 GetProjectionMatrix();
	ers::vec3 GetPosition();
	void SetPosition(const ers::vec3& pos_);
	void ToggleMode();
	void SetZoom(s32 zoom_);
	void Zoom(s32 zoom_amount);

	void UpdateVectors();
	void UpdateProjection(f32 new_width, f32 new_height);
	Ray GetRayAt(f32 mouse_x, f32 mouse_y);
	Ray GetSimpleRayAt(f32 mouse_x, f32 mouse_y); // same as above, but ignores field of depth.

private:
	ers::vec3 m_position;
	ers::vec3 m_front;
	ers::vec3 m_up;
	ers::vec3 m_right;
	ers::vec3 m_worldUp;	

	f32 m_width;
	f32 m_height; 
	f32 m_fov;
	f32 m_zNear; // doubles as focus distance/field of depth for ray tracing
	f32 m_zFar;
	f32 m_lensRadius;

	f32 m_yaw;
	f32 m_pitch;

	f32 m_movementSensitivity;
	f32 m_mouseSensitivity;
	
	s32 m_zoom;

	Camera_Mode m_mode;

	f32 m_halfTan;
	f32 m_aspectRatio;
};

#endif