#pragma once

#include "math/vector.h"

namespace Math {

struct Sphere
{
    Sphere(const Vector& pos = Vector(), float radius = 0.0f)
        : pos(pos), radius(radius) {}

    Vector pos;
    float radius;
};

//! Compute distance between given \a point and \a sphere
inline float DistanceToSphere(const Vector& point, const Sphere& sphere)
{
    return Math::Distance(point, sphere.pos) - sphere.radius;
}

inline float DistanceBetweenSpheres(const Sphere& sphere1, const Sphere& sphere2)
{
    return Math::Distance(sphere1.pos, sphere2.pos) - sphere1.radius - sphere2.radius;
}

} // namespace Math
