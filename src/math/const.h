// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file math/const.h
 * \brief Constants used in math functions
 */

#pragma once


#include <cmath>


// Math module namespace
namespace Math {


//! Tolerance level -- minimum accepted float value
const float TOLERANCE = 1e-6f;

//! Very small number (used in testing/returning some values)
const float VERY_SMALL_NUM = 1e-6f;
//! Very big number (used in testing/returning some values)
const float VERY_BIG_NUM = 1e6f;

//! Huge number
const float HUGE_NUM = 1.0e+38f;

//! PI
const float PI       = 3.14159265358979323846f;

//! Degrees to radians multiplier
const float DEG_TO_RAD =  0.01745329251994329547f;
//! Radians to degrees multiplier
const float RAD_TO_DEG = 57.29577951308232286465f;

//! Natural logarithm of 2
const float LOG_2 = log(2.0f);


} // namespace Math

