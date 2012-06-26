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

// light.h

#pragma once


#include "graphics/common/color.h"
#include "math/vector.h"


namespace Gfx {

/** \enum LightType
 *  \brief Type of light */
enum LightType
{
    LT_Point,
    LT_Spot,
    LT_Directional
};

/**
 * \struct Light
 * \brief Light
 *
 * This structure was created as analog to DirectX's D3DLIGHT.
 *
 * It contains analogous fields as the D3DLIGHT struct.
 */
struct Light
{
    //! Type of light source
    Gfx::LightType      type;
    //! Color of light
    Gfx::Color          color;
    //! Position in world space
    Math::Vector    position;
    //! Direction in world space
    Math::Vector    direction;
    //! Cutoff range
    float           range;
    //! Falloff
    float           falloff;
    //! Constant attenuation
    float           attenuation0;
    //! Linear attenuation
    float           attenuation1;
    //! Quadratic attenuation
    float           attenuation2;
    //! Inner angle of spotlight cone
    float           theta;
    //! Outer angle of spotlight cone
    float           phi;

    Light() : type(LT_Point), range(0.0f), falloff(0.0f),
              attenuation0(0.0f), attenuation1(0.0f), attenuation2(0.0f),
              theta(0.0f), phi(0.0f) {}
};

struct LightProg
{
    float   starting;
    float   ending;
    float   current;
    float   progress;
    float   speed;
};

/**
 * \struct SceneLight
 * \brief Dynamic light in 3D scene
 *
 * TODO documentation
 */
struct SceneLight
{
    //! true -> light exists
    bool            used;
    //! true -> light turned on
    bool            enable;

    //! Type of all objects included
    //D3DTypeObj        incluType;
    //! Type of all objects excluded
    //D3DTypeObj        excluType;

    //! Configuration of the light
    Gfx::Light      light;

    //! intensity (0 .. 1)
    Gfx::LightProg      intensity;
    Gfx::LightProg      colorRed;
    Gfx::LightProg      colorGreen;
    Gfx::LightProg      colorBlue;
};

// TODO CLight

}; // namespace Gfx
