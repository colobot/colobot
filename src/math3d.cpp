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

// math3d.cpp

#define STRICT
#define D3D_OVERLOADS

#include <math.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "d3dutil.h"
#include "math3d.h"



// Returns TRUE if two numbers are nearly equal.

BOOL IsEqual(float a, float b)
{
    return Abs(a-b) < CHOUIA;
}


// Returns the minimum value.

float Min(float a, float b)
{
    if ( a <= b )  return a;
    else           return b;
}

float Min(float a, float b, float c)
{
    return Min( Min(a,b), c );
}

float Min(float a, float b, float c, float d)
{
    return Min( Min(a,b), Min(c,d) );
}

float Min(float a, float b, float c, float d, float e)
{
    return Min( Min(a,b), Min(c,d), e );
}


// Returns the maximum value.

float Max(float a, float b)
{
    if ( a >= b )  return a;
    else           return b;
}

float Max(float a, float b, float c)
{
    return Max( Max(a,b), c );
}

float Max(float a, float b, float c, float d)
{
    return Max( Max(a,b), Max(c,d) );
}

float Max(float a, float b, float c, float d, float e)
{
    return Max( Max(a,b), Max(c,d), e );
}


// Returns the normalized value (0 .. 1).

float Norm(float a)
{
    if ( a < 0.0f )  return 0.0f;
    if ( a > 1.0f )  return 1.0f;
    return a;
}


// Returns the absolute value of a number.

float Abs(float a)
{
    return (float)fabs(a);
}


// Swaps two integers.

void Swap(int &a, int &b)
{
    int     c;

    c = a;
    a = b;
    b = c;
}

// Swaps two real numbers.

void Swap(float &a, float &b)
{
    float   c;

    c = a;
    a = b;
    b = c;
}

// Permutes two points.

void Swap(FPOINT &a, FPOINT &b)
{
    FPOINT  c;

    c = a;
    a = b;
    b = c;
}

// Returns the modulo of a floating point number.
//  Mod(8.1, 4) = 0.1
//  Mod(n, 1) = fractional part of n

float Mod(float a, float m)
{
    return a - ((int)(a/m))*m;
}

// Returns a normalized angle, that is in other words between 0 and 2 * PI.

float NormAngle(float angle)
{
    angle = Mod(angle, PI*2.0f);
    if ( angle < 0.0f )
    {
        return PI*2.0f + angle;
    }
    else
    {
        return angle;
    }
}

// Test if a angle is between two terminals.

BOOL TestAngle(float angle, float min, float max)
{
    angle = NormAngle(angle);
    min   = NormAngle(min);
    max   = NormAngle(max);

    if ( min > max )
    {
        return ( angle <= max || angle >= min );
    }
    else
    {
        return ( angle >= min && angle <= max );
    }
}


// Calculates the angle to rotate the angle a to the angle g.
// A positive angle is counterclockwise (CCW).

float Direction(float a, float g)
{
    a = NormAngle(a);
    g = NormAngle(g);

    if ( a < g )
    {
        if ( a+PI*2.0f-g < g-a )  a += PI*2.0f;
    }
    else
    {
        if ( g+PI*2.0f-a < a-g )  g += PI*2.0f;
    }
    return (g-a);
}


// Rotates a point around a center.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

FPOINT RotatePoint(FPOINT center, float angle, FPOINT p)
{
    FPOINT  a, b;

    a.x = p.x-center.x;
    a.y = p.y-center.y;

    b.x = a.x*cosf(angle) - a.y*sinf(angle);
    b.y = a.x*sinf(angle) + a.y*cosf(angle);

    b.x += center.x;
    b.y += center.y;
    return b;
}

// Rotates a point around the origin.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

FPOINT RotatePoint(float angle, FPOINT p)
{
    FPOINT  a;

    a.x = p.x*cosf(angle) - p.y*sinf(angle);
    a.y = p.x*sinf(angle) + p.y*cosf(angle);

    return a;
}

// Rotates a vector (dist, 0).
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

