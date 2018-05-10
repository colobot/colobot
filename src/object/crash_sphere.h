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

#include "common/logger.h"

#include "math/sphere.h"
#include "math/vector.h"

#include "sound/sound_type.h"

/**
 * \struct CrashSphere
 * \brief Sphere used to detect object collisions
 */
struct CrashSphere
{
    CrashSphere(const Math::Vector& pos = Math::Vector(),
                float radius = 0.0f,
                SoundType _sound = SOUND_NONE,
                float _hardness = 0.45f)
     : sphere(pos, radius)
     , sound(_sound)
     , hardness(_hardness)
    {
        if (sound == SOUND_CLICK)
        {
            GetLogger()->Warn("Crash sphere using SOUND_CLICK, using SOUND_NONE instead.\n"); // TODO: Make sure v3 model files don't use this ~krzys_h
            sound = SOUND_NONE;
        }
    }

    //! Sphere (position + radius)
    //! Sphere position is given in either object or world coordinates
    //! (see CObject functions for more info)
    Math::Sphere sphere;
    //! Sound to be played on collision
    SoundType sound;
    //! "Hardness" of the sphere
    float hardness;
};
