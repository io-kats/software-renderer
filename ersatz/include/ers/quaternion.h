#ifndef QUATERNION_H
#define QUATERNION_H

#include "./ers/common.h"
#include "./ers/matrix.h"
#include "./ers/vec.h"

namespace ers
{
	// ----------------------------
	// :::::::::::VEC 4::::::::::::
	// ----------------------------

	template<typename T>
	struct Quat {
		T x;
		T y;
		T z;
		T w;

		Quat() = default;
		Quat(T w_) : x(T(0)), y(T(0)), z(T(0)), w(w_) { }
		Quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) { }
		Quat(const ers::Vec3<T>& v, T w_) : x(v.e[0]), y(v.e[1]), z(v.e[2]), w(w_) { }

		static inline Quat identity()
		{
			return Quat(T(1));
		}

		static inline Quat make_rotate(const T& angle, Vec3<T> v)
		{
			v = ers::normalize(v);
			const T half_angle = T(0.5) * angle;
			const T s = sin(half_angle);
			return Quat(s * v.x, s * v.y, s * v.z, cos(half_angle));
		}

		inline Quat<T> operator +(const Quat<T>& q2) const { return Quat<T>(x + q2.x, y + q2.y, z + q2.z, w + q2.w); }	
		inline Quat<T> operator *(const T& s) const { return Quat<T>(x * s, y * s, z * s, w * s); }
		inline Quat<T>& operator +=(const Quat<T>& q2) { x += q2.x; y += q2.y; z += q2.z; w += q2.w; return *this; }
		inline Quat<T>& operator *=(const T& s) { x *= s; y *= s; z *= s; w *= s; return *this; }

		inline Quat<T> operator *(const Quat<T>& q2) const
		{
			return Quat<T>(y * q2.z - z * q2.y + q2.w * x + w * q2.x,
				           z * q2.x - x * q2.z + q2.w * y + w * q2.y,
						   x * q2.y - y * q2.x + q2.w * z + w * q2.z,
						   w * q2.w - x * q2.x - y * q2.y - z * q2.z);
		}

		inline Quat<T>& operator /=(T s)
		{
			ERS_ASSERT(abs(s) > eps<T>::val());
			s = T(1) / s;
			x *= s; y *= s; z *= s; w *= s;
			return *this;
		}

		inline Quat<T> operator /(T s)  const
		{
			ERS_ASSERT(abs(s) > eps<T>::val());
			s = T(1) / s;
			return Quat<T>(x * s, y * s, z * s, w * s);
		}

		inline bool operator ==(const Quat<T>& v2) const
		{
			return abs(x - v2.x) <= ers::eps<T>::val()
				&& abs(y - v2.y) <= ers::eps<T>::val()
				&& abs(z - v2.z) <= ers::eps<T>::val()
				&& abs(w - v2.w) <= ers::eps<T>::val();
		}

		inline bool operator !=(const Quat<T>& v2) const
		{
			return abs(x - v2.x) > ers::eps<T>::val()
				|| abs(y - v2.y) > ers::eps<T>::val()
				|| abs(z - v2.z) > ers::eps<T>::val()
				|| abs(w - v2.w) > ers::eps<T>::val();
		}	