FPOINT RotatePoint(float angle, float dist)
{
    FPOINT  a;

    a.x = dist*cosf(angle);
    a.y = dist*sinf(angle);

    return a;
}

// Calculates the angle of a right triangle.
// The angle is counterclockwise (CCW), between 0 and 2 * PI.
// For an angle clockwise (CW), just go ahead.
//
//      ^
//      |
//    y o----o
//      |  / |
//      |/)a |
//  ----o----o-->
//      |    x
//      |

float RotateAngle(float x, float y)
{
#if 1
    if ( x == 0.0f && y == 0.0f )  return 0.0f;

    if ( x >= 0.0f )
    {
        if ( y >= 0.0f )
        {
            if ( x > y )  return           atanf(y/x);
            else          return PI*0.5f - atanf(x/y);
        }
        else
        {
            if ( x > -y )  return PI*2.0f + atanf(y/x);
            else           return PI*1.5f - atanf(x/y);
        }
    }
    else
    {
        if ( y >= 0.0f )
        {
            if ( -x > y )  return PI*1.0f + atanf(y/x);
            else           return PI*0.5f - atanf(x/y);
        }
        else
        {
            if ( -x > -y )  return PI*1.0f + atanf(y/x);
            else            return PI*1.5f - atanf(x/y);
        }
    }
#else
    float   angle;

    if ( x == 0.0f )
    {
        if ( y > 0.0f )
        {
            return 90.0f*PI/180.0f;
        }
        else
        {
            return 270.0f*PI/180.0f;
        }
    }
    else
    {
        angle = atanf(y/x);
        if ( x < 0.0f )
        {
            angle += PI;
        }
        return angle;
    }
#endif
}

// Calculates the angle between two points and one center.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

float RotateAngle(FPOINT center, FPOINT p1, FPOINT p2)
{
    float   a1, a2, a;

    if ( p1.x == center.x &&
         p1.y == center.y )  return 0;

    if ( p2.x == center.x &&
         p2.y == center.y )  return 0;

    a1 = asinf((p1.y-center.y)/Length(p1,center));
    a2 = asinf((p2.y-center.y)/Length(p2,center));

    if ( p1.x < center.x )  a1 = PI-a1;
    if ( p2.x < center.x )  a2 = PI-a2;

    a = a2-a1;
    if ( a < 0 )  a += PI*2;
    return a;
}

// Returns py up on the line ab.

float MidPoint(FPOINT a, FPOINT b, float px)
{
    if ( Abs(a.x-b.x) < CHOUIA )
    {
        if ( a.y < b.y )  return  BEAUCOUP;
        else              return -BEAUCOUP;
    }
    return (b.y-a.y)*(px-a.x)/(b.x-a.x)+a.y;
}

// Advance "dist" along the segment p1-p2.

D3DVECTOR SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist)
{
    return p1+Normalize(p2-p1)*dist;
}

// Check if a point is inside a triangle.

BOOL IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c, FPOINT p)
{
    float   n, m;

    if ( p.x < a.x && p.x < b.x && p.x < c.x )  return FALSE;
    if ( p.x > a.x && p.x > b.x && p.x > c.x )  return FALSE;
    if ( p.y < a.y && p.y < b.y && p.y < c.y )  return FALSE;
    if ( p.y > a.y && p.y > b.y && p.y > c.y )  return FALSE;

    if ( a.x > b.x )  Swap(a,b);
    if ( a.x > c.x )  Swap(a,c);
    if ( c.x < a.x )  Swap(c,a);
    if ( c.x < b.x )  Swap(c,b);

    n = MidPoint(a, b, p.x);
    m = MidPoint(a, c, p.x);
    if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return FALSE;

    n = MidPoint(c, b, p.x);
    m = MidPoint(c, a, p.x);
    if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return FALSE;

    return TRUE;
}

// Calculates the intersection "i" right "of" the plan "abc".

