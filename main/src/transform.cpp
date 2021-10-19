#include "transform.h"

Transform::Transform()
    : m_translation(ers::vec3(0.0f)), m_rotation(ers::quat::identity()), m_scale(ers::vec3(1.0f)) { }

Transform::Transform(const ers::vec3& t, const ers::quat& r, const ers::vec3& s)
    : m_translation(t), m_rotation(r), m_scale(s) { }

ers::mat3 Transform::GetRotationScaleMatrix() const
{
    ers::mat3 r(m_scale);
    r = ers::quatToMat3(m_rotation) * r;
    return r;
}

ers::mat3 Transform::GetRotationMatrix() const
{
    return ers::quatToMat3(m_rotation);
}

ers::vec3 Transform::GetScale() const
{
    return m_scale;
}

ers::vec3 Transform::GetTranslation() const
{
    return m_translation;
}

void Transform::SetRotation(const f32& angle, ers::vec3 r)
{
    r = ers::normalize(r);
    const f32 half_angle = 0.5f * angle;
    const f32 s = sin(half_angle);
    
    m_rotation = ers::quat(s * r, cosf(half_angle));
}

void Transform::SetScale(const ers::vec3& s)
{
    m_scale = s;
}

void Transform::SetTranslation(const ers::vec3& t)
{
    m_translation = t;
}

void Transform::SetTranslationX(f32 x)
{
    m_translation.x() = x;
}

void Transform::SetTranslationY(f32 y)
{
    m_translation.y() = y;
}

void Transform::SetTranslationZ(f32 z)
{
    m_translation.z() = z;
}

f32 Transform::GetTranslationX() const
{
    return m_translation.x();
}

f32 Transform::GetTranslationY() const
{
    return m_translation.y();
}

f32 Transform::GetTranslationZ() const
{
    return m_translation.z();
}

void Transform::SetScalenX(f32 x)
{
    m_scale.x() = x;
}

void Transform::SetScalenY(f32 y)
{
    m_scale.y() = y;
}

void Transform::SetScaleZ(f32 z)
{
    m_scale.z() = z;
}

f32 Transform::GetScaleX() const
{
    return m_scale.x();
}

f32 Transform::GetScaleY() const
{
    return m_scale.y();
}

f32 Transform::GetScaleZ() const
{
    return m_scale.z();
}

void Transform::Translate(const ers::vec3& t)
{
    m_translation += t;
}

void Transform::Rotate(const f32& angle, const ers::vec3& r)
{
    m_rotation = ers::rotate(m_rotation, angle, r);
}

void Transform::Scale(const ers::vec3& s)
{
    m_scale *= s;
}

void Transform::Reset()
{
    m_translation = ers::vec3(0.0f);
    m_rotation = ers::quat::identity();
    m_scale = ers::vec3(1.0f);
}

ers::mat4 Transform::GetModelMatrix() const
{
    ers::mat4 m(1.0f);
    m = ers::scale(m, m_scale);
    m = ers::quatToMat4(m_rotation) * m;
    m = ers::translate(m, m_translation);
    return m;
}