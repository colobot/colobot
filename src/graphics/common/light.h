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


#include "graphics/common/engine.h"
#include "graphics/common/color.h"
#include "math/vector.h"


namespace Gfx {

/** \enum LightType
 *  \brief Type of light */
enum LightType
{
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_DIRECTIONAL
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
    Gfx::LightType  type;
    //! Color of ambient light
    Gfx::Color      ambient;
    //! Color of diffuse light
    Gfx::Color      diffuse;
    //! Color of specular light
    Gfx::Color      specular;
    //! Position in world space
    Math::Vector    position;
    //! Direction in world space
    Math::Vector    direction;
    //! Cutoff range
    float           range;
    //! Falloff
    float           falloff;
    //! Inner angle of spotlight cone
    float           theta;
    //! Outer angle of spotlight cone
    float           phi;
    //! Constant attenuation
    float           attenuation0;
    //! Linear attenuation
    float           attenuation1;
    //! Quadratic attenuation
    float           attenuation2;

    Light()
    {
        type = LIGHT_POINT;
        range = falloff = theta = phi = attenuation0 = attenuation1 = attenuation2 = 0.0f;
    }
};

/**
 * \struct LightProgression
 * \brief Describes the progression of light parameters change
 *
 * TODO documentation
 */
struct LightProgression
{
    float   starting;
    float   ending;
    float   current;
    float   progress;
    float   speed;

    LightProgression()
    {
        starting = ending = current = progress = speed = 0.0f;
    }
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
    bool used;
    //! true -> light turned on
    bool enabled;

    //! Type of all objects included
    Gfx::EngineObjectType includeType;
    //! Type of all objects excluded
    Gfx::EngineObjectType excludeType;

    //! Configuration of the light
    Gfx::Light light;

    //! intensity (0 .. 1)
    Gfx::LightProgression intensity;
    Gfx::LightProgression colorRed;
    Gfx::LightProgression colorGreen;
    Gfx::LightProgression colorBlue;
};

// TODO CLight

}; // namespace Gfx