BOOL Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c,
               D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i)
{
    float   d1, d2;

    d1 = (d.x-a.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
         (d.y-a.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
         (d.z-a.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

    d2 = (d.x-e.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
         (d.y-e.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
         (d.z-e.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

    if ( d2 == 0 )  return FALSE;

    i.x = d.x + d1/d2*(e.x-d.x);
    i.y = d.y + d1/d2*(e.y-d.y);
    i.z = d.z + d1/d2*(e.z-d.z);
    return TRUE;
}

// Calculates the intersection of the straight line passing through p (x, z)
// parallel to the y axis, with the plane abc. Returns p.y.

BOOL IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p)
{
#if 0
    D3DVECTOR   d,e,i;

    d.x = p.x;
    d.y = 0.0f;
    d.z = p.z;
    e.x = p.x;
    e.y = 1.0f;
    e.z = p.z;
    if ( !Intersect(a,b,c,d,e,i) )  return FALSE;
    p.y = i.y;
    return TRUE;
#else
    float   d, d1, d2;

    d  = (b.x-a.x)*(c.z-a.z) - (c.x-a.x)*(b.z-a.z);
    d1 = (p.x-a.x)*(c.z-a.z) - (c.x-a.x)*(p.z-a.z);
    d2 = (b.x-a.x)*(p.z-a.z) - (p.x-a.x)*(b.z-a.z);

    if ( d == 0.0f )  return FALSE;

    p.y = a.y + d1/d*(b.y-a.y) + d2/d*(c.y-a.y);
    return TRUE;
#endif
}


// Rotates a point around a center in the plan.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

void RotatePoint(float cx, float cy, float angle, float &px, float &py)
{
    float   ax, ay;

    px -= cx;
    py -= cy;

    ax = px*cosf(angle) - py*sinf(angle);
    ay = px*sinf(angle) + py*cosf(angle);

    px = cx+ax;
    py = cy+ay;
}

// Rotates a point around a center in space.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

void RotatePoint(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p)
{
    D3DVECTOR   a, b;

    p.x -= center.x;
    p.y -= center.y;
    p.z -= center.z;

    b.x = p.x*cosf(angleH) - p.z*sinf(angleH);
    b.y = p.z*sinf(angleV) + p.y*cosf(angleV);
    b.z = p.x*sinf(angleH) + p.z*cosf(angleH);

    p.x = center.x+b.x;
    p.y = center.y+b.y;
    p.z = center.z+b.z;
}

// Rotates a point around a center in space.
// The angle is in radians.
// A positive angle is counterclockwise (CCW).

void RotatePoint2(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p)
{
    D3DVECTOR   a, b;

    p.x -= center.x;
    p.y -= center.y;
    p.z -= center.z;

    a.x = p.x*cosf(angleH) - p.z*sinf(angleH);
    a.y = p.y;
    a.z = p.x*sinf(angleH) + p.z*cosf(angleH);

    b.x = a.x;
    b.y = a.z*sinf(angleV) + a.y*cosf(angleV);
    b.z = a.z*cosf(angleV) - a.y*sinf(angleV);

    p.x = center.x+b.x;
    p.y = center.y+b.y;
    p.z = center.z+b.z;
}

// Calculation point of view to look at a center
// two angles and a distance.

D3DVECTOR RotateView(D3DVECTOR center, float angleH, float angleV, float dist)
{
    D3DMATRIX   mat1, mat2, mat;
    D3DVECTOR   eye;

    D3DUtil_SetRotateZMatrix(mat1, -angleV);
    D3DUtil_SetRotateYMatrix(mat2, -angleH);
    D3DMath_MatrixMultiply(mat, mat1, mat2);

    eye.x = 0.0f+dist;
    eye.y = 0.0f;
    eye.z = 0.0f;
    eye = Transform(mat, eye);

    return eye+center;
}

// Calculates the end point.

D3DVECTOR LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length )
{
    D3DVECTOR   lookat;

    lookat = eye;
    lookat.z += length;

//? RotatePoint(eye.x, eye.z, angleH, lookat.x, lookat.z);
//? RotatePoint(eye.z, eye.y, angleV, lookat.z, lookat.y);
    RotatePoint(eye, angleH, angleV, lookat);

    return lookat;
}


// Returns the distance between two points.

float Length(FPOINT a, FPOINT b)
{
    return sqrtf( (a.x-b.x)*(a.x-b.x) +
                  (a.y-b.y)*(a.y-b.y) );
}

// Returns the hypotenuse of a right triangle.

float Length(float x, float y)
{
    return sqrtf( (x*x) + (y*y) );
}

// Returns the length of a vector.

float Length(const D3DVECTOR &u)
{
    return sqrtf( (u.x*u.x) + (u.y*u.y) + (u.z*u.z) );
}

// Returns the distance between two points.

float Length(const D3DVECTOR &a, const D3DVECTOR &b)
{
    return sqrtf( (a.x-b.x)*(a.x-b.x) +
                  (a.y-b.y)*(a.y-b.y) +
                  (a.z-b.z)*(a.z-b.z) );
}

// Returns the distance "a flat" between two points.

float Length2d(const D3DVECTOR &a, const D3DVECTOR &b)
{
    return sqrtf( (a.x-b.x)*(a.x-b.x) +
                  (a.z-b.z)*(a.z-b.z) );
}


// Returns the angle formed by two vectors.

float Angle( D3DVECTOR u, D3DVECTOR v )
{
#if 0
    return acosf( Abs(u.x*v.x + u.y*v.y + u.z*v.z) / (Length(u)*Length(v)) );
#endif
#if 0
    float   d;
    d = (u.y*v.z-u.z*v.y) + (u.z*v.x-u.x*v.z) + (u.x*v.y-u.y*v.x);
    return asinf( Abs(d) / (Length(u)*Length(v)) );
#endif
#if 0
    return asinf( Length(Cross(u,v)) / (Length(u)*Length(v)) );
#endif
#if 1
    float   len, a, b;

    len = Length(u)*Length(v);
    a = acosf( (u.x*v.x + u.y*v.y + u.z*v.z) / len );
    b = asinf( Length(Cross(u,v)) / len );
    return a;
#endif
}

// Returns the product of two vectors.

D3DVECTOR Cross( D3DVECTOR u, D3DVECTOR v )
{
    return D3DVECTOR( u.y*v.z - u.z*v.y,
                      u.z*v.x - u.x*v.z,
                      u.x*v.y - u.y*v.x );
}

// Returns the normal vector of a triangular face.

D3DVECTOR ComputeNormal( D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3 )
{
    D3DVECTOR   u, v;

    u = D3DVECTOR( p3.x-p1.x, p3.y-p1.y, p3.z-p1.z );
    v = D3DVECTOR( p2.x-p1.x, p2.y-p1.y, p2.z-p1.z );

    return Normalize(Cross(u, v));
}


// Transforms a point in a matrix, in exactly the same manner as Direct3D.

D3DVECTOR Transform(const D3DMATRIX &m, D3DVECTOR p)
{
    D3DVECTOR   pp;

    pp.x = p.x*m._11 + p.y*m._21 + p.z*m._31 + m._41;
    pp.y = p.x*m._12 + p.y*m._22 + p.z*m._32 + m._42;
    pp.z = p.x*m._13 + p.y*m._23 + p.z*m._33 + m._43;

    return pp;
}


// Calculates the projection of a point P on a straight line AB.

D3DVECTOR Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p)
{
    float       k;

    k  = (b.x-a.x)*(p.x-a.x) + (b.y-a.y)*(p.y-a.y) + (b.z-a.z)*(p.z-a.z);
    k /= (b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y) + (b.z-a.z)*(b.z-a.z);

    return a + k*(b-a);
}

// The texture plate in the xz plane.

void MappingObject(D3DVERTEX2* pVertices, int nb, float scale)
{
    int     i;

    for ( i=0 ; i<nb ; i++ )
    {
        pVertices[i].tu = pVertices[i].x*scale;
        pVertices[i].tv = pVertices[i].z*scale;
    }
}

// Smooths normal.

void SmoothObject(D3DVERTEX2* pVertices, int nb)
{
    char*       bDone;
    int         index[100];
    int         i, j, rank;
    D3DVECTOR   sum;

    bDone = (char*)malloc(nb*sizeof(char));
    ZeroMemory(bDone, nb*sizeof(char));

    for ( i=0 ; i<nb ; i++ )
    {
        bDone[i] = TRUE;
        rank = 0;
        index[rank++] = i;

        for ( j=0 ; j<nb ; j++ )
        {
            if ( bDone[j] )  continue;
            if ( pVertices[j].x == pVertices[i].x &&
                 pVertices[j].y == pVertices[i].y &&
                 pVertices[j].z == pVertices[i].z )
            {
                bDone[j] = TRUE;
                index[rank++] = j;
                if ( rank >= 100 )  break;
            }
        }

        sum.x = 0;
        sum.y = 0;
        sum.z = 0;
        for ( j=0 ; j<rank ; j++ )
        {
            sum.x += pVertices[index[j]].nx;
            sum.y += pVertices[index[j]].ny;
            sum.z += pVertices[index[j]].nz;
        }
        sum = Normalize(sum);

        for ( j=0 ; j<rank ; j++ )
        {
            pVertices[index[j]].nx = sum.x;
            pVertices[index[j]].ny = sum.y;
            pVertices[index[j]].nz = sum.z;
        }
    }

    free(bDone);
}



// Calculates the parameters a and b of the segment passing
// through the points p1 and p2, knowing that:
//      f(x) = ax+b
// Returns FALSE if the line is vertical.

BOOL LineFunction(FPOINT p1, FPOINT p2, float &a, float &b)
{
    if ( D3DMath_IsZero(p1.x-p2.x) )
    {
        a = g_HUGE;  // infinite slope!
        b = p2.x;
        return FALSE;
    }

    a = (p2.y-p1.y)/(p2.x-p1.x);
    b = p2.y - p2.x*a;
    return TRUE;
}


// Calculates the distance between a plane ABC and a point P.

float DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b,
                        const D3DVECTOR &c, const D3DVECTOR &p)
{
    D3DVECTOR   n;
    float       aa,bb,cc,dd;

    n = ComputeNormal(a,b,c);

    aa = n.x;
    bb = n.y;
    cc = n.z;
    dd = -(n.x*a.x + n.y*a.y + n.z*a.z);

    return Abs(aa*p.x + bb*p.y + cc*p.z + dd);
}

// Check if two planes defined by 3 points are part of the same plan.

BOOL IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2)
{
    D3DVECTOR   n1, n2;
    float       dist;

    n1 = ComputeNormal(plan1[0], plan1[1], plan1[2]);
    n2 = ComputeNormal(plan2[0], plan2[1], plan2[2]);

    if ( Abs(n1.x-n2.x) > 0.1f ||
         Abs(n1.y-n2.y) > 0.1f ||
         Abs(n1.z-n2.z) > 0.1f )  return FALSE;

    dist = DistancePlanPoint(plan1[0], plan1[1], plan1[2], plan2[0]);
    if ( dist > 0.1f )  return FALSE;

    return TRUE;
}


// Calculates the matrix to make three rotations in the X, Y and Z
// >>>>>> OPTIMIZING!!!

void MatRotateXZY(D3DMATRIX &mat, D3DVECTOR angle)
{
    D3DMATRIX   temp;

    D3DUtil_SetRotateXMatrix(temp, angle.x);
    D3DUtil_SetRotateZMatrix(mat, angle.z);
    D3DMath_MatrixMultiply(mat, mat, temp);
    D3DUtil_SetRotateYMatrix(temp, angle.y);
    D3DMath_MatrixMultiply(mat, mat, temp);  // X-Z-Y
}

// Calculates the matrix to make three rotations in the order Z, X and Y.
// >>>>>> OPTIMIZING!!!

void MatRotateZXY(D3DMATRIX &mat, D3DVECTOR angle)
{
    D3DMATRIX   temp;

    D3DUtil_SetRotateZMatrix(temp, angle.z);
    D3DUtil_SetRotateXMatrix(mat, angle.x);
    D3DMath_MatrixMultiply(mat, mat, temp);
    D3DUtil_SetRotateYMatrix(temp, angle.y);
    D3DMath_MatrixMultiply(mat, mat, temp);  // Z-X-Y
}


// Returns a random value between 0 and 1.

float Rand()
{
    return (float)rand()/RAND_MAX;
}


// Managing the dead zone of a joystick.

//  in:   -1            0            1
//       --|-------|----o----|-------|-->
//                      <---->
//                       dead
//  out:  -1       0         0       1

float Neutral(float value, float dead)
{
    if ( Abs(value) <= dead )
    {
        return 0.0f;
    }
    else
    {
        if ( value > 0.0f )  return (value-dead)/(1.0f-dead);
        else                 return (value+dead)/(1.0f-dead);
    }
}


// Calculates a value (radians) proportional between a and b (degrees).

float Prop(int a, int b, float p)
{
    float   aa, bb;

    aa = (float)a*PI/180.0f;
    bb = (float)b*PI/180.0f;

    return aa+p*(bb-aa);
}

// Gently advanced a desired value from its current value.
// Over time, the greater the progression is rapid.

float Smooth(float actual, float hope, float time)
{
    float   futur;

    futur = actual + (hope-actual)*time;

    if ( hope > actual )
    {
        if ( futur > hope )  futur = hope;
    }
    if ( hope < actual )
    {
        if ( futur < hope )  futur = hope;
    }

    return futur;
}


// Bounces any movement.

//  out
//   |
//  1+------o-------o---
//   |    o | o   o |  | bounce
//   |   o  |   o---|---
//   |  o   |       |
//   | o    |       |
//  -o------|-------+----> progress
//  0|      |       1
//   |<---->|middle

float Bounce(float progress, float middle, float bounce)
{
    if ( progress < middle )
    {
        progress = progress/middle;  // 0..1
        return 0.5f+sinf(progress*PI-PI/2.0f)/2.0f;
    }
    else
    {
        progress = (progress-middle)/(1.0f-middle);  // 0..1
        return (1.0f-bounce/2.0f)+sinf((0.5f+progress*2.0f)*PI)*(bounce/2.0f);
    }
}


// Returns the color corresponding D3DCOLOR.

D3DCOLOR RetColor(float intensity)
{
    D3DCOLOR    color;

    if ( intensity <= 0.0f )  return 0x00000000;
    if ( intensity >= 1.0f )  return 0xffffffff;

    color  = (int)(intensity*255.0f)<<24;
    color |= (int)(intensity*255.0f)<<16;
    color |= (int)(intensity*255.0f)<<8;
    color |= (int)(intensity*255.0f);

    return color;
}

// Returns the color corresponding D3DCOLOR.

D3DCOLOR RetColor(D3DCOLORVALUE intensity)
{
    D3DCOLOR    color;

    color  = (int)(intensity.a*255.0f)<<24;
    color |= (int)(intensity.r*255.0f)<<16;
    color |= (int)(intensity.g*255.0f)<<8;
    color |= (int)(intensity.b*255.0f);

    return color;
}

// Returns the color corresponding D3DCOLORVALUE.

D3DCOLORVALUE RetColor(D3DCOLOR intensity)
{
    D3DCOLORVALUE   color;

    color.r = (float)((intensity>>16)&0xff)/256.0f;
    color.g = (float)((intensity>>8 )&0xff)/256.0f;
    color.b = (float)((intensity>>0 )&0xff)/256.0f;
    color.a = (float)((intensity>>24)&0xff)/256.0f;

    return color;
}


// RGB to HSV conversion.

void RGB2HSV(D3DCOLORVALUE src, ColorHSV &dest)
{
    float   min, max, delta;

    min = Min(src.r, src.g, src.b);
    max = Max(src.r, src.g, src.b);

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

void HSV2RGB(ColorHSV src, D3DCOLORVALUE &dest)
{
    int     i;
    float   f,v,p,q,t;

    src.h = Norm(src.h)*360.0f;
    src.s = Norm(src.s);
    src.v = Norm(src.v);

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

