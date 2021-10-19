#include "camera.h"

Camera::Camera()
	: m_position(ers::vec3(0.0f, 0.0f, 0.0f)),
	m_worldUp(ers::vec3(0.0f, 1.0f, 0.0f)),
	m_width(1280.0f),
	m_height(720.f),
	m_fov(ers::HALF_PI<f32>::val() * 0.5f),
	m_zNear(0.1f),
	m_zFar(100.0f),		
	m_lensRadius(0.0f),
	m_yaw(DEFAULT_YAW),
	m_pitch(DEFAULT_PITCH),
	m_movementSensitivity(DEFAULT_MOVEMENT_SENSITIVITY),
	m_mouseSensitivity(DEFAULT_MOUSE_SENSITIVITY),	
	m_zoom(DEFAULT_ZOOM),
	m_mode(Camera_Mode::FPS)
{ 
	m_halfTan = tanf(((f32)m_zoom / 100.f) * m_fov * 0.5f);
	m_aspectRatio = m_width / m_height;
	UpdateVectors();
}

Camera::Camera(
	ers::vec3 position_,
	f32 width_,
	f32 height_,
	f32 fov_,
	f32 aperture_,
	f32 zNear_,
	f32 zFar_,	
	f32 yaw_,
	f32 pitch_,
	ers::vec3 world_up_,
	f32 movement_sensitivity_,
	f32 mouse_sensitivity_,			   
	s32 zoom_,
	Camera_Mode mode_)
	: m_position(position_), 
	m_worldUp(ers::normalize(world_up_)), 
	m_width(width_),
	m_height(height_),
	m_fov(fov_),	
	m_zNear(zNear_),
	m_zFar(zFar_),
	m_lensRadius(aperture_ * 0.5f),
	m_yaw(yaw_),
	m_pitch(pitch_),	
	m_movementSensitivity(movement_sensitivity_),
	m_mouseSensitivity(mouse_sensitivity_),	
	m_zoom(zoom_),
	m_mode(mode_)
{
	m_halfTan = tanf(((f32)m_zoom / 100.f) * m_fov * 0.5f);
	m_aspectRatio = m_width / m_height;
	UpdateVectors();
}

Camera::~Camera() { }

void Camera::KeyboardControls(Camera_Movement mov, f32 delta_time)
{
	ers::vec3 move_direction = (m_mode == Camera_Mode::FPS) ? ers::normalize(ers::vec3(m_front.x(), 0.0f, m_front.z())) : m_front;
	switch (mov) 
	{
		case Camera_Movement::FORWARD: 
		{
			m_position += move_direction * m_movementSensitivity * delta_time;
		} break;
		case Camera_Movement::RIGHT: 
		{
			m_position += m_right * m_movementSensitivity * delta_time;
		} break;
		case Camera_Movement::BACKWARD: 
		{
			m_position -= move_direction * m_movementSensitivity * delta_time;
		} break;
		case Camera_Movement::LEFT: 
		{
			m_position -= m_right * m_movementSensitivity * delta_time;
		} break;
		case Camera_Movement::DOWNWARD: 
		{
			m_position -= m_worldUp * m_movementSensitivity * delta_time;
		} break;
		case Camera_Movement::UPWARD: 
		{
			m_position += m_worldUp * m_movementSensitivity * delta_time;
		} break;
		default: 
		{
			ERS_ASSERT(false);
		}
	}

	UpdateVectors();
}

void Camera::MouseControls(f32 xoffset, f32 yoffset)
{
	xoffset *= m_mouseSensitivity;
	yoffset *= m_mouseSensitivity;

	m_yaw += xoffset;
	m_pitch += yoffset;

	m_pitch = ers::clamp(m_pitch, -MAX_PITCH, MAX_PITCH);
	UpdateVectors();
}

ers::mat4 Camera::GetViewMatrix()
{	
	return ers::lookAt(m_position, m_position + m_front, m_up);
}

ers::mat4 Camera::GetProjectionMatrix()
{
	return ers::perspective(((f32)m_zoom * 0.01f) * m_fov, 1.0f / m_aspectRatio, m_zNear, m_zFar);
}

ers::vec3 Camera::GetPosition()
{
	return m_position;
}

void Camera::SetPosition(const ers::vec3& pos_)
{
	m_position = pos_;
}

void Camera::ToggleMode()
{
	if (m_mode == Camera_Mode::FLY)
		m_mode = Camera_Mode::FPS;
	else
		m_mode = Camera_Mode::FLY;
	UpdateVectors();
}

void Camera::SetZoom(s32 zoom_)
{
	m_zoom = zoom_;
	m_zoom = ers::clamp(m_zoom, 5, 200);
	m_halfTan = tanf(((f32)m_zoom * 0.01f) * m_fov * 0.5f);
}

void Camera::Zoom(s32 zoom_amount)
{
	m_zoom += zoom_amount;
	m_zoom = ers::clamp(m_zoom, 5, 200);
	m_halfTan = tanf(((f32)m_zoom * 0.01f) * m_fov * 0.5f);
}

void Camera::UpdateVectors()
{
	m_front.x() = cosf(m_yaw) * cosf(m_pitch);
	m_front.y() = sinf(m_pitch);
	m_front.z() = sinf(m_yaw) * cosf(m_pitch);
	m_front = ers::normalize(m_front);
	m_right = ers::normalize(ers::cross(m_front, m_worldUp));
	m_up = ers::normalize(cross(m_right, m_front));
}

void Camera::UpdateProjection(f32 new_width, f32 new_height)
{
	m_width = new_width;
	m_height = new_height;
	// half_tan = tanf(((f32)zoom * 0.01f) * fov * 0.5f);
	m_aspectRatio = m_width / m_height;
}

Ray Camera::GetRayAt(f32 mouse_x, f32 mouse_y)
{
	// From screen space to world space.
	mouse_x = 2.0f * (mouse_x / m_width) - 1.0f; 
	mouse_x *= m_aspectRatio * m_halfTan * m_zNear;

	mouse_y = 2.0f * (mouse_y / m_height) - 1.0f; 
	mouse_y *= m_halfTan * m_zNear;

	// Make ray.
	ers::vec3 perturbation(0.0f);
	if (m_lensRadius != 0.0f)
	{
		const ers::vec2 rand_c = random_point_in_disk((f32)ers::random_frac(), (f32)ers::random_frac()) * m_lensRadius;
		perturbation = m_right * rand_c.x() + m_up * rand_c.y();		
	}
	
	Ray result;
	const ers::vec3 target = m_position + m_front * m_zNear + m_right * mouse_x + m_up * mouse_y;
	result.orig = m_position + perturbation;
	result.dir = ers::normalize(target - result.orig);
	return result;
}

Ray Camera::GetSimpleRayAt(f32 mouse_x, f32 mouse_y)
{
	// From screen space to world space.
	mouse_x = 2.0f * (mouse_x / m_width) - 1.0f; 
	mouse_x *= m_aspectRatio * m_halfTan;

	mouse_y = 2.0f * (mouse_y / m_height) - 1.0f; 
	mouse_y *= m_halfTan;

	// Make ray.	
	Ray result;
	result.orig = m_position;
	result.dir = ers::normalize(m_front + m_right * mouse_x + m_up * mouse_y);
	return result;
}