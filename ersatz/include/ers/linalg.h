#ifndef LINALG_H
#define LINALG_H

#include "./ers/common.h"
#include <limits>

// "Small" linear algebra library for graphics. No swizzling operators. Sort-of under construction.
// TODO: Find a way to make it smaller with a better use of templates, if possible.

namespace ers
{
	// ----------------------------
	// :::::::::::VEC 2::::::::::::
	// ----------------------------

	template<typename T>
	struct Vec3;

	template<typename T>
	struct Vec4;

	template<typename T>
	struct Vec2 {
		T e[2];

		Vec2() = default;
		Vec2(T elem) { e[0] = elem; e[1] = elem; }
		Vec2(T x_, T y_) { e[0] = x_; e[1] = y_; }
		Vec2(const Vec3<T>& v) { e[0] = v.e[0]; e[1] = v.e[1]; }
		Vec2(const Vec4<T>& v) { e[0] = v.e[0]; e[1] = v.e[1]; }

		inline Vec2<T> operator +(const Vec2<T>& v2) const { return Vec2<T>(e[0] + v2.e[0], e[1] + v2.e[1]); }
		inline Vec2<T> operator -(const Vec2<T>& v2) const { return Vec2<T>(e[0] - v2.e[0], e[1] - v2.e[1]); }
		inline Vec2<T> operator -() const { return Vec2<T>(-e[0], -e[1]); }
		inline Vec2<T> operator *(const Vec2<T>& v2) const { return Vec2<T>(e[0] * v2.e[0], e[1] * v2.e[1]); }
		inline Vec2<T> operator *(const T& s) const { return Vec2<T>(e[0] * s, e[1] * s); }
		inline Vec2<T>& operator +=(const Vec2<T>& v2) { e[0] += v2.e[0]; e[1] += v2.e[1]; return *this; }
		inline Vec2<T>& operator -=(const Vec2<T>& v2) { e[0] -= v2.e[0]; e[1] -= v2.e[1]; return *this; }
		inline Vec2<T>& operator *=(const Vec2<T>& v2) { e[0] *= v2.e[0]; e[1] *= v2.e[1]; return *this; }
		inline Vec2<T>& operator *=(const T& s) { e[0] *= s; e[1] *= s; return *this; }

		inline T& operator [](int i)
		{
			ERS_ASSERTF(i >= 0 && i < 2, "%s", "Vec2: Out of bounds.");
			return e[i];
		}

		inline T operator [](int i) const
		{
			ERS_ASSERTF(i >= 0 && i < 2, "%s", "Vec2: Out of bounds.");
			return e[i];
		}

