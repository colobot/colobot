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

/**
 * \file graphics/core/material.h
 * \brief Material struct
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/transparency.h"

#include <filesystem>

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

/**
 * \enum Mark
 * \brief Special material mark for identifying model parts
 */
enum class Mark : unsigned char
{
    NONE,
    ENERGY,
    LEFT_TRACKER,
    RIGHT_TRACKER,
};

//! PBR material
struct Material
{
    //! Albedo color
    Color albedoColor = Color{ 1.0f, 1.0f, 1.0f, 1.0f };
    //! Albedo texture
    std::filesystem::path albedoTexture = "";
    //! Roughness
    float roughness = 1.0;
    //! Metalness
    float metalness = 0.0;
    //! AO strength
    float aoStrength = 0.0;
    //! AO-roughness-metalness texture
    std::filesystem::path materialTexture = "";
    //! Emissive color
    Color emissiveColor = Color{ 0.0f, 0.0f, 0.0f, 0.0f };
    //! Emissive texture
    std::filesystem::path emissiveTexture = "";
    //! Normal map
    std::string normalTexture = "";
    //! Alpha mode
    AlphaMode alphaMode = AlphaMode::NONE;
    //! Alpha threshold
    float alphaThreshold = 0.5;
    // Cull face
    CullFace cullFace = CullFace::BACK;
    // Special tag
    std::string tag = "";
    // Special mark
    Mark mark = Mark::NONE;

    // Legacy functionality
    //! Variable detail texture
    bool variableDetail = false;
    //! Detail texture
    std::filesystem::path detailTexture = "";

    bool operator==(const Material& other) const = default;
};

} // namespace Gfx
