#include "ray.h"

ers::vec3 Ray::At(f32 t) const
{
    return orig + dir * t;
}

bool closest_points_between_two_rays(const Ray& r1, const Ray& r2, f32& t1, f32& t2)
{
    const ers::vec3 d0 = r2.orig - r1.orig;
    const f32 a = ers::length2(r1.dir);
    const f32 b = ers::dot(r1.dir, r2.dir);
    const f32 c = -ers::length2(r2.dir);
    const f32 e = ers::dot(r1.dir, d0);
    f32 det = a * c + b * b;
    bool result = !ers::equals_epsilon(det, 0.0f);
    if (result)
    {
        const f32 f = ers::dot(r2.dir, d0);
        det = 1 / det;
        t1 = (e * c + f * b) * det;
        t2 = (a * f - b * e) * det;
    }
    else // Rays are almost parallel. Only one equation for t1 and t2. Pick any solution.
    {
        t2 = 0.0f;
        t1 = e / a;
    }

    return result;
}

ers::vec3 random_point_on_sphere(f32 r1, f32 r2)
{
    const f32 theta = ers::TAU<f32>::val() * r1;
    const f32 u = 2.0f * r2 - 1.0f;
    const f32 t = sqrt(1.0f - u * u);
    return ers::vec3(t * cosf(theta), t * sinf(theta), u);
}

ers::vec3 random_point_in_sphere(f32 r1, f32 r2, f32 r3)
{
    return random_point_on_sphere(r1, r2) * r3;
}

ers::vec3 random_point_on_hemisphere(const ers::vec3& n, f32 r1, f32 r2)
{
    ers::vec3 result = random_point_on_sphere(r1, r2);
    const f32 dot_rn = ers::dot(result, n);
    if (dot_rn < 0.0f) result = -result;
    return result;
}

ers::vec2 random_point_on_circle(f32 r)
{
    const f32 theta = ers::TAU<f32>::val() * r;
    return ers::vec2(cosf(theta), sinf(theta));
}

ers::vec2 random_point_in_disk(f32 r1, f32 r2)
{    
    return random_point_on_circle(r1) * r2;
}