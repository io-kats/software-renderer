#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "ers/matrix.h"
#include "ers/vec.h"
#include "ers/quaternion.h"

class Transform
{
public:
	Transform();
	Transform(const ers::vec3& t, const ers::quat& r, const ers::vec3& s);

	ers::mat3 GetRotationScaleMatrix() const;
	ers::mat3 GetRotationMatrix() const;
	ers::vec3 GetScale() const;
	ers::vec3 GetTranslation() const;

	void SetRotation(const f32& angle, ers::vec3 r);
	void SetScale(const ers::vec3& s);
	void SetTranslation(const ers::vec3& t);

	void SetTranslationX(f32 x);
	void SetTranslationY(f32 y);
	void SetTranslationZ(f32 z);
	f32 GetTranslationX() const;
	f32 GetTranslationY() const;
	f32 GetTranslationZ() const;

	void SetScalenX(f32 x);
	void SetScalenY(f32 y);
	void SetScaleZ(f32 z);
	f32 GetScaleX() const;
	f32 GetScaleY() const;
	f32 GetScaleZ() const;

	void Translate(const ers::vec3& t);
	void Rotate(const f32& angle, const ers::vec3& r);
	void Scale(const ers::vec3& s);

	void Reset();

	ers::mat4 GetModelMatrix() const;

private:
	ers::vec3 m_translation;
	ers::quat m_rotation;
	ers::vec3 m_scale;
};

#endif // TRANSFORM_H
