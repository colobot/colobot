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

/** @defgroup MathIntSizeModule math/intsize.h
     Contains the IntSize struct.
 */

#pragma once

// Math module namespace
namespace Math
{

/* @{ */ // start of group

/** \struct IntSize math/size.h
    \brief 2D size with integer dimensions */
struct IntSize
{
    //! Width
    int w;
    //! Height
    int h;

    //! Constructs a zero size: (0,0)
    inline IntSize()
    {
        LoadZero();
    }

    //! Constructs a size from given dimensions: (w,h)
    inline explicit IntSize(int w, int h)
    {
        this->w = w;
        this->h = h;
    }

    //! Sets the zero size: (0,0)
    inline void LoadZero()
    {
        w = h = 0;
    }
}; // struct Size


/* @} */ // end of group

}; // namespace Math
