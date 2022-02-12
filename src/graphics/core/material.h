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
 * \file graphics/core/material.h
 * \brief Material struct
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/transparency.h"


// Graphics module namespace
namespace Gfx
{

/**
 * \enum CullFace
 * \brief Specifies which faces to cull while rendering polygons
 */
enum class CullFace : unsigned char
{
    NONE,
    BACK,
    FRONT,
    BOTH,
};

//! Remains of the legacy material structure, to be reused
struct Material
{
    // Albedo
    Color albedoColor = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::string albedoTexture = "";
    // Dirt
    bool variableDetail = false;
    std::string detailTexture = "";
    // Alpha mode
    AlphaMode alphaMode = AlphaMode::OPAQUE;
    float alphaThreshold = 0.0;
    // Cull face
    CullFace cullFace = CullFace::BACK;
    // Special tag
    std::string tag = "";

    bool operator==(const Material& other) const
    {
        return albedoColor == other.albedoColor
            && albedoTexture == other.albedoTexture
            && variableDetail == other.variableDetail
            && detailTexture == other.detailTexture
            && alphaMode == other.alphaMode
            && alphaThreshold == other.alphaThreshold
            && cullFace == other.cullFace
            && tag == other.tag;
    }

    bool operator!=(const Material& other) const
    {
        return !operator==(other);
    }
};

} // namespace Gfx
