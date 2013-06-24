// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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
 * \file graphics/core/light.h
 * \brief Light struct and related enums
 */

#pragma once


#include "graphics/core/color.h"

#include "math/vector.h"


// Graphics module namespace
namespace Gfx {

/**
 * \enum LightType
 * \brief Type of light in 3D scene
 */
enum LightType
{
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_DIRECTIONAL
};

/**
 * \struct Light
 * \brief Properties of light in 3D scene
 *
 * This structure was created as analog to DirectX's D3DLIGHT.
 */
struct Light
{
    //! Type of light source
    LightType  type;
    //! Color of ambient light
    Color      ambient;
    //! Color of diffuse light
    Color      diffuse;
    //! Color of specular light
    Color      specular;
    //! Position in world space (for point & spot lights)
    Math::Vector    position;
    //! Direction in world space (for directional & spot lights)
    Math::Vector    direction;
    //! Constant attenuation factor
    float           attenuation0;
    //! Linear attenuation factor
    float           attenuation1;
    //! Quadratic attenuation factor
    float           attenuation2;
    //! Angle of spotlight cone (0-PI/2 radians)
    float           spotAngle;

    //! Intensity of spotlight (0 = uniform; 128 = most intense)
    float           spotIntensity;

    //! Constructor; calls LoadDefault()
    Light()
    {
        LoadDefault();
    }

    //! Loads default values
    void LoadDefault()
    {
        type = LIGHT_POINT;
        ambient = Color(0.4f, 0.4f, 0.4f);
        diffuse = Color(0.8f, 0.8f, 0.8f);
        specular = Color(1.0f, 1.0f, 1.0f);
        position = Math::Vector(0.0f, 0.0f, 0.0f);
        direction = Math::Vector(0.0f, 0.0f, 1.0f);
        attenuation0 = 1.0f;
        attenuation1 = attenuation2 = 0.0f;
        spotAngle = Math::PI/2.0f;
        spotIntensity = 0.0f;
    }
};


} // namespace Gfx

