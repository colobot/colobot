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

// material.h

#pragma once


#include "graphics/core/color.h"


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
    Gfx::Color diffuse;
    //! Ambient color
    Gfx::Color ambient;
    //! Specular color
    Gfx::Color specular;
};

}; // namespace Gfx
