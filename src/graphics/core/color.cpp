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


#include "graphics/core/color.h"

#include "math/func.h"


// Graphics module namespace
namespace Gfx {


ColorHSV RGB2HSV(Color color)
{
    ColorHSV result;

    float min = Math::Min(color.r, color.g, color.b);
    float max = Math::Max(color.r, color.g, color.b);

    result.v = max;  // intensity

    if ( max == 0.0f )
    {
        result.s = 0.0f;  // saturation
        result.h = 0.0f;  // undefined color!
    }
    else
    {
        float delta = max-min;
        result.s = delta/max;  // saturation

        if ( color.r == max )  // between yellow & magenta
        {
        result.h = (color.g-color.b)/delta;
        }
        else if ( color.g == max )  // between cyan & yellow
        {
        result.h = 2.0f+(color.b-color.r)/delta;
        }
        else  // between magenta & cyan
        {
        result.h = 4.0f+(color.r-color.g)/delta;
        }

        result.h *= 60.0f;  // in degrees
        if ( result.h < 0.0f )  result.h += 360.0f;
        result.h /= 360.0f;  // 0..1
    }

    return result;
}

Color HSV2RGB(ColorHSV color)
{
    Color result;

    color.h = Math::Norm(color.h)*360.0f;
    color.s = Math::Norm(color.s);
    color.v = Math::Norm(color.v);

    if ( color.s == 0.0f )  // zero saturation?
    {
        result.r = color.v;
        result.g = color.v;
        result.b = color.v;  // gray
    }
    else
    {
        if ( color.h == 360.0f )  color.h = 0.0f;
        color.h /= 60.0f;
        int i = static_cast<int>(color.h);  // integer part (0 .. 5)
        float f = color.h-i;   // fractional part

        float v = color.v;
        float p = color.v*(1.0f-color.s);
        float q = color.v*(1.0f-(color.s*f));
        float t = color.v*(1.0f-(color.s*(1.0f-f)));

        switch (i)
        {
        case 0:  result.r=v; result.g=t; result.b=p;  break;
        case 1:  result.r=q; result.g=v; result.b=p;  break;
        case 2:  result.r=p; result.g=v; result.b=t;  break;
        case 3:  result.r=p; result.g=q; result.b=v;  break;
        case 4:  result.r=t; result.g=p; result.b=v;  break;
        case 5:  result.r=v; result.g=p; result.b=q;  break;
        }
    }

    return result;
}


} // namespace Gfx

