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

// math3d.h

#ifndef _MATH3D_H_
#define _MATH3D_H_


#define STRICT
#define D3D_OVERLOADS
#include <math.h>


#define PI              3.14159265358979323846f
#define CHOUIA          1e-6f
#define BEAUCOUP        1e6f



BOOL            IsEqual(float a, float b);

float       Min(float a, float b);
float       Min(float a, float b, float c);
float       Min(float a, float b, float c, float d);
float       Min(float a, float b, float c, float d, float e);

float       Max(float a, float b);
float       Max(float a, float b, float c);
float       Max(float a, float b, float c, float d);
float       Max(float a, float b, float c, float d, float e);

float       Norm(float a);
float       Abs(float a);

void            Swap(int &a, int &b);
void            Swap(float &a, float &b);
void            Swap(FPOINT &a, FPOINT &b);

float       Mod(float a, float m);
float       NormAngle(float angle);
BOOL            TestAngle(float angle, float min, float max);

float       Direction(float a, float g);
FPOINT      RotatePoint(FPOINT center, float angle, FPOINT p);
FPOINT      RotatePoint(float angle, FPOINT p);
FPOINT      RotatePoint(float angle, float dist);
float       RotateAngle(float x, float y);
float       RotateAngle(FPOINT center, FPOINT p1, FPOINT p2);
float       MidPoint(FPOINT a, FPOINT b, float px);
D3DVECTOR   SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist);
BOOL            IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c, FPOINT p);
BOOL            Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i);
BOOL            IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p);
void            RotatePoint(float cx, float cy, float angle, float &px, float &py);
void            RotatePoint(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);
void            RotatePoint2(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);
D3DVECTOR   RotateView(D3DVECTOR center, float angleH, float angleV, float dist);
D3DVECTOR   LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );
float       Length(FPOINT a, FPOINT b);
float       Length(float x, float y);
float       Length(const D3DVECTOR &u);
float       Length(const D3DVECTOR &a, const D3DVECTOR &b);
float       Length2d(const D3DVECTOR &a, const D3DVECTOR &b);
float       Angle( D3DVECTOR u, D3DVECTOR v );
D3DVECTOR   Cross( D3DVECTOR u, D3DVECTOR v );
D3DVECTOR   ComputeNormal( D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3 );
D3DVECTOR   Transform(const D3DMATRIX &m, D3DVECTOR p);
D3DVECTOR   Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p);

void            MappingObject( D3DVERTEX2* pVertices, int nb, float scale );
void            SmoothObject( D3DVERTEX2* pVertices, int nb );
BOOL            LineFunction(FPOINT p1, FPOINT p2, float &a, float &b);
float       DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &c, const D3DVECTOR &p);
BOOL            IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2);
void            MatRotateXZY(D3DMATRIX &mat, D3DVECTOR angle);
void            MatRotateZXY(D3DMATRIX &mat, D3DVECTOR angle);

float       Rand();
float       Neutral(float value, float dead);

float       Prop(int a, int b, float p);
float       Smooth(float actual, float hope, float time);
float       Bounce(float progress, float middle=0.3f, float bounce=0.4f);

D3DCOLOR        RetColor(float intensity);
D3DCOLOR        RetColor(D3DCOLORVALUE intensity);
D3DCOLORVALUE RetColor(D3DCOLOR intensity);

void            RGB2HSV(D3DCOLORVALUE src, ColorHSV &dest);
void            HSV2RGB(ColorHSV src, D3DCOLORVALUE &dest);

#endif //_MATH3D_H_
