#ifndef VEC_H
#define VEC_H

#include "./ers/common.h"
#include "./ers/linalg.h"

// Vector instatiations
// TODO: split into seperate headers, since I don't always need all of these.
namespace ers
{
    typedef Vec2<f32> vec2; static_assert(sizeof(vec2) == 2 * sizeof(f32), "vec2: Struct layout problem.");
	typedef Vec3<f32> vec3; static_assert(sizeof(vec3) == 3 * sizeof(f32), "vec3: Struct layout problem.");
	typedef Vec4<f32> vec4; static_assert(sizeof(vec4) == 4 * sizeof(f32), "vec4: Struct layout problem.");

	typedef Vec2<f64> dvec2; static_assert(sizeof(dvec2) == 2 * sizeof(f64), "dvec2: Struct layout problem.");
	typedef Vec3<f64> dvec3; static_assert(sizeof(dvec3) == 3 * sizeof(f64), "dvec3: Struct layout problem.");
	typedef Vec4<f64> dvec4; static_assert(sizeof(dvec4) == 4 * sizeof(f64), "dvec4: Struct layout problem.");

	typedef Vec2<s32> ivec2; static_assert(sizeof(ivec2) == 2 * sizeof(s32), "ivec2: Struct layout problem.");
	typedef Vec3<s32> ivec3; static_assert(sizeof(ivec3) == 3 * sizeof(s32), "ivec3: Struct layout problem.");
	typedef Vec4<s32> ivec4; static_assert(sizeof(ivec4) == 4 * sizeof(s32), "ivec4: Struct layout problem.");

	typedef Vec2<u32> uivec2; static_assert(sizeof(uivec2) == 2 * sizeof(u32), "uivec2: Struct layout problem.");
	typedef Vec3<u32> uivec3; static_assert(sizeof(uivec3) == 3 * sizeof(u32), "uivec3: Struct layout problem.");
	typedef Vec4<u32> uivec4; static_assert(sizeof(uivec4) == 4 * sizeof(u32), "uivec4: Struct layout problem.");
}

#endif // VEC_H