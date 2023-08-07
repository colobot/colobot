/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#pragma once

#include <glm/glm.hpp>

namespace Math
{

struct Sphere
{
    Sphere(const glm::vec3& pos = glm::vec3(0, 0, 0), float radius = 0.0f)
        : pos(pos), radius(radius) {}

    glm::vec3 pos;
    float radius;
};

//! Compute distance between given \a point and \a sphere
inline float DistanceToSphere(const glm::vec3& point, const Sphere& sphere)
{
    return glm::distance(point, sphere.pos) - sphere.radius;
}

inline float DistanceBetweenSpheres(const Sphere& sphere1, const Sphere& sphere2)
{
    return glm::distance(sphere1.pos, sphere2.pos) - sphere1.radius - sphere2.radius;
}

inline Sphere BoundingSphereForBox(glm::vec3 mins, glm::vec3 maxs)
{
    auto centroid = (maxs + mins) / 2.0f;
    auto halfExtent = (maxs - centroid);
    return Sphere{centroid, glm::length(halfExtent)};
}

} // namespace Math
