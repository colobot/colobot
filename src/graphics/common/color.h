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


namespace Gfx {

struct Color
{
    float r, g, b, a;

    Color(float aR = 0.0f, float aG = 0.0f, float aB = 0.0f, float aA = 0.0f)
     : r(aR), g(aG), b(aB), a(aA) {}
};


struct ColorHSV
{
    float h, s, v;

    ColorHSV(float aH = 0.0f, float aS = 0.0f, float aV = 0.0f)
     : h(aH), s(aS), v(aV) {}
};


long  RetColor(float intensity);
long  RetColor(Color intensity);
Color RetColor(long intensity);

void RGB2HSV(Color src, ColorHSV &dest);
void HSV2RGB(ColorHSV src, Color &dest);

}; // namespace Gfx

