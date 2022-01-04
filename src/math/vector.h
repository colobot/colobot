/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file math/vector.h
 * \brief Vector struct and related functions
 */

#pragma once


#include "math/const.h"
#include "math/func.h"


#include <cmath>
#include <sstream>
#include <glm/glm.hpp>

// Math module namespace
namespace Math
{

//! Checks if two vectors are equal within given \a tolerance
inline bool VectorsEqual(const glm::vec3 &a, const glm::vec3 &b, float tolerance = TOLERANCE)
{
    return IsEqual(a.x, b.x, tolerance)
            && IsEqual(a.y, b.y, tolerance)
            && IsEqual(a.z, b.z, tolerance);
}

//! Convenience function for getting normalized vector
inline glm::vec3 Normalize(const glm::vec3 &v)
{
    return glm::normalize(v);
}

//! Convenience function for calculating dot product
inline float DotProduct(const glm::vec3 &left, const glm::vec3 &right)
{
    return glm::dot(left, right);
}

//! Convenience function for calculating cross product
inline glm::vec3 CrossProduct(const glm::vec3 &left, const glm::vec3 &right)
{
    return glm::cross(left, right);
}

//! Convenience function for calculating angle (in radians) between two vectors
inline float Angle(const glm::vec3 &a, const glm::vec3 &b)
{
    return std::acosf(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
}

//! Returns the distance between the ends of two vectors
inline float Distance(const glm::vec3 &a, const glm::vec3 &b)
{
    return glm::distance(a, b);
}

//! Clamps the vector \a vec to range between \a min and \a max
inline glm::vec3 Clamp(const glm::vec3&vec, const glm::vec3&min, const glm::vec3&max)
{
    return glm::clamp(vec, min, max);
}

inline std::string ToString(const glm::vec3& vector)
{
    std::stringstream s;
    s.precision(3);
    s << "[" << vector.x << ", " << vector.y << ", " << vector.z << "]";
    return s.str();
}


} // namespace Math

