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
 * \file math/matrix.h
 * \brief Matrix struct and related functions
 */

#pragma once


#include "math/const.h"
#include "math/func.h"


#include <cmath>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


// Math module namespace
namespace Math
{

using Matrix = glm::mat4;


//! Calculates the result of multiplying m * v
/**
    The multiplication is performed thus:

\verbatim
[  m.m[0 ] m.m[4 ] m.m[8 ] m.m[12]  ]   [ v.x ]
[  m.m[1 ] m.m[5 ] m.m[9 ] m.m[13]  ]   [ v.y ]
[  m.m[2 ] m.m[6 ] m.m[10] m.m[14]  ] * [ v.z ]
[  m.m[3 ] m.m[7 ] m.m[11] m.m[15]  ]   [  1  ]
\endverbatim

   The result, a 4x1 vector is then converted to 3x1 by dividing
   x,y,z coords by the fourth coord (w). */
inline glm::vec3 MatrixVectorMultiply(const Math::Matrix &m, const glm::vec3 &v, bool wDivide = false)
{
    glm::vec4 result = m * glm::vec4(v, 1.0);

    if (!wDivide)
        return result;

    if (IsZero(result.w))
        return glm::vec3(result);

    return glm::vec3(result) / result.w;
}


} // namespace Math