		inline void print()
		{
			printf("[%f, %f, %f, %f]\n", x, y, z, w);
		}
	};

	template<typename T>
	inline Quat<T> operator*(const T& s, const Quat<T>& q)
	{
		return Quat<T>(q.x * s, q.y * s, q.z * s, q.w * s);
	}

	template<typename T>
	inline Quat<T> conjugate(const Quat<T>& q)
	{
		return Quat<T>(-q.x, -q.y, -q.z, q.w);
	}

	template<typename T>
	inline T norm2(const Quat<T>& q)
	{
		return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	}

	template<typename T>
	inline T norm(const Quat<T>& q)
	{
		return sqrt(norm2(q));
	}

	template<typename T>
	inline Quat<T> inverse(const Quat<T>& q)
	{
		T n2 = norm2(q);
		ERS_ASSERTF(n2 > ers::eps<T>::val(), "%s", "Quat::inverse: norm is almost zero.");
		n2 = T(1) / n2;
		return n2 * conjugate(q);
	}

	// Does not assume unit quaternion, produces orthogonal matrix.
	template<typename T>
	inline Mat3<T> quatToMat3(const Quat<T>& q)
	{
		const T xx = q.x * q.x; const T yy = q.y * q.y; const T zz = q.z * q.z;
		T s = xx + yy + zz + q.w * q.w;
		ERS_ASSERTF(s > 0.0f, "%s", "quatToMat4: norm is almost zero.");
		s = T(2) / s;
		const T xy = q.x * q.y; const T xz = q.x * q.z; const T xw = q.x * q.w;
		const T yz = q.y * q.z; const T yw = q.y * q.w;
		const T zw = q.z * q.w;

		Mat3<T> m_out;
		m_out.v[0].x() = T(1) - s * (yy + zz);   m_out.v[1].x() =        s * (xy - zw); m_out.v[2].x() = 		s * (xz + yw);
		m_out.v[0].y() = 		  s * (xy + zw); m_out.v[1].y() = T(1) - s * (xx + zz); m_out.v[2].y() =        s * (yz - xw);
		m_out.v[0].z() = 		  s * (xz - yw); m_out.v[1].z() =        s * (yz + xw); m_out.v[2].z() = T(1) - s * (xx + yy);

		return m_out;
	}

	template<typename T>
	inline Mat4<T> quatToMat4(const Quat<T>& q)
	{
		const T xx = q.x * q.x; const T yy = q.y * q.y; const T zz = q.z * q.z;
		T s = xx + yy + zz + q.w * q.w;
		ERS_ASSERTF(s > 0.0f, "%s", "quatToMat4: norm is almost zero.");
		s = T(2) / s;
		const T xy = q.x * q.y; const T xz = q.x * q.z; const T xw = q.x * q.w;
		const T yz = q.y * q.z; const T yw = q.y * q.w;
		const T zw = q.z * q.w;

		Mat4<T> m_out;
		m_out.v[0].x() = T(1) - s * (yy + zz);   m_out.v[1].x() =        s * (xy - zw); m_out.v[2].x() = 		s * (xz + yw); m_out.v[3].x() = T(0);
		m_out.v[0].y() = 		  s * (xy + zw); m_out.v[1].y() = T(1) - s * (xx + zz); m_out.v[2].y() =        s * (yz - xw); m_out.v[3].y() = T(0);
		m_out.v[0].z() = 		  s * (xz - yw); m_out.v[1].z() =        s * (yz + xw); m_out.v[2].z() = T(1) - s * (xx + yy); m_out.v[3].z() = T(0);
		m_out.v[0].w() = 				   T(0); m_out.v[1].w() = 				T(0);   m_out.v[2].w() = 				 T(0); m_out.v[3].w() = T(1);

		return m_out;
	}

	// Assumes orthogonal matrix, produces unit quaternion.
	// TODO: Avoid doing all square roots.
	template<typename T>
	inline Quat<T> mat4ToQuat(const Mat4<T>& m)
	{
		Quat<T> q_out;
		q_out.x = T(0.5) * sqrt( m.v[0].x() - m.v[1].y() - m.v[2].z() + m.v[3].w());
		q_out.y = T(0.5) * sqrt(-m.v[0].x() + m.v[1].y() - m.v[2].z() + m.v[3].w());
		q_out.z = T(0.5) * sqrt(-m.v[0].x() - m.v[1].y() + m.v[2].z() + m.v[3].w());
		q_out.w = T(0.5) * sqrt( m.v[0].x() + m.v[1].y() + m.v[2].z() + m.v[3].w());
		return q_out;
	}

	template<typename T>
	inline Quat<T> slerp(const Quat<T>& q, const Quat<T>& r, const T& t)
	{
		const T phi = acos(q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w);
		ERS_ASSERTF(phi > T(0) && phi < ers::PI<T>::val(), "%s", "slerp: Quaternions (anti-)parallel.");
		const T rs = T(1) / sin(phi);
		
		return (q * sin(phi * (1 - t)) + r * sin(phi * t)) * rs;
	}

	template<typename T>
	inline Quat<T> rotate(const Quat<T>& q, const T& angle, Vec3<T> v)
	{
		v = ers::normalize(v);
		const T half_angle = T(0.5) * angle;
		const T s = sin(half_angle);
		Quat<T> r(s * v.x(), s * v.y(), s * v.z(), cos(half_angle));
		return r * q;
	}

	typedef Quat<float> quat;
}

#endif // QUATERNION_H