		inline Vec2<T>& operator /=(const Vec2<T>& v2)
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0));
			e[0] /= v2.e[0]; e[1] /= v2.e[1];
			return *this;
		}

		inline Vec2<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			e[0] *= s; e[1] *= s;
			return *this;
		}

		inline Vec2<T> operator /(const Vec2<T>& v2)  const
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0));
			return Vec2<T>(e[0] / v2.e[0], e[1] / v2.e[1]);
		}

		inline Vec2<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			return Vec2<T>(e[0] * s, e[1] * s);
		}

		inline bool operator ==(const Vec2<T>& v2) const
		{
			return e[0] == v2.e[0] && e[1] == v2.e[1];
		}

		inline bool operator !=(const Vec2<T>& v2) const
		{
			return e[0] != v2.e[0] || e[1] != v2.e[1];
		}

		inline bool operator <(const Vec2<T>& v2) const
		{
			return e[0] < v2.e[0] && e[1] < v2.e[1];
		}

		inline bool operator >(const Vec2<T>& v2) const
		{
			return e[0] > v2.e[0] && e[1] > v2.e[1];
		}

        inline T& x() { return e[0]; }
        inline T& y() { return e[1]; }

        inline T x() const { return e[0]; }
        inline T y() const { return e[1]; }
	};

	template<typename T>
	inline Vec2<T> operator*(const T& s, const Vec2<T>& v)
	{
		return Vec2<T>(v.e[0] * s, v.e[1] * s);
	}

	template<typename T>
	inline T dot(const Vec2<T>& v1, const Vec2<T>& v2)
	{
		return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1];
	}

	template<typename T>
	inline T cross(const Vec2<T>& v1, const Vec2<T>& v2)
	{
		return v1.e[0] * v2.e[1] - v2.e[0] * v1.e[1];
	}

	template<typename T>
	inline T length(const Vec2<T>& v)
	{
		return sqrt(v.e[0] * v.e[0] + v.e[1] * v.e[1]);
	}

	template<typename T>
	inline T length2(const Vec2<T>& v)
	{
		return v.e[0] * v.e[0] + v.e[1] * v.e[1];
	}

	template<typename T>
	inline Vec2<T> normalize(const Vec2<T>& v)
	{
		T len = length(v);
		ERS_ASSERTF(len > T(0), "%s", "Vec2::Normalize: zero length.");
		len = 1 / len;
		return v * len;
	}

	template<typename T>
	inline Vec2<T> abs(const Vec2<T>& v)
	{		
		return Vec2<T>(abs(v.e[0]), abs(v.e[1]));
	}

	template<typename T>
	inline Vec2<T> min(const Vec2<T>& v1, const Vec2<T>& v2)
	{
		return Vec2<T>(min(v1.e[0], v2.e[0]), min(v1.e[1], v2.e[1]));
	}

	template<typename T>
	inline Vec2<T> max(const Vec2<T>& v1, const Vec2<T>& v2)
	{
		return Vec2<T>(max(v1.e[0], v2.e[0]), max(v1.e[1], v2.e[1]));
	}

	template<typename T>
	inline Vec2<T> clamp(const Vec2<T>& v, const Vec2<T>& v_min, const Vec2<T>& v_max)
	{
		return Vec2<T>(clamp(v.e[0], v_min.e[0], v_max.e[0]), clamp(v.e[1], v_min.e[1], v_max.e[1]));
	}

	template<typename T>
	inline bool equals_epsilon(const Vec2<T>& v1, const Vec2<T>& v2, const T& epsilon = eps<T>::val())
	{
		return equals_epsilon(v1.e[0], v2.e[0], epsilon) && equals_epsilon(v1.e[1], v2.e[1], epsilon);
	}

	template<typename T>
	inline Vec2<T> approach(const Vec2<T>& from, const Vec2<T>& to, T amount)
	{
		const Vec2<T> result = from + amount * ers::normalize(to - from);
		return equals_epsilon(result, to) ? to : result;
	}

	// ----------------------------
	// :::::::::::VEC 3::::::::::::
	// ----------------------------

	template<typename T>
	struct Vec3 {
		T e[3];

		Vec3() = default;
		Vec3(T elem) { e[0] = elem; e[1] = elem; e[2] = elem; }
		Vec3(T x_, T y_, T z_) { e[0] = x_; e[1] = y_; e[2] = z_; }
		Vec3(const Vec2<T>& v, T z_) { e[0] = v.e[0]; e[1] = v.e[1]; e[2] = z_; }
		Vec3(const Vec4<T>& v) { e[0] = v.e[0]; e[1] = v.e[1]; e[2] = v.e[2]; }

		inline Vec3<T> operator +(const Vec3<T>& v2) const { return Vec3<T>( e[0] + v2.e[0], e[1] + v2.e[1], e[2] + v2.e[2]); }
		inline Vec3<T> operator -(const Vec3<T>& v2) const { return Vec3<T>( e[0] - v2.e[0], e[1] - v2.e[1], e[2] - v2.e[2]); }
		inline Vec3<T> operator -() const { return Vec3<T>(-e[0], -e[1], -e[2]); }
		inline Vec3<T> operator *(const Vec3<T>& v2) const { return Vec3<T>(e[0] * v2.e[0], e[1] * v2.e[1], e[2] * v2.e[2]); }
		inline Vec3<T> operator *(const T& s) const { return Vec3<T>(e[0] * s, e[1] * s, e[2] * s); }
		inline Vec3<T>& operator +=(const Vec3<T>& v2) { e[0] += v2.e[0]; e[1] += v2.e[1]; e[2] += v2.e[2]; return *this; }
		inline Vec3<T>& operator -=(const Vec3<T>& v2) { e[0] -= v2.e[0]; e[1] -= v2.e[1]; e[2] -= v2.e[2]; return *this; }
		inline Vec3<T>& operator *=(const Vec3<T>& v2) { e[0] *= v2.e[0]; e[1] *= v2.e[1]; e[2] *= v2.e[2]; return *this; }
		inline Vec3<T>& operator *=(const T& s) { e[0] *= s; e[1] *= s; e[2] *= s; return *this; }

		inline T& operator [](int i)
		{
			ERS_ASSERTF(i >= 0 && i < 3, "%s", "Vec3: Out of bounds.");
			return e[i];
		}

		inline T operator [](int i) const
		{
			ERS_ASSERTF(i >= 0 && i < 3, "%s", "Vec3: Out of bounds.");
			return e[i];
		}

		inline Vec3<T>& operator /=(const Vec3<T>& v2)
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0) && abs(v2.e[2]) > T(0));
			e[0] /= v2.e[0]; e[1] /= v2.e[1]; e[2] /= v2.e[2];
			return *this;
		}

		inline Vec3<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			e[0] *= s; e[1] *= s; e[2] *= s;
			return *this;
		}

		inline Vec3<T> operator /(const Vec3<T>& v2)  const
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0) && abs(v2.e[2]) > T(0));
			return Vec3<T>(e[0] / v2.e[0], e[1] / v2.e[1], e[2] / v2.e[2]);
		}

		inline Vec3<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			return Vec3<T>(e[0] * s, e[1] * s, e[2] * s);
		}

		inline bool operator ==(const Vec3<T>& v2) const
		{
			return e[0] == v2.e[0] && e[1] == v2.e[1] && e[2] == v2.e[2];
		}

		inline bool operator !=(const Vec3<T>& v2) const
		{
			return e[0] != v2.e[0] || e[1] != v2.e[1] || e[2] != v2.e[2];
		}

		inline bool operator <(const Vec3<T>& v2) const
		{
			return e[0] < v2.e[0] && e[1] < v2.e[1] && e[2] < v2.e[2];
		}

		inline bool operator >(const Vec3<T>& v2) const
		{
			return e[0] > v2.e[0] && e[1] > v2.e[1] && e[2] > v2.e[2];
		}

		inline T& x() { return e[0]; }
        inline T& y() { return e[1]; }
        inline T& z() { return e[2]; }

        inline T x() const { return e[0]; }
        inline T y() const { return e[1]; }
        inline T z() const { return e[2]; }
	};

	template<typename T>
	inline Vec3<T> operator*(const T& s, const Vec3<T>& v)
	{
		return Vec3<T>(v.e[0] * s, v.e[1] * s, v.e[2] * s);
	}

	template<typename T>
	inline T dot(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
	}

	template<typename T>
	inline Vec3<T> cross(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return Vec3<T>(v1.e[1] * v2.e[2] - v2.e[1] * v1.e[2], v1.e[2] * v2.e[0] - v2.e[2] * v1.e[0], v1.e[0] * v2.e[1] - v2.e[0] * v1.e[1]);
	}

	// v pointing at surface, n assumed normalized surface normal.
	template<typename T>
	inline Vec3<T> reflect(const Vec3<T>& v, const Vec3<T>& n)
	{
		const f32 temp = -2.0f * dot(v, n);
		return v + n * temp;
	}

	template<typename T>
	inline Vec3<T> refract(const Vec3<T>& v, const Vec3<T>& n, f32 refraction_index)
	{
		const f32 ri2 = refraction_index * refraction_index;
		const f32 dot_vn = dot(v, n);
		const f32 len_v2 = length2(v);
		f32 rn = len_v2 * (1.0f - ri2) + ri2 * dot_vn * dot_vn;

		// rn = (rn <= 0.0f) ? (-2.0f * dot_vn) : (-sqrf(rn) - refraction_index * dot_vn);
		// const f32 rv = (rn <= 0.0f) ? 1.0f : refraction_index;

		rn = (rn <= 0.0f) ? 0.0f : (-sqrtf(rn) - refraction_index * dot_vn);
		const f32 rv = (rn <= 0.0f) ? 0.0f : refraction_index;

		return v * rv + n * rn;
	}

	template<typename T>
	inline T length(const Vec3<T>& v)
	{
		return sqrt(v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2]);
	}

	template<typename T>
	inline T length2(const Vec3<T>& v)
	{
		return v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2];
	}

	template<typename T>
	inline Vec3<T> normalize(const Vec3<T>& v)
	{
		T len = ers::length(v);
		ERS_ASSERTF(len > T(0), "%s", "Vec3::Normalize: zero length.");	
		len = 1 / len;
		return v * len;
	}

	// return a non-unique vector that is perpendicular to v.
	template<typename T>
	inline Vec3<T> normal_to(const Vec3<T>& v)
	{
		if (abs(v.e[0]) > T(0) || abs(v.e[1]) > T(0))
			return Vec3<T>(-v.e[1], v.e[0], T(0));
		else if (abs(v.e[2]) > T(0))
			return Vec3<T>(T(0), T(1), T(0));
		ERS_ASSERT(false);
		return Vec3<T>(T(0));
	}

	template<typename T>
	inline Vec3<T> abs(const Vec3<T>& v)
	{
		return Vec3<T>(abs(v.e[0]), abs(v.e[1]), abs(v.e[2]));
	}

	template<typename T>
	inline Vec3<T> min(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return Vec3<T>(min(v1.e[0], v2.e[0]), min(v1.e[1], v2.e[1]), min(v1.e[2], v2.e[2]));
	}

	template<typename T>
	inline Vec3<T> max(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return Vec3<T>(max(v1.e[0], v2.e[0]), max(v1.e[1], v2.e[1]), max(v1.e[2], v2.e[2]));
	}

	template<typename T>
	inline Vec3<T> clamp(const Vec3<T>& v, const Vec3<T>& v_min, const Vec3<T>& v_max)
	{
		return Vec3<T>(clamp(v.e[0], v_min.e[0], v_max.e[0]), clamp(v.e[1], v_min.e[1], v_max.e[1]), clamp(v.e[2], v_min.e[2], v_max.e[2]));
	}

	template<typename T>
	inline bool equals_epsilon(const Vec3<T>& v1, const Vec3<T>& v2, const T& epsilon = eps<T>::val())
	{
		return equals_epsilon(v1.e[0], v2.e[0], epsilon) && equals_epsilon(v1.e[1], v2.e[1], epsilon) && equals_epsilon(v1.e[2], v2.e[2], epsilon);
	}

	// ----------------------------
	// :::::::::::VEC 4::::::::::::
	// ----------------------------

	template<typename T>
	struct Vec4 {
		T e[4];

		Vec4() = default;
		Vec4(T elem) { e[0] = elem; e[1] = elem; e[2] = elem; e[3] = elem; }
		Vec4(T x_, T y_, T z_, T w_) { e[0] = x_; e[1] = y_; e[2] = z_; e[3] = w_; }
		Vec4(const Vec2<T>& v, T z_, T w_) { e[0] = v.e[0]; e[1] = v.e[1]; e[2] = z_; e[3] = w_; }
		Vec4(const Vec3<T>& v, T w_) { e[0] = v.e[0]; e[1] = v.e[1]; e[2] = v.e[2]; e[3] = w_; }

		inline Vec4<T> operator +(const Vec4<T>& v2) const { return Vec4<T>(e[0] + v2.e[0], e[1] + v2.e[1], e[2] + v2.e[2], e[3] + v2.e[3]); }
		inline Vec4<T> operator -(const Vec4<T>& v2) const { return Vec4<T>(e[0] - v2.e[0], e[1] - v2.e[1], e[2] - v2.e[2], e[3] - v2.e[3]); }
		inline Vec4<T> operator -() const { return Vec4<T>(-e[0], -e[1], -e[2], -e[3]); }
		inline Vec4<T> operator *(const Vec4<T>& v2) const { return Vec4<T>(e[0] * v2.e[0], e[1] * v2.e[1], e[2] * v2.e[2], e[3] * v2.e[3]); }
		inline Vec4<T> operator *(const T& s) const { return Vec4<T>(e[0] * s, e[1] * s, e[2] * s, e[3] * s); }
		inline Vec4<T>& operator +=(const Vec4<T>& v2) { e[0] += v2.e[0]; e[1] += v2.e[1]; e[2] += v2.e[2]; e[3] += v2.e[3]; return *this; }
		inline Vec4<T>& operator -=(const Vec4<T>& v2) { e[0] -= v2.e[0]; e[1] -= v2.e[1]; e[2] -= v2.e[2]; e[3] -= v2.e[3]; return *this; }
		inline Vec4<T>& operator *=(const Vec4<T>& v2) { e[0] *= v2.e[0]; e[1] *= v2.e[1]; e[2] *= v2.e[2]; e[3] *= v2.e[3]; return *this; }
		inline Vec4<T>& operator *=(const T& s) { e[0] *= s; e[1] *= s; e[2] *= s; e[3] *= s; return *this; }

		inline T& operator [](int i)
		{
			ERS_ASSERTF(i >= 0 && i < 4, "%s", "Vec4: Out of bounds.");
			return e[i];
		}

		inline T operator [](int i) const
		{
			ERS_ASSERTF(i >= 0 && i < 4, "%s", "Vec4: Out of bounds.");
			return e[i];
		}

		inline Vec4<T>& operator /=(const Vec4<T>& v2)
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0)
				&& abs(v2.e[2]) > T(0) && abs(v2.e[3]) > T(0));
			e[0] /= v2.e[0]; e[1] /= v2.e[1]; e[2] /= v2.e[2]; e[3] /= v2.e[3];
			return *this;
		}

		inline Vec4<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			e[0] *= s; e[1] *= s; e[2] *= s; e[3] *= s;
			return *this;
		}

		inline Vec4<T> operator /(const Vec4<T>& v2)  const
		{
			ERS_ASSERT(abs(v2.e[0]) > T(0) && abs(v2.e[1]) > T(0)
				&& abs(v2.e[2]) > T(0) && abs(v2.e[3]) > T(0));
			return Vec4<T>(e[0] / v2.e[0], e[1] / v2.e[1], e[2] / v2.e[2], e[3] / v2.e[3]);
		}

		inline Vec4<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			return Vec4<T>(e[0] * s, e[1] * s, e[2] * s, e[3] * s);
		}

		inline bool operator ==(const Vec4<T>& v2) const
		{
			return e[0] == v2.e[0] && e[1] == v2.e[1] && e[2] == v2.e[2] && e[3] == v2.e[3];
		}

		inline bool operator !=(const Vec4<T>& v2) const
		{
			return e[0] != v2.e[0] || e[1] != v2.e[1] || e[2] != v2.e[2] || e[3] != v2.e[3];
		}

		inline bool operator <(const Vec4<T>& v2) const
		{
			return e[0] < v2.e[0] && e[1] < v2.e[1] && e[2] < v2.e[2] && e[3] < v2.e[3];
		}

		inline bool operator >(const Vec4<T>& v2) const
		{
			return e[0] > v2.e[0] && e[1] > v2.e[1] && e[2] > v2.e[2] && e[3] > v2.e[3];
		}

		inline T& x() { return e[0]; }
        inline T& y() { return e[1]; }
        inline T& z() { return e[2]; }
        inline T& w() { return e[3]; }

        inline T x() const { return e[0]; }
        inline T y() const { return e[1]; }
        inline T z() const { return e[2]; }
        inline T w() const { return e[3]; }
	};

	template<typename T>
	inline Vec4<T> operator*(const T& s, const Vec4<T>& v)
	{
		return Vec4<T>(v.e[0] * s, v.e[1] * s, v.e[2] * s, v.e[3] * s);
	}

	template<typename T>
	inline T dot(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
	}

	template<typename T>
	inline T length(const Vec4<T>& v)
	{
		return sqrt(v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2] + v.e[3] * v.e[3]);
	}

	template<typename T>
	inline T length2(const Vec4<T>& v)
	{
		return v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2] + v.e[3] * v.e[3];
	}

	template<typename T>
	inline Vec4<T> normalize(const Vec4<T>& v)
	{
		T len = length(v);
		ERS_ASSERTF(len > T(0), "%s", "Vec4::Normalize: zero length.");
		len = 1 / len;
		return v * len;
	}

	template<typename T>
	inline Vec4<T> cross(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return Vec4<T>(v1.e[1] * v2.e[2] - v2.e[1] * v1.e[2], v1.e[2] * v2.e[0] - v2.e[2] * v1.e[0], v1.e[0] * v2.e[1] - v2.e[0] * v1.e[1], T(1));
	}

	template<typename T>
	inline Vec4<T> abs(const Vec4<T>& v)
	{
		return Vec4<T>(abs(v.e[0]), abs(v.e[1]), abs(v.e[2]), abs(v.e[3]));
	}

	template<typename T>
	inline Vec4<T> min(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return Vec4<T>(min(v1.e[0], v2.e[0]), min(v1.e[1], v2.e[1]), min(v1.e[2], v2.e[2]), min(v1.e[3], v2.e[3]));
	}

	template<typename T>
	inline Vec4<T> max(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return Vec4<T>(max(v1.e[0], v2.e[0]), max(v1.e[1], v2.e[1]), max(v1.e[2], v2.e[2]), max(v1.e[3], v2.e[3]));
	}

	template<typename T>
	inline Vec4<T> clamp(const Vec4<T>& v, const Vec4<T>& v_min, const Vec4<T>& v_max)
	{
		return Vec4<T>(clamp(v.e[0], v_min.e[0], v_max.e[0]), clamp(v.e[1], v_min.e[1], v_max.e[1]), clamp(v.e[2], v_min.e[2], v_max.e[2]), clamp(v.e[3], v_min.e[3], v_max.e[3]));
	}

	template<typename T>
	inline bool equals_epsilon(const Vec4<T>& v1, const Vec4<T>& v2, const T& epsilon = eps<T>::val())
	{
		return equals_epsilon(v1.e[0], v2.e[0], epsilon) && equals_epsilon(v1.e[1], v2.e[1], epsilon) 
			&& equals_epsilon(v1.e[2], v2.e[2], epsilon) && equals_epsilon(v1.e[3], v2.e[3], epsilon);
	}

	// ---------------------------- 
	// :::::::::::MAT 2::::::::::::
	// ----------------------------

	template<typename T>
	struct Mat3;

	template<typename T>
	struct Mat4;

	template<typename T>
	struct Mat2 {
		Vec2<T> v[2];

		Mat2() = default;

		explicit Mat2(T elem)
		{
			v[0].e[0] = elem; v[1].e[0] = T(0);
			v[0].e[1] = T(0); v[1].e[1] = elem;
		}

		explicit Mat2(const Vec2<T>& col1, const Vec2<T>& col2) { v[0] = col1; v[1] = col2; }

		explicit Mat2(const Vec2<T>& v_in)
		{
			v[0].e[0] = v_in.e[0]; v[1].e[0] = T(0);
			v[0].e[1] = T(0); v[1].e[1] = v_in.e[1];
		}

		explicit Mat2(const Mat3<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0];
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1];
		}

		explicit Mat2(const Mat4<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0];
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1];
		}

		inline T& operator ()(int i, int j)
		{
			ERS_ASSERTF(i >= 0 && i < 2 && j >= 0 && j < 2, "%s", "Mat2: Out of bounds.");
			return v[j].e[i];
		}

		inline T operator ()(int i, int j) const
		{
			ERS_ASSERTF(i >= 0 && i < 2 && j >= 0 && j < 2, "%s", "Mat2: Out of bounds.");
			return v[j].e[i];
		}

		inline Mat2<T> operator +(const Mat2<T>& m2) const
		{
			Mat2<T> m_out;
			m_out.v[0] = v[0] + m2.v[0]; m_out.v[1] = v[1] + m2.v[1];
			return m_out;
		}

		inline Mat2<T> operator -(const Mat2<T>& m2) const
		{
			Mat2<T> m_out;
			m_out.v[0] = v[0] - m2.v[0]; m_out.v[1] = v[1] - m2.v[1];
			return m_out;
		}

		inline Mat2<T> operator -() const
		{
			Mat2<T> m_out;
			m_out.v[0] = -v[0]; m_out.v[1] = -v[1];
			return m_out;
		}

		inline Mat2<T> operator *(const T& s) const
		{
			Mat2<T> m_out;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s;
			return m_out;
		}

		inline Mat2<T> operator *(const Mat2<T>& m2) const
		{
			Mat2<T> m_out;
			m_out.v[0] = v[0] * m2.v[0].e[0] + v[1] * m2.v[0].e[1];
			m_out.v[1] = v[0] * m2.v[1].e[0] + v[1] * m2.v[1].e[1];
			return m_out;
		}

		inline Vec2<T> operator *(const Vec2<T>& v_in) const
		{
			return v[0] * v_in.e[0] + v[1] * v_in.e[1];
		}

		inline Mat2<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			Mat2<T> m_out;
			s = 1 / s;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s;
			return m_out;
		}

		inline Mat2<T>& operator +=(const Mat2<T>& m2)
		{
			v[0] += m2.v[0];  v[1] += m2.v[1];
			return *this;
		}

		inline Mat2<T>& operator -=(const Mat2<T>& m2)
		{
			v[0] -= m2.v[0]; v[1] -= m2.v[1];
			return *this;
		}

		inline Mat2<T>& operator *=(const T& s)
		{
			v[0] *= s; v[1] *= s;
			return *this;
		}

		inline Mat2<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			v[0] *= s; v[1] *= s;
			return *this;
		}

		inline void Print() const
		{
			printf("-------------------\n");
			printf("[%f, %f]\n", v[0].e[0], v[1].e[0]);
			printf("[%f, %f]\n", v[0].e[1], v[1].e[1]);
			printf("-------------------\n\n");
		}
	};

	template<typename T>
	inline Mat2<T> operator *(const T& s, const Mat2<T>& m2)
	{
		Mat2<T> m_out;
		m_out.v[0] = m2.v[0] * s; m_out.v[1] = m2.v[1] * s;
		return m_out;
	}

	// ----------------------------
	// :::::::::::MAT 3::::::::::::
	// ----------------------------

	template<typename T>
	struct Mat3 {
		Vec3<T> v[3];

		Mat3() = default;

		explicit Mat3(T elem)
		{
			v[0].e[0] = elem; v[1].e[0] = T(0); v[2].e[0] = T(0);
			v[0].e[1] = T(0); v[1].e[1] = elem; v[2].e[1] = T(0);
			v[0].e[2] = T(0); v[1].e[2] = T(0); v[2].e[2] = elem;
		}

		explicit Mat3(const Vec3<T>& v_in)
		{
			v[0].e[0] = v_in.e[0]; v[1].e[0] = T(0); v[2].e[0] = T(0);
			v[0].e[1] = T(0); v[1].e[1] = v_in.e[1]; v[2].e[1] = T(0);
			v[0].e[2] = T(0); v[1].e[2] = T(0); v[2].e[2] = v_in.e[2];
		}

		explicit Mat3(const Vec3<T>& col1, const Vec3<T>& col2, const Vec3<T>& col3)
		{
			v[0] = col1; v[1] = col2; v[2] = col3;
		}

		explicit Mat3(const Mat2<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0]; v[2].e[0] = T(0);
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1]; v[2].e[1] = T(0);
			v[0].e[2] = T(0); v[1].e[2] = T(0); v[2].e[2] = T(1);
		}

		explicit Mat3(const Mat4<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0]; v[2].e[0] = m.v[2].e[0];
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1]; v[2].e[1] = m.v[2].e[1];
			v[0].e[2] = m.v[0].e[2]; v[1].e[2] = m.v[1].e[2]; v[2].e[2] = m.v[2].e[2];
		}

		inline T& operator ()(int i, int j)
		{
			ERS_ASSERTF(i >= 0 && i < 3 && j >= 0 && j < 3, "%s", "Mat3: Out of bounds.");
			return v[j].e[i];
		}

		inline T operator ()(int i, int j) const
		{
			ERS_ASSERTF(i >= 0 && i < 3 && j >= 0 && j < 3, "%s", "Mat3: Out of bounds.");
			return v[j].e[i];
		}

		inline Mat3<T> operator +(const Mat3<T>& m2) const
		{
			Mat3<T> m_out;
			m_out.v[0] = v[0] + m2.v[0]; m_out.v[1] = v[1] + m2.v[1]; m_out.v[2] = v[2] + m2.v[2];
			return m_out;
		}

		inline Mat3<T> operator -(const Mat3<T>& m2) const
		{
			Mat3<T> m_out;
			m_out.v[0] = v[0] - m2.v[0]; m_out.v[1] = v[1] - m2.v[1]; m_out.v[2] = v[2] - m2.v[2];
			return m_out;
		}

		inline Mat3<T> operator -() const
		{
			Mat3<T> m_out;
			m_out.v[0] = -v[0]; m_out.v[1] = -v[1]; m_out.v[2] = -v[2];
			return m_out;
		}

		inline Mat3<T> operator *(const T& s) const
		{
			Mat3<T> m_out;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s; m_out.v[2] = v[2] * s;
			return m_out;
		}

		inline Mat3<T> operator *(const Mat3<T>& m_in) const
		{
			Mat3<T> m_out;
			m_out.v[0] = v[0] * m_in.v[0].e[0] + v[1] * m_in.v[0].e[1] + v[2] * m_in.v[0].e[2];
			m_out.v[1] = v[0] * m_in.v[1].e[0] + v[1] * m_in.v[1].e[1] + v[2] * m_in.v[1].e[2];
			m_out.v[2] = v[0] * m_in.v[2].e[0] + v[1] * m_in.v[2].e[1] + v[2] * m_in.v[2].e[2];
			return m_out;
		}

		inline Vec3<T> operator *(const Vec3<T>& v_in) const
		{
			return v[0] * v_in.e[0] + v[1] * v_in.e[1] + v[2] * v_in.e[2];
		}

		inline Mat3<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			Mat3<T> m_out;
			s = 1 / s;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s; m_out.v[2] = v[2] * s;
			return m_out;
		}

		inline Mat3<T>& operator +=(const Vec2<T>& m2)
		{
			v[0] += m2.v[0]; v[1] += m2.v[1]; v[2] += m2.v[2];
			return *this;
		}

		inline Mat3<T>& operator -=(const Vec2<T>& m2)
		{
			v[0] -= m2.v[0]; v[1] -= m2.v[1]; v[2] -= m2.v[2];
			return *this;
		}

		inline Mat3<T>& operator *=(const T& s)
		{
			v[0] *= s; v[1] *= s; v[2] *= s;
			return *this;
		}

		inline Mat3<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			v[0] *= s; v[1] *= s; v[2] *= s;
			return *this;
		}

		inline void Print() const
		{
			printf("-------------------\n");
			printf("[%f, %f, %f]\n", v[0].e[0], v[1].e[0], v[2].e[0]);
			printf("[%f, %f, %f]\n", v[0].e[1], v[1].e[1], v[2].e[1]);
			printf("[%f, %f, %f]\n", v[0].e[2], v[1].e[2], v[2].e[2]);
			printf("-------------------\n\n");
		}
	};

	template<typename T>
	inline Mat3<T> operator *(const T& s, const Mat3<T>& m2)
	{
		Mat3<T> m_out;
		m_out.v[0] = m2.v[0] * s; m_out.v[1] = m2.v[1] * s; m_out.v[2] = m2.v[2] * s;
		return m_out;
	}

	template<typename T>
	inline Mat3<T> skew(const Vec3<T>& v)
	{
		Mat3<T> m_out;
		m_out.v[0].e[0] = T(0); m_out.v[1].e[0] = -v.e[2]; m_out.v[2].e[0] = v.e[1];
		m_out.v[0].e[1] = v.e[2]; m_out.v[1].e[1] = T(0); m_out.v[2].e[1] = -v.e[0];
		m_out.v[0].e[2] = -v.e[1]; m_out.v[1].e[2] = v.e[0]; m_out.v[2].e[2] = T(0);
		return m_out;
	}

	// ----------------------------
	// :::::::::::MAT 4::::::::::::
	// ----------------------------

	template<typename T>
	struct Mat4 {
		Vec4<T> v[4];

		Mat4() = default;

		explicit Mat4(T elem)
		{
			v[0].e[0] = elem; v[1].e[0] = T(0); v[2].e[0] = T(0); v[3].e[0] = T(0);
			v[0].e[1] = T(0); v[1].e[1] = elem; v[2].e[1] = T(0); v[3].e[1] = T(0);
			v[0].e[2] = T(0); v[1].e[2] = T(0); v[2].e[2] = elem; v[3].e[2] = T(0);
			v[0].e[3] = T(0); v[1].e[3] = T(0); v[2].e[3] = T(0); v[3].e[3] = elem;
		}

		explicit Mat4(const Vec4<T>& col1, const Vec4<T>& col2, const Vec4<T>& col3, const Vec4<T>& col4)
		{
			v[0] = col1; v[1] = col2; v[2] = col3; v[3] = col4;
		}

		explicit Mat4(const Mat2<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0]; v[2].e[0] = T(0); v[3].e[0] = T(0);
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1]; v[2].e[1] = T(0); v[3].e[1] = T(0);
			v[0].e[2] = T(0); v[1].e[2] = T(0); v[2].e[2] = T(1); v[3].e[2] = T(0);
			v[0].e[3] = T(0); v[1].e[3] = T(0); v[2].e[3] = T(0); v[3].e[3] = T(1);
		}

		explicit Mat4(const Mat3<T>& m)
		{
			v[0].e[0] = m.v[0].e[0]; v[1].e[0] = m.v[1].e[0]; v[2].e[0] = m.v[2].e[0]; v[3].e[0] = T(0);
			v[0].e[1] = m.v[0].e[1]; v[1].e[1] = m.v[1].e[1]; v[2].e[1] = m.v[2].e[1]; v[3].e[1] = T(0);
			v[0].e[2] = m.v[0].e[2]; v[1].e[2] = m.v[1].e[2]; v[2].e[2] = m.v[2].e[2]; v[3].e[2] = T(0);
			v[0].e[3] = T(0); v[1].e[3] = T(0); v[2].e[3] = T(0); v[3].e[3] = T(1);
		}

		inline T& operator ()(int i, int j)
		{
			ERS_ASSERTF(i >= 0 && i < 4 && j >= 0 && j < 4, "%s", "Mat4: Out of bounds.");
			return v[j].e[i];
		}

		inline T operator ()(int i, int j) const
		{
			ERS_ASSERTF(i >= 0 && i < 4 && j >= 0 && j < 4, "%s", "Mat4: Out of bounds.");
			return v[j].e[i];
		}

		inline Mat4<T> operator +(const Mat4<T>& m2) const
		{
			Mat4<T> m_out;
			m_out.v[0] = v[0] + m2.v[0]; m_out.v[1] = v[1] + m2.v[1];
			m_out.v[2] = v[2] + m2.v[2]; m_out.v[3] = v[3] + m2.v[3];
			return m_out;
		}

		inline Mat4<T> operator -(const Mat4<T>& m2) const
		{
			Mat4<T> m_out;
			m_out.v[0] = v[0] - m2.v[0]; m_out.v[1] = v[1] - m2.v[1];
			m_out.v[2] = v[2] - m2.v[2]; m_out.v[3] = v[3] - m2.v[3];
			return m_out;
		}

		inline Mat4<T> operator -() const
		{
			Mat4<T> m_out;
			m_out.v[0] = -v[0]; m_out.v[1] = -v[1];
			m_out.v[2] = -v[2]; m_out.v[3] = -v[3];
			return m_out;
		}

		inline Mat4<T> operator *(const T& s) const
		{
			Mat4<T> m_out;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s; m_out.v[2] = v[2] * s; m_out.v[3] = v[3] * s;
			return m_out;
		}

		inline Mat4<T> operator *(const Mat4<T>& m_in) const
		{
			Mat4<T> m_out;
			m_out.v[0] = v[0] * m_in.v[0].e[0] + v[1] * m_in.v[0].e[1] + v[2] * m_in.v[0].e[2] + v[3] * m_in.v[0].e[3];
			m_out.v[1] = v[0] * m_in.v[1].e[0] + v[1] * m_in.v[1].e[1] + v[2] * m_in.v[1].e[2] + v[3] * m_in.v[1].e[3];
			m_out.v[2] = v[0] * m_in.v[2].e[0] + v[1] * m_in.v[2].e[1] + v[2] * m_in.v[2].e[2] + v[3] * m_in.v[2].e[3];
			m_out.v[3] = v[0] * m_in.v[3].e[0] + v[1] * m_in.v[3].e[1] + v[2] * m_in.v[3].e[2] + v[3] * m_in.v[3].e[3];
			return m_out;
		}

		inline Vec4<T> operator *(const Vec4<T>& v_in) const
		{
			return v[0] * v_in.e[0] + v[1] * v_in.e[1] + v[2] * v_in.e[2] + v[3] * v_in.e[3];
		}

		inline Mat4<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > T(0));
			Mat4<T> m_out;
			s = 1 / s;
			m_out.v[0] = v[0] * s; m_out.v[1] = v[1] * s; m_out.v[2] = v[2] * s; m_out.v[3] = v[3] * s;
			return m_out;
		}

		inline Mat4<T>& operator +=(const Vec2<T>& m2)
		{
			v[0] += m2.v[0]; v[1] += m2.v[1]; v[2] += m2.v[2]; v[3] += m2.v[3];
			return *this;
		}

		inline Mat4<T>& operator -=(const Vec2<T>& m2)
		{
			v[0] -= m2.v[0]; v[1] -= m2.v[1]; v[2] -= m2.v[2]; v[3] -= m2.v[3];
			return *this;
		}

		inline Mat4<T>& operator *=(const T& s)
		{
			v[0] *= s; v[1] *= s; v[2] *= s; v[3] *= s;
			return *this;
		}

		inline Mat4<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > T(0));
			s = 1 / s;
			v[0] *= s; v[1] *= s; v[2] *= s; v[3] *= s;
			return *this;
		}

		inline void Print() const
		{
			printf("-------------------\n");
			printf("[%f, %f, %f, %f]\n", v[0].e[0], v[1].e[0], v[2].e[0], v[3].e[0]);
			printf("[%f, %f, %f, %f]\n", v[0].e[1], v[1].e[1], v[2].e[1], v[3].e[1]);
			printf("[%f, %f, %f, %f]\n", v[0].e[2], v[1].e[2], v[2].e[2], v[3].e[2]);
			printf("[%f, %f, %f, %f]\n", v[0].e[3], v[1].e[3], v[2].e[3], v[3].e[3]);
			printf("-------------------\n\n");
		}
	};

	template<typename T>
	inline Mat4<T> operator *(const T& s, const Mat4<T>& m2)
	{
		Mat4<T> m_out;
		m_out.v[0] = m2.v[0] * s;
		m_out.v[1] = m2.v[1] * s;
		m_out.v[2] = m2.v[2] * s;
		m_out.v[3] = m2.v[3] * s;
		return m_out;
	}

	// ----------------------------
	// ::::::MATRIX FUNCTIONS::::::
	// ----------------------------

	template<typename T>
	inline Mat4<T> transpose(const Mat4<T>& m_in)
	{
		Mat4<T> m_out;

		m_out.v[0].e[0] = m_in.v[0].e[0]; 
		m_out.v[0].e[1] = m_in.v[1].e[0]; 
		m_out.v[0].e[2] = m_in.v[2].e[0]; 
		m_out.v[0].e[3] = m_in.v[3].e[0]; 

		m_out.v[1].e[0] = m_in.v[0].e[1]; 
		m_out.v[1].e[1] = m_in.v[1].e[1]; 
		m_out.v[1].e[2] = m_in.v[2].e[1]; 
		m_out.v[1].e[3] = m_in.v[3].e[1]; 

		m_out.v[2].e[0] = m_in.v[0].e[2]; 
		m_out.v[2].e[1] = m_in.v[1].e[2]; 
		m_out.v[2].e[2] = m_in.v[2].e[2]; 
		m_out.v[2].e[3] = m_in.v[3].e[2]; 

		m_out.v[3].e[0] = m_in.v[0].e[3];
		m_out.v[3].e[1] = m_in.v[1].e[3];
		m_out.v[3].e[2] = m_in.v[2].e[3];
		m_out.v[3].e[3] = m_in.v[3].e[3];

		return m_out;
	}

	template<typename T>
	inline Mat3<T> transpose(const Mat3<T>& m_in)
	{
		Mat3<T> m_out;
		m_out.v[0].e[0] = m_in.v[0].e[0]; m_out.v[1].e[0] = m_in.v[0].e[1]; m_out.v[2].e[0] = m_in.v[0].e[2];
		m_out.v[0].e[1] = m_in.v[1].e[0]; m_out.v[1].e[1] = m_in.v[1].e[1]; m_out.v[2].e[1] = m_in.v[1].e[2];
		m_out.v[0].e[2] = m_in.v[2].e[0]; m_out.v[1].e[2] = m_in.v[2].e[1]; m_out.v[2].e[2] = m_in.v[2].e[2];
		return m_out;
	}

	template<typename T>
	inline Mat2<T> transpose(const Mat2<T>& m_in)
	{
		Mat2<T> m_out;
		m_out.v[0].e[0] = m_in.v[0].e[0]; m_out.v[1].e[0] = m_in.v[0].e[1];
		m_out.v[0].e[1] = m_in.v[1].e[0]; m_out.v[1].e[1] = m_in.v[1].e[1];
		return m_out;
	}

	// In case there is for sure no skew transformation.
	template<typename T>
	inline Mat4<T> h_inverse(const Mat4<T>& m_in)
	{
		Mat4<T> m_out;

		T len1 = sqrt(m_in.v[0].e[0] * m_in.v[0].e[0] + m_in.v[0].e[1] * m_in.v[0].e[1] + m_in.v[0].e[2] * m_in.v[0].e[2]);
		T len2 = sqrt(m_in.v[1].e[0] * m_in.v[1].e[0] + m_in.v[1].e[1] * m_in.v[1].e[1] + m_in.v[1].e[2] * m_in.v[1].e[2]);
		T len3 = sqrt(m_in.v[2].e[0] * m_in.v[2].e[0] + m_in.v[2].e[1] * m_in.v[2].e[1] + m_in.v[2].e[2] * m_in.v[2].e[2]);

		ERS_ASSERTF(len1 > 0.0f && len2 > 0.0f && len3 > 0.0f, "%s", "Mat4 h_inverse: Degenerate matrix.");

		len1 = 1 / len1;
		len2 = 1 / len2;
		len3 = 1 / len3;

		m_out.v[0].e[0] = m_in.v[0].e[0] * len1; 
		m_out.v[0].e[1] = m_in.v[1].e[0] * len2; 
		m_out.v[0].e[2] = m_in.v[2].e[0] * len3; 
		m_out.v[0].e[3] = T(0);

		m_out.v[1].e[0] = m_in.v[0].e[1] * len1; 
		m_out.v[1].e[1] = m_in.v[1].e[1] * len2; 
		m_out.v[1].e[2] = m_in.v[2].e[1] * len3; 
		m_out.v[1].e[3] = T(0);

		m_out.v[2].e[0] = m_in.v[0].e[2] * len1;
		m_out.v[2].e[1] = m_in.v[1].e[2] * len2;
		m_out.v[2].e[2] = m_in.v[2].e[2] * len3;
		m_out.v[2].e[3] = T(0);

		m_out.v[3].e[0] = -m_out.v[0].e[0] * m_in.v[3].e[0] - m_out.v[1].e[0] * m_in.v[3].e[1] - m_out.v[2].e[0] * m_in.v[3].e[2];
		m_out.v[3].e[1] = -m_out.v[0].e[1] * m_in.v[3].e[0] - m_out.v[1].e[1] * m_in.v[3].e[1] - m_out.v[2].e[1] * m_in.v[3].e[2];
		m_out.v[3].e[2] = -m_out.v[0].e[2] * m_in.v[3].e[0] - m_out.v[1].e[2] * m_in.v[3].e[1] - m_out.v[2].e[2] * m_in.v[3].e[2];
		m_out.v[3].e[3] = T(1);

		return m_out;
	}

	template<typename T>
	inline Mat4<T> inverse(const Mat4<T>& m_in)
	{
		Mat4<T> m_out;

		T tt0 = m_in.v[2].e[2] * m_in.v[3].e[3] - m_in.v[2].e[3] * m_in.v[3].e[2];
		T tt1 = m_in.v[1].e[2] * m_in.v[3].e[3] - m_in.v[1].e[3] * m_in.v[3].e[2];
		T tt2 = m_in.v[1].e[2] * m_in.v[2].e[3] - m_in.v[1].e[3] * m_in.v[2].e[2];
		T tt3 = m_in.v[2].e[1] * m_in.v[3].e[3] - m_in.v[2].e[3] * m_in.v[3].e[1];
		T tt4 = m_in.v[1].e[1] * m_in.v[3].e[3] - m_in.v[1].e[3] * m_in.v[3].e[1];
		T tt5 = m_in.v[1].e[1] * m_in.v[2].e[3] - m_in.v[1].e[3] * m_in.v[2].e[1];
		T tt6 = m_in.v[2].e[1] * m_in.v[3].e[2] - m_in.v[2].e[2] * m_in.v[3].e[1];
		T tt7 = m_in.v[1].e[1] * m_in.v[3].e[2] - m_in.v[1].e[2] * m_in.v[3].e[1];
		T tt8 = m_in.v[1].e[1] * m_in.v[2].e[2] - m_in.v[1].e[2] * m_in.v[2].e[1];

		m_out.v[0].e[0] = m_in.v[1].e[1] * tt0 - m_in.v[2].e[1] * tt1 + m_in.v[3].e[1] * tt2;
		m_out.v[1].e[0] = -m_in.v[1].e[0] * tt0 + m_in.v[2].e[0] * tt1 - m_in.v[3].e[0] * tt2;
		m_out.v[2].e[0] = m_in.v[1].e[0] * tt3 - m_in.v[2].e[0] * tt4 + m_in.v[3].e[0] * tt5;
		m_out.v[3].e[0] = -m_in.v[1].e[0] * tt6 + m_in.v[2].e[0] * tt7 - m_in.v[3].e[0] * tt8;

		T det = m_in.v[0].e[0] * m_out.v[0].e[0] + m_in.v[0].e[1] * m_out.v[1].e[0] + m_in.v[0].e[2] * m_out.v[2].e[0] + m_in.v[0].e[3] * m_out.v[3].e[0];
		ERS_ASSERTF(!ers::equals_epsilon(ers::abs(det), 0.0f), "%s", "Mat4 inverse: Degenerate matrix.");
		det = 1 / det;

		m_out.v[0].e[0] *= det;
		m_out.v[1].e[0] *= det;
		m_out.v[2].e[0] *= det;
		m_out.v[3].e[0] *= det;

		T tt9 = m_in.v[0].e[2] * m_in.v[3].e[3] - m_in.v[0].e[3] * m_in.v[3].e[2];
		T tt10 = m_in.v[0].e[2] * m_in.v[2].e[3] - m_in.v[0].e[3] * m_in.v[2].e[2];
		T tt11 = m_in.v[0].e[1] * m_in.v[3].e[3] - m_in.v[0].e[3] * m_in.v[3].e[1];
		T tt12 = m_in.v[0].e[1] * m_in.v[2].e[3] - m_in.v[0].e[3] * m_in.v[2].e[1];
		T tt13 = m_in.v[0].e[1] * m_in.v[3].e[2] - m_in.v[0].e[2] * m_in.v[3].e[1];
		T tt14 = m_in.v[0].e[1] * m_in.v[2].e[2] - m_in.v[0].e[2] * m_in.v[2].e[1];
		T tt15 = m_in.v[0].e[2] * m_in.v[1].e[3] - m_in.v[0].e[3] * m_in.v[1].e[2];
		T tt16 = m_in.v[0].e[1] * m_in.v[1].e[3] - m_in.v[0].e[3] * m_in.v[1].e[1];
		T tt17 = m_in.v[0].e[1] * m_in.v[1].e[2] - m_in.v[0].e[2] * m_in.v[1].e[1];

		m_out.v[0].e[1] = (-m_in.v[0].e[1] * tt0 + m_in.v[2].e[1] * tt9 - m_in.v[3].e[1] * tt10) * det;
		m_out.v[1].e[1] = (m_in.v[0].e[0] * tt0 - m_in.v[2].e[0] * tt9 + m_in.v[3].e[0] * tt10) * det;
		m_out.v[0].e[2] = (m_in.v[0].e[1] * tt1 - m_in.v[1].e[1] * tt9 + m_in.v[3].e[1] * tt15) * det;
		m_out.v[1].e[2] = (-m_in.v[0].e[0] * tt1 + m_in.v[1].e[0] * tt9 - m_in.v[3].e[0] * tt15) * det;
		m_out.v[0].e[3] = (-m_in.v[0].e[1] * tt2 + m_in.v[1].e[1] * tt10 - m_in.v[2].e[1] * tt15) * det;
		m_out.v[1].e[3] = (m_in.v[0].e[0] * tt2 - m_in.v[1].e[0] * tt10 + m_in.v[2].e[0] * tt15) * det;
		m_out.v[2].e[1] = (-m_in.v[0].e[0] * tt3 + m_in.v[2].e[0] * tt11 - m_in.v[3].e[0] * tt12) * det;
		m_out.v[2].e[2] = (m_in.v[0].e[0] * tt4 - m_in.v[1].e[0] * tt11 + m_in.v[3].e[0] * tt16) * det;
		m_out.v[2].e[3] = (-m_in.v[0].e[0] * tt5 + m_in.v[1].e[0] * tt12 - m_in.v[2].e[0] * tt16) * det;
		m_out.v[3].e[1] = (m_in.v[0].e[0] * tt6 - m_in.v[2].e[0] * tt13 + m_in.v[3].e[0] * tt14) * det;
		m_out.v[3].e[2] = (-m_in.v[0].e[0] * tt7 + m_in.v[1].e[0] * tt13 - m_in.v[3].e[0] * tt17) * det;
		m_out.v[3].e[3] = (m_in.v[0].e[0] * tt8 - m_in.v[1].e[0] * tt14 + m_in.v[2].e[0] * tt17) * det;
		return m_out;
	}

	template<typename T>
	inline Mat3<T> h_inverse(const Mat3<T>& m_in)
	{
		Mat3<T> m_out;

		T len1 = std::sqrt(m_in.v[0].e[0] * m_in.v[0].e[0] + m_in.v[0].e[1] * m_in.v[0].e[1] + m_in.v[0].e[2] * m_in.v[0].e[2]);
		T len2 = std::sqrt(m_in.v[1].e[0] * m_in.v[1].e[0] + m_in.v[1].e[1] * m_in.v[1].e[1] + m_in.v[1].e[2] * m_in.v[1].e[2]);
		T len3 = std::sqrt(m_in.v[2].e[0] * m_in.v[2].e[0] + m_in.v[2].e[1] * m_in.v[2].e[1] + m_in.v[2].e[2] * m_in.v[2].e[2]);

		ERS_ASSERTF(len1 > T(0) && len2 > T(0) && len3 > T(0), "%s", "Mat3 h_inverse: Degenerate matrix.");

		len1 = T(1) / len1;
		len2 = T(1) / len2;
		len3 = T(1) / len3;

		m_out.v[0].e[0] = m_in.v[0].e[0] * len1;
		m_out.v[0].e[1] = m_in.v[1].e[0] * len2;
		m_out.v[0].e[2] = m_in.v[2].e[0] * len3;

		m_out.v[1].e[0] = m_in.v[0].e[1] * len1;
		m_out.v[1].e[1] = m_in.v[1].e[1] * len2;
		m_out.v[1].e[2] = m_in.v[2].e[1] * len3;

		m_out.v[2].e[0] = m_in.v[0].e[2] * len1;
		m_out.v[2].e[1] = m_in.v[1].e[2] * len2;
		m_out.v[2].e[2] = m_in.v[2].e[2] * len3;

		return m_out;
	}

	template<typename T>
	inline Mat3<T> inverse(const Mat3<T>& m_in)
	{
		Mat3<T> m_out;
		T t0 = m_in.v[1].e[1] * m_in.v[2].e[2] - m_in.v[1].e[2] * m_in.v[2].e[1];
		T t1 = m_in.v[1].e[2] * m_in.v[2].e[0] - m_in.v[1].e[0] * m_in.v[2].e[2];
		T t2 = m_in.v[1].e[0] * m_in.v[2].e[1] - m_in.v[1].e[1] * m_in.v[2].e[0];

		T det = m_in.v[0].e[0] * t0 + m_in.v[0].e[1] * t1 + m_in.v[0].e[2] * t2;
		// ERS_ASSERTF(!ers::equals_epsilon(ers::abs(det), 0.0f), "%s", "Mat3 inverse: Degenerate matrix.");
		det = 1 / det;

		m_out.v[0].e[0] = t0 * det;  m_out.v[1].e[0] = t1 * det; m_out.v[2].e[0] = t2 * det;
		m_out.v[0].e[1] = (m_in.v[0].e[2] * m_in.v[2].e[1] - m_in.v[0].e[1] * m_in.v[2].e[2]) * det;
		m_out.v[1].e[1] = (m_in.v[0].e[0] * m_in.v[2].e[2] - m_in.v[0].e[2] * m_in.v[2].e[0]) * det;
		m_out.v[2].e[1] = (m_in.v[0].e[1] * m_in.v[2].e[0] - m_in.v[0].e[0] * m_in.v[2].e[1]) * det;
		m_out.v[0].e[2] = (m_in.v[0].e[1] * m_in.v[1].e[2] - m_in.v[0].e[2] * m_in.v[1].e[1]) * det;
		m_out.v[1].e[2] = (m_in.v[0].e[2] * m_in.v[1].e[0] - m_in.v[0].e[0] * m_in.v[1].e[2]) * det;
		m_out.v[2].e[2] = (m_in.v[0].e[0] * m_in.v[1].e[1] - m_in.v[0].e[1] * m_in.v[1].e[0]) * det;
		return m_out;
	}

	template<typename T>
	inline Mat2<T> inverse(const Mat2<T>& m_in)
	{
		Mat2<T> m_out;

		T det = m_in.v[0].e[0] * m_in.v[1].e[1] - m_in.v[0].e[1] * m_in.v[1].e[0];
		ERS_ASSERTF(det > 0.0f, "%s", "Mat2 inverse: Degenerate matrix.");
		det = 1 / det;

		m_out.v[0].e[0] = m_in.v[1].e[1] * det; m_out.v[1].e[0] = -m_in.v[1].e[0] * det;
		m_out.v[0].e[1] = -m_in.v[0].e[1] * det; m_out.v[1].e[1] = m_in.v[0].e[0] * det;
		return m_out;
	}

	template<typename T>
	inline Mat4<T> translate(const Mat4<T>& m_in, const Vec3<T>& d)
	{
		Mat4<T> result;

		result.v[0].e[0] = m_in.v[0].e[0] + m_in.v[0].e[3] * d.e[0];
		result.v[0].e[1] = m_in.v[0].e[1] + m_in.v[0].e[3] * d.e[1];
		result.v[0].e[2] = m_in.v[0].e[2] + m_in.v[0].e[3] * d.e[2];
		result.v[0].e[3] = m_in.v[0].e[3];

		result.v[1].e[0] = m_in.v[1].e[0] + m_in.v[1].e[3] * d.e[0];
		result.v[1].e[1] = m_in.v[1].e[1] + m_in.v[1].e[3] * d.e[1];
		result.v[1].e[2] = m_in.v[1].e[2] + m_in.v[1].e[3] * d.e[2];
		result.v[1].e[3] = m_in.v[1].e[3];

		result.v[2].e[0] = m_in.v[2].e[0] + m_in.v[2].e[3] * d.e[0];
		result.v[2].e[1] = m_in.v[2].e[1] + m_in.v[2].e[3] * d.e[1];
		result.v[2].e[2] = m_in.v[2].e[2] + m_in.v[2].e[3] * d.e[2];
		result.v[2].e[3] = m_in.v[2].e[3];

		result.v[3].e[0] = m_in.v[3].e[0] + m_in.v[3].e[3] * d.e[0];
		result.v[3].e[1] = m_in.v[3].e[1] + m_in.v[3].e[3] * d.e[1];
		result.v[3].e[2] = m_in.v[3].e[2] + m_in.v[3].e[3] * d.e[2];
		result.v[3].e[3] = m_in.v[3].e[3];

		return result;
	}

	template<typename T>
	inline Mat3<T> scale(const Mat3<T>& m_in, const Vec3<T>& s)
	{
		Mat3<T> result;

		result.v[0].e[0] = m_in.v[0].e[0] * s.e[0];
		result.v[0].e[1] = m_in.v[0].e[1] * s.e[1];
		result.v[0].e[2] = m_in.v[0].e[2] * s.e[2];

		result.v[1].e[0] = m_in.v[1].e[0] * s.e[0];
		result.v[1].e[1] = m_in.v[1].e[1] * s.e[1];
		result.v[1].e[2] = m_in.v[1].e[2] * s.e[2];

		result.v[2].e[0] = m_in.v[2].e[0] * s.e[0];
		result.v[2].e[1] = m_in.v[2].e[1] * s.e[1];
		result.v[2].e[2] = m_in.v[2].e[2] * s.e[2];

		return result;
	}

	template<typename T>
	inline Mat4<T> scale(const Mat4<T>& m_in, const Vec3<T>& s)
	{
		Mat4<T> result;

		result.v[0].e[0] = m_in.v[0].e[0] * s.e[0];
		result.v[0].e[1] = m_in.v[0].e[1] * s.e[1];
		result.v[0].e[2] = m_in.v[0].e[2] * s.e[2];
		result.v[0].e[3] = m_in.v[0].e[3];

		result.v[1].e[0] = m_in.v[1].e[0] * s.e[0];
		result.v[1].e[1] = m_in.v[1].e[1] * s.e[1];
		result.v[1].e[2] = m_in.v[1].e[2] * s.e[2];
		result.v[1].e[3] = m_in.v[1].e[3];

		result.v[2].e[0] = m_in.v[2].e[0] * s.e[0];
		result.v[2].e[1] = m_in.v[2].e[1] * s.e[1];
		result.v[2].e[2] = m_in.v[2].e[2] * s.e[2];
		result.v[2].e[3] = m_in.v[2].e[3];

		result.v[3].e[0] = m_in.v[3].e[0] * s.e[0];
		result.v[3].e[1] = m_in.v[3].e[1] * s.e[1];
		result.v[3].e[2] = m_in.v[3].e[2] * s.e[2];
		result.v[3].e[3] = m_in.v[3].e[3];

		return result;
	}

	template<typename T>
	inline Mat4<T> rotate(const Mat4<T>& m_in, const T& angle, Vec3<T> axis)
	{
		axis = normalize(axis);
		const T c = cos(angle); const T s = sin(angle); const T one_minus_c = T(1) - cos(angle);
		const T xs = axis.e[0] * s;
		const T ys = axis.e[1] * s; const T yc = axis.e[1] * one_minus_c;
		const T zs = axis.e[2] * s; const T zc = axis.e[2] * one_minus_c;

		Mat4<T> rot;

		rot.v[0].e[0] = axis.e[0] * axis.e[0] * one_minus_c + c;
		rot.v[0].e[1] = axis.e[0] * yc;
		rot.v[0].e[2] = axis.e[0] * zc;
		rot.v[0].e[3] = T(0);

		rot.v[1].e[0] = T(0);
		rot.v[1].e[1] = axis.e[1] * yc + c;
		rot.v[1].e[2] = axis.e[1] * zc;
		rot.v[1].e[3] = T(0);

		rot.v[2].e[0] = T(0);
		rot.v[2].e[1] = T(0);
		rot.v[2].e[2] = axis.e[2] * zc + c;
		rot.v[2].e[3] = T(0);

		rot.v[3].e[0] = T(0);
		rot.v[3].e[1] = T(0);
		rot.v[3].e[2] = T(0);
		rot.v[3].e[3] = T(1);

		rot.v[1].e[0] += rot.v[0].e[1] - zs;  rot.v[0].e[1] += zs;
		rot.v[2].e[0] += rot.v[0].e[2] + ys;  rot.v[0].e[2] -= ys;
		rot.v[2].e[1] += rot.v[1].e[2] - xs;  rot.v[1].e[2] += xs;

		return rot * m_in;
	}

	template<typename T>
	inline Mat4<T> perspective(const T& fov, const T& reciproc_aspect_ratio, const T& z_near, const T& z_far)
	{
		ERS_ASSERTF(fov < ers::PI<T>::val(), "%s", "Mat4 perspective.");
		const T reciproc_tan = T(1) / tan(T(0.5) * fov);
		const T reciproc_dz = T(1) / (z_near - z_far);

		Mat4<T> result;

		result.v[0].e[0] = reciproc_aspect_ratio * reciproc_tan;
		result.v[0].e[1] = T(0);
		result.v[0].e[2] = T(0);
		result.v[0].e[3] = T(0);

		result.v[1].e[0] = T(0);
		result.v[1].e[1] = reciproc_tan;
		result.v[1].e[2] = T(0);
		result.v[1].e[3] = T(0);

		result.v[2].e[0] = T(0);
		result.v[2].e[1] = T(0);
		result.v[2].e[2] = reciproc_dz * (z_near + z_far);
		result.v[2].e[3] = T(-1);

		result.v[3].e[0] = T(0);
		result.v[3].e[1] = T(0);
		result.v[3].e[2] = T(2) * z_near * z_far * reciproc_dz;
		result.v[3].e[3] = T(0);

		return result;
	}

	template<typename T>
	inline Mat4<T> perspective(T l, T r, T t, T b, T n, T f)
	{
		ERS_ASSERTF(n < f, "%s", "Mat4 perspective.");
		const T rx = T(1) / (r - l);
		const T ry = T(1) / (t - b);
		const T rz = T(1) / (n - f);
		const T n2 = T(2) * n;

		Mat4<T> result;

		result.v[0].e[0] = rx * n2;
		result.v[0].e[1] = T(0);
		result.v[0].e[2] = T(0);
		result.v[0].e[3] = T(0);

		result.v[1].e[0] = T(0);
		result.v[1].e[1] = ry * n2;
		result.v[1].e[2] = T(0);
		result.v[1].e[3] = T(0);

		result.v[2].e[0] = rx * (r + l);
		result.v[2].e[1] = ry * (t + b);
		result.v[2].e[2] = rz * (f + n);
		result.v[2].e[3] = T(-1);

		result.v[3].e[0] = T(0);
		result.v[3].e[1] = T(0);
		result.v[3].e[2] = rz * n2 * f;
		result.v[3].e[3] = T(0);

		return result;
	}

	template<typename T>
	inline Mat4<T> ortho(const T& x0, const T& xf, const T& y0, const T& yf, const T& z0, const T& zf)
	{
		const T reciprocal_dx = T(1) / (xf - x0);
		const T reciprocal_dy = T(1) / (yf - y0);
		const T reciprocal_dz = T(1) / (z0 - zf);

		Mat4<T> result;

		result.v[0].e[0] = T(2) * reciprocal_dx;
		result.v[0].e[1] = T(0);
		result.v[0].e[2] = T(0);
		result.v[0].e[3] = T(0);

		result.v[1].e[0] = T(0);
		result.v[1].e[1] = T(2) * reciprocal_dy;
		result.v[1].e[2] = T(0);
		result.v[1].e[3] = T(0);

		result.v[2].e[0] = T(0);
		result.v[2].e[1] = T(0);
		result.v[2].e[2] = T(2) * reciprocal_dz;
		result.v[2].e[3] = T(0);

		result.v[3].e[0] = -(x0 + xf) * reciprocal_dx;
		result.v[3].e[1] = -(y0 + yf) * reciprocal_dy;
		result.v[3].e[2] = (z0 + zf) * reciprocal_dz;
		result.v[3].e[3] = T(1);

		return result;
	}

	template<typename T>
	inline Mat4<T> ortho(const T& x0, const T& xf, const T& y0, const T& yf)
	{
		const T reciprocal_dx = T(1) / (xf - x0);
		const T reciprocal_dy = T(1) / (yf - y0);

		Mat4<T> result;

		result.v[0].e[0] = T(2) * reciprocal_dx;
		result.v[0].e[1] = T(0);
		result.v[0].e[2] = T(0);
		result.v[0].e[3] = T(0);

		result.v[1].e[0] = T(0);
		result.v[1].e[1] = T(2) * reciprocal_dy;
		result.v[1].e[2] = T(0);
		result.v[1].e[3] = T(0);

		result.v[2].e[0] = T(0);
		result.v[2].e[1] = T(0);
		result.v[2].e[2] = T(-1);
		result.v[2].e[3] = T(0);

		result.v[3].e[0] = -(x0 + xf) * reciprocal_dx;
		result.v[3].e[1] = -(y0 + yf) * reciprocal_dy;
		result.v[3].e[2] = T(0);
		result.v[3].e[3] = T(1);

		return result;
	}

	template<typename T>
	inline Mat4<T> lookAt(const Vec3<T>& position, const Vec3<T>& target, Vec3<T> up)
	{
		const Vec3<T> direction = normalize(position - target);
		const Vec3<T> right = normalize(cross(normalize(up), direction));
		up = cross(direction, right);

		Mat4<T> result;

		result.v[0].e[0] = right.e[0];
		result.v[0].e[1] = up.e[0];
		result.v[0].e[2] = direction.e[0];
		result.v[0].e[3] = T(0);

		result.v[1].e[0] = right.e[1];
		result.v[1].e[1] = up.e[1];
		result.v[1].e[2] = direction.e[1];
		result.v[1].e[3] = T(0);

		result.v[2].e[0] = right.e[2];
		result.v[2].e[1] = up.e[2];
		result.v[2].e[2] = direction.e[2];
		result.v[2].e[3] = T(0);

		result.v[3].e[0] = -dot(right, position);
		result.v[3].e[1] = -dot(up, position);
		result.v[3].e[2] = -dot(direction, position);
		result.v[3].e[3] = T(1);

		return result;
	}
}

#endif // LINALG_H