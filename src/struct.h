// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// struct.h

#ifndef _STRUCT_H_
#define _STRUCT_H_

#include <d3d.h>


#define NAN     999999

#define D3DFVF_VERTEX2  (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2)

struct D3DVERTEX2
{
    float       x,y,z;
    float       nx,ny,nz;
    float       tu, tv;
    float       tu2, tv2;

    D3DVERTEX2() { }
    D3DVERTEX2(const D3DVECTOR& _v, const D3DVECTOR& _n, float _tu=0.0f, float _tv=0.0f, float _tu2=0.0f, float _tv2=0.0f)
    {
        x   = _v.x;
        y   = _v.y;
        z   = _v.z;
        nx  = _n.x;
        ny  = _n.y;
        nz  = _n.z;
        tu  = _tu;
        tv  = _tv;
        tu2 = _tu2;
        tv2 = _tv2;
    }
};


struct FPOINT
{
    float   x;
    float   y;

    FPOINT() { }
    FPOINT(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
};


struct ColorHSV
{
    float   h,s,v;
};


#endif //_STRUCT_H_
