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

// color.h

#pragma once


// TODO
namespace Gfx {

struct Color
{
    
    float r, g, b, a;
    
};


struct ColorHSV
{
    float	h,s,v;
};


long  RetColor(float intensity);
long  RetColor(Color intensity);
Color RetColor(long intensity);

void      RGB2HSV(Color src, ColorHSV &dest);
void      HSV2RGB(ColorHSV src, Color &dest);

};

