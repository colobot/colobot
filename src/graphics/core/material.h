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
 * \file graphics/core/material.h
 * \brief Material struct
 */

#pragma once


#include "graphics/core/color.h"


// Graphics module namespace
namespace Gfx {

/**
 * \struct Material
 * \brief Material of a surface
 *
 * This structure was created as analog to DirectX's D3DMATERIAL.
 *
 * It contains values of 3 material colors: diffuse, ambient and specular.
 * In D3DMATERIAL there are other fields, but they are not used
 * by the graphics engine.
 */
struct Material
{
    //! Diffuse color
    Color diffuse;
    //! Ambient color
    Color ambient;
    //! Specular color
    Color specular;

    bool operator==(const Material &mat) const
    {
        return diffuse == mat.diffuse && ambient == mat.ambient && specular == mat.specular;
    }

    bool operator!=(const Material &mat) const
    {
        return ! operator==(mat);
    }
};


} // namespace Gfx

