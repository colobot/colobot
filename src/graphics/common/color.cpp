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

// color.cpp

#include "graphics/common/color.h"


// Returns the color corresponding long.

long RetColor(float intensity)
{
  long  color;
  
  if ( intensity <= 0.0f )  return 0x00000000;
  if ( intensity >= 1.0f )  return 0xffffffff;
  
  color  = (int)(intensity*255.0f)<<24;
  color |= (int)(intensity*255.0f)<<16;
  color |= (int)(intensity*255.0f)<<8;
  color |= (int)(intensity*255.0f);
  
  return color;
}

// Returns the color corresponding long.

long RetColor(Color intensity)
{
  long  color;
  
  color  = (int)(intensity.a*255.0f)<<24;
  color |= (int)(intensity.r*255.0f)<<16;
  color |= (int)(intensity.g*255.0f)<<8;
  color |= (int)(intensity.b*255.0f);
  
  return color;
}

// Returns the color corresponding Color.

Color RetColor(long intensity)
{
  Color color;
  
  color.r = (float)((intensity>>16)&0xff)/256.0f;
  color.g = (float)((intensity>>8 )&0xff)/256.0f;
  color.b = (float)((intensity>>0 )&0xff)/256.0f;
  color.a = (float)((intensity>>24)&0xff)/256.0f;
  
  return color;
}


// RGB to HSV conversion.

void RGB2HSV(Color src, ColorHSV &dest)
{
  float min, max, delta;
  
  min = Math::Min(src.r, src.g, src.b);
  max = Math::Max(src.r, src.g, src.b);
  
  dest.v = max;  // intensity
  
  if ( max == 0.0f )
  {
    dest.s = 0.0f;  // saturation
    dest.h = 0.0f;  // undefined color!
  }
  else
  {
    delta = max-min;
    dest.s = delta/max;  // saturation
    
    if ( src.r == max )  // between yellow & magenta
    {
      dest.h = (src.g-src.b)/delta;
    }
    else if ( src.g == max )  // between cyan & yellow
    {
      dest.h = 2.0f+(src.b-src.r)/delta;
    }
    else  // between magenta & cyan
    {
      dest.h = 4.0f+(src.r-src.g)/delta;
    }
    
    dest.h *= 60.0f;  // in degrees
    if ( dest.h < 0.0f )  dest.h += 360.0f;
    dest.h /= 360.0f;  // 0..1
  }
}

// HSV to RGB conversion.

void HSV2RGB(ColorHSV src, Color &dest)
{
  int   i;
  float f,v,p,q,t;
  
  src.h = Math::Norm(src.h)*360.0f;
  src.s = Math::Norm(src.s);
  src.v = Math::Norm(src.v);
  
  if ( src.s == 0.0f )  // zero saturation?
  {
    dest.r = src.v;
    dest.g = src.v;
    dest.b = src.v;  // gray
  }
  else
  {
    if ( src.h == 360.0f )  src.h = 0.0f;
    src.h /= 60.0f;
    i = (int)src.h;  // integer part (0 .. 5)
    f = src.h-i;     // fractional part
    
    v = src.v;
    p = src.v*(1.0f-src.s);
    q = src.v*(1.0f-(src.s*f));
    t = src.v*(1.0f-(src.s*(1.0f-f)));
    
    switch (i)
    {
      case 0:  dest.r=v; dest.g=t; dest.b=p;  break;
      case 1:  dest.r=q; dest.g=v; dest.b=p;  break;
      case 2:  dest.r=p; dest.g=v; dest.b=t;  break;
      case 3:  dest.r=p; dest.g=q; dest.b=v;  break;
      case 4:  dest.r=t; dest.g=p; dest.b=v;  break;
      case 5:  dest.r=v; dest.g=p; dest.b=q;  break;
    }
  }
}

