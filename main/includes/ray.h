#ifndef RAY_H
#define RAY_H

#include "ers/common.h"
#include "ers/vec.h"

// Simple ray structure, defined by an origin and a direction.
// A point is on the ray if the following holds for the parameter t:
// t >= 0 AND t_min <= t <= t_max (implying: t_min, t_max >= 0)
struct Ray
{
    ers::vec3 orig;
    f32 t_min;
    ers::vec3 dir;
    f32 t_max;

    ers::vec3 At(f32 t) const; // helper method to calculated a point along the ray.
};

bool closest_points_between_two_rays(const Ray& r1, const Ray& r2, f32& t1, f32& t2);

// Math helpers for uniformly distributed random point on/in a sphere/hemisphere/disc.
// Please provide (uniformly distributed) random numbers as arguments.
ers::vec3 random_point_on_sphere(f32 r1, f32 r2);
ers::vec3 random_point_in_sphere(f32 r1, f32 r2, f32 r3);
ers::vec3 random_point_on_hemisphere(const ers::vec3& n, f32 r1, f32 r2); // n: the direction of the hemisphere.
ers::vec2 random_point_on_circle(f32 r);
ers::vec2 random_point_in_disk(f32 r1, f32 r2);

#endif // RAY_H