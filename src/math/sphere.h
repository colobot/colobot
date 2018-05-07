/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "math/vector.h"

namespace Math
{

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
