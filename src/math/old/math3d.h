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

#pragma once


#define STRICT
#define D3D_OVERLOADS
#include <math.h>


#define PI				3.14159265358979323846f
#define CHOUIA			1e-6f
#define BEAUCOUP		1e6f



//>>> func.h IsEqual()
bool			IsEqual(float a, float b);

//>>> func.h Min()
float		Min(float a, float b);
float		Min(float a, float b, float c);
float		Min(float a, float b, float c, float d);
float		Min(float a, float b, float c, float d, float e);

//>>> func.h Max()
float		Max(float a, float b);
float		Max(float a, float b, float c);
float		Max(float a, float b, float c, float d);
float		Max(float a, float b, float c, float d, float e);

//>>> func.h Norm()
float		Norm(float a);
//>>> fabs()
float		Abs(float a);

//>>> func.h Swap()
void			Swap(int &a, int &b);
//>>> func.h Swap()
void			Swap(float &a, float &b);
//>>> point.h Swap() (FPOINT -> Point)
void			Swap(FPOINT &a, FPOINT &b);

//>>> func.h Mod()
float		Mod(float a, float m);
//>>> func.h NormAngle()
float		NormAngle(float angle);
//>>> func.h TestAngle()
bool			TestAngle(float angle, float min, float max);

//>>> func.h Direction()
float		Direction(float a, float g);
//>>> geometry.h RotatePoint()
FPOINT		RotatePoint(FPOINT center, float angle, FPOINT p);
//>>> geometry.h RotatePoint()
FPOINT		RotatePoint(float angle, FPOINT p);
//>>> geometry.h RotatePoint()
FPOINT		RotatePoint(float angle, float dist);
//>>> geometry.h RotateAngle()
float		RotateAngle(float x, float y);
//>>> geometry.h RotateAngle()
float		RotateAngle(FPOINT center, FPOINT p1, FPOINT p2);
//>>> geometry.h MidPoint()
float		MidPoint(FPOINT a, FPOINT b, float px);
//>>> geometry.h SegmentPoint()
D3DVECTOR	SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist);
//>>> geometry.h IsInsideTriangle()
bool			IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c, FPOINT p);
//>>> geometry.h Intersect()
bool			Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i);
//>>> geometry.h IntersectY()
bool			IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p);
//>>> geometry.h RotatePoint()
void			RotatePoint(float cx, float cy, float angle, float &px, float &py);
//>>> geometry.h RotatePoint()
void			RotatePoint(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);
//>>> geometry.h RotatePoint2()
void			RotatePoint2(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p);
//>>> geometry.h RotateView()
// TODO test & verify
D3DVECTOR	RotateView(D3DVECTOR center, float angleH, float angleV, float dist);
//>>> geometry.h LookatPoint()
// TODO test & verify
D3DVECTOR	LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );
//>>> point.h Distance()
float		Length(FPOINT a, FPOINT b);
//>>> point.h Point::Length()
float		Length(float x, float y);
//>>> vector.h Vector::Length()
float		Length(const D3DVECTOR &u);
//>>> vector.h Distance()
float		Length(const D3DVECTOR &a, const D3DVECTOR &b);
//>>> geometry.h DistanceProjected()
float		Length2d(const D3DVECTOR &a, const D3DVECTOR &b);
//>>> vector.h Angle()
// TODO test & verify
float		Angle( D3DVECTOR u, D3DVECTOR v );
//>>> vector.h CrossProduct()
D3DVECTOR	Cross( D3DVECTOR u, D3DVECTOR v );
//>>> geometry.h NormalToPlane()
D3DVECTOR	ComputeNormal( D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3 );
//>>> geometry.h Transform()
// TODO test & verify
D3DVECTOR	Transform(const D3DMATRIX &m, D3DVECTOR p);
//>>> geometry.h Projection()
// TODO test & verify
D3DVECTOR	Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p);

// TODO
void			MappingObject( D3DVERTEX2* pVertices, int nb, float scale );
// TODO
void			SmoothObject( D3DVERTEX2* pVertices, int nb );
//>>> geometry.h LinearFunction()
bool			LineFunction(FPOINT p1, FPOINT p2, float &a, float &b);
//>>> geometry.h DistanceToPlane()
float		DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &c, const D3DVECTOR &p);
//>>> geometry.h IsSamePlane()
bool			IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2);
//>>> geometry.h LoadRotationXZYMatrix()
// TODO test & verify
void			MatRotateXZY(D3DMATRIX &mat, D3DVECTOR angle);
//>>> geometry.h LoadRotationZXYMatrix()
// TODO test & verify
void			MatRotateZXY(D3DMATRIX &mat, D3DVECTOR angle);

//>>> func.h Rand()
float		Rand();
//>>> func.h Neutral()
float		Neutral(float value, float dead);

//>>> func.h PropAngle()
float		Prop(int a, int b, float p);
//>>> func.h Smooth()
float		Smooth(float actual, float hope, float time);
//>>> func.h Bounce()
float		Bounce(float progress, float middle=0.3f, float bounce=0.4f);

// TODO
D3DCOLOR		RetColor(float intensity);
// TODO
D3DCOLOR		RetColor(D3DCOLORVALUE intensity);
// TODO
D3DCOLORVALUE RetColor(D3DCOLOR intensity);

// TODO
void			RGB2HSV(D3DCOLORVALUE src, ColorHSV &dest);
// TODO
void			HSV2RGB(ColorHSV src, D3DCOLORVALUE &dest);


