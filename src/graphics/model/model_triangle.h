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

#include "graphics/core/color.h"
#include "graphics/core/vertex.h"

namespace Gfx
{

/**
 * \enum ModelSpecialMark
 * \brief Special marking for some models
 *
 * TODO: refactor/remove in the future
 */
enum class ModelSpecialMark
{
    None,
    Part1,
    Part2,
    Part3
};

/**
 * \enum ModelTransparentMode
 * \brief Describes how to deal with texture transparency
 *
 * TODO: get rid of it in the future (use only alpha channel)
 */
enum class ModelTransparentMode
{
    None,            //!< no transparency
    AlphaChannel,    //!< use alpha channel
    MapBlackToAlpha, //!< map black color to alpha
    MapWhiteToAlpha  //!< map white color to alpha
};

/**
 * \struct ModelTriangle
 * \brief A single triangle in mesh as saved in model file
 */
struct ModelTriangle
{
    //! 1st vertex
    VertexTex2  p1;
    //! 2nd vertex
    VertexTex2  p2;
    //! 3rd vertex
    VertexTex2  p3;
    //! Diffuse color
    Color diffuse;
    //! Ambient color
    Color ambient;
    //! Specular color
    Color specular;
    //! Name of 1st texture
    std::string tex1Name;
    //! Name of 2nd texture
    std::string tex2Name;
    //! If true, 2nd texture will be taken from current engine setting
    bool variableTex2 = false;
    //! Whether to render as double-sided surface
    bool doubleSided = false;
    //! How to deal with texture transparency
    ModelTransparentMode transparentMode = ModelTransparentMode::None;
    //! Special marking
    ModelSpecialMark specialMark = ModelSpecialMark::None;
};

} // namespace Gfx
