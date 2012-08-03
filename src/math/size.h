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
// * along with this program. If not, see    http://www.gnu.org/licenses/.

/** @defgroup MathSizeModule math/size.h
     Contains the Size struct.
 */

#pragma once

#include "math/point.h"


// Math module namespace
namespace Math
{

/* @{ */ // start of group

/** \struct Size math/size.h
    \brief 2D size

    Represents a 2D size (w, h).
    Is separate from Math::Point to avoid confusion.

 */
struct Size
{
    //! Width
    float w;
    //! Height
    float h;

    //! Constructs a zero size: (0,0)
    inline Size()
    {
        LoadZero();
    }

    //! Constructs a size from given dimensions: (w,h)
    inline explicit Size(float w, float h)
    {
        this->w = w;
        this->h = h;
    }

    //! Sets the zero size: (0,0)
    inline void LoadZero()
    {
        w = h = 0.0f;
    }

    //! Converts Point to Size
    inline static Math::Size FromPoint(Math::Point p)
    {
        return Math::Size(p.x, p.y);
    }
}; // struct Size


/* @} */ // end of group

}; // namespace Math
