// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

/**
 * \file math/geometry.h
 * \brief Math functions related to 3D geometry calculations, transformations, etc.
 */

#pragma once


#include "math/const.h"
#include "math/func.h"
#include "math/point.h"
#include "math/matrix.h"
#include "math/vector.h"


#include <cmath>
#include <cstdlib>


// Math module namespace
namespace Math {


//! Returns py up on the line \a a - \a b
inline float MidPoint(const Math::Point &a, const Math::Point &b, float px)
{
    if (IsEqual(a.x, b.x))
    {
        if (a.y < b.y)
            return Math::HUGE_NUM;
        else
            return -Math::HUGE_NUM;
    }
    return (b.y-a.y) * (px-a.x) / (b.x-a.x) + a.y;
}

//! Tests whether the point \a p is inside the triangle (\a a,\a b,\a c)
inline bool IsInsideTriangle(Math::Point a, Math::Point b, Math::Point c, Math::Point p)
{
    float n, m;

    if ( p.x < a.x && p.x < b.x && p.x < c.x ) return false;
    if ( p.x > a.x && p.x > b.x && p.x > c.x ) return false;
    if ( p.y < a.y && p.y < b.y && p.y < c.y ) return false;
    if ( p.y > a.y && p.y > b.y && p.y > c.y ) return false;

    if ( a.x > b.x ) Swap(a,b);
    if ( a.x > c.x ) Swap(a,c);
    if ( c.x < a.x ) Swap(c,a);
    if ( c.x < b.x ) Swap(c,b);

    n = MidPoint(a, b, p.x);
    m = MidPoint(a, c, p.x);
    if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) ) return false;

    n = MidPoint(c, b, p.x);
    m = MidPoint(c, a, p.x);
    if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) ) return false;

    return true;
}

//! Rotates a point around a center
/**
 * \param center  center of rotation
 * \param angle   angle [radians] (positive is CCW)
 * \param p       the point to be rotated
 */
inline Math::Point RotatePoint(const Math::Point &center, float angle, const Math::Point &p)
{
    Math::Point a;
    a.x = p.x-center.x;
    a.y = p.y-center.y;

    Math::Point b;
    b.x = a.x*cosf(angle) - a.y*sinf(angle);
    b.y = a.x*sinf(angle) + a.y*cosf(angle);

    b.x += center.x;
    b.y += center.y;

    return b;
}

//! Rotates a point around the origin (0,0)
/**
 * \param angle angle [radians] (positive is CCW)
 * \param p     the point to be rotated
 */
inline Math::Point RotatePoint(float angle, const Math::Point &p)
{
    float x = p.x*cosf(angle) - p.y*sinf(angle);
    float y = p.x*sinf(angle) + p.y*cosf(angle);

    return Math::Point(x, y);
}

//! Rotates a vector (dist, 0)
/**
 * \param angle angle [radians] (positive is CCW)
 * \param dist  distance to origin
 */
inline Math::Point RotatePoint(float angle, float dist)
{
    float x = dist*cosf(angle);
    float y = dist*sinf(angle);

    return Math::Point(x, y);
}

//! Rotates a point around a center on 2D plane
/**
 * \param cx,cy  center of rotation
 * \param angle  angle of rotation [radians] (positive is CCW)
 * \param px,py  point coordinates to rotate
 */
inline void RotatePoint(float cx, float cy, float angle, float &px, float &py)
{
    float ax, ay;

    px -= cx;
    py -= cy;

    ax = px*cosf(angle) - py*sinf(angle);
    ay = px*sinf(angle) + py*cosf(angle);

    px = cx+ax;
    py = cy+ay;
}

//! Rotates a point around a center in space
/**
 * \a angleH is rotation along Y axis (heading) while \a angleV is rotation along X axis (TODO: ?).
 *
 * \param center         center of rotation
 * \param angleH,angleV  rotation angles [radians] (positive is CCW)
 * \param p              the point to be rotated
 */
inline void RotatePoint(const Math::Vector &center, float angleH, float angleV, Math::Vector &p)
{
    p.x -= center.x;
    p.y -= center.y;
    p.z -= center.z;

    Math::Vector b;
    b.x = p.x*cosf(angleH) - p.z*sinf(angleH);
    b.y = p.z*sinf(angleV) + p.y*cosf(angleV);
    b.z = p.x*sinf(angleH) + p.z*cosf(angleH);

    p = center + b;
}

//! Rotates a point around a center in space
/**
 * The rotation is performed first along Y axis (\a angleH) and then along X axis (\a angleV).
 *
 * \param center         center of rotation
 * \param angleH,angleV  rotation angles [radians] (positive is CCW)
 * \param p              the point to be rotated
 */
inline void RotatePoint2(const Math::Vector center, float angleH, float angleV, Math::Vector &p)
{
    p.x -= center.x;
    p.y -= center.y;
    p.z -= center.z;

    Math::Vector a;
    a.x = p.x*cosf(angleH) - p.z*sinf(angleH);
    a.y = p.y;
    a.z = p.x*sinf(angleH) + p.z*cosf(angleH);

    Math::Vector b;
    b.x = a.x;
    b.y = a.z*sinf(angleV) + a.y*cosf(angleV);
    b.z = a.z*cosf(angleV) - a.y*sinf(angleV);

    p = center + b;
}

//! Returns the angle between point (x,y) and (0,0)
inline float RotateAngle(float x, float y)
{
    if (x == 0.0f && y == 0.0f) return 0.0f;

    if (x >= 0.0f)
    {
        if (y >= 0.0f)
        {
            if (x > y) return atanf(y/x);
            else       return PI*0.5f - atanf(x/y);
        }
        else
        {
            if (x > -y) return PI*2.0f + atanf(y/x);
            else        return PI*1.5f - atanf(x/y);
        }
    }
    else
    {
        if (y >= 0.0f)
        {
            if (-x > y) return PI*1.0f + atanf(y/x);
            else        return PI*0.5f - atanf(x/y);
        }
        else
        {
            if (-x > -y) return PI*1.0f + atanf(y/x);
            else         return PI*1.5f - atanf(x/y);
        }
    }
}

//! Calculates the angle between two points and a center
/**
 * \param center  the center point
 * \param p1,p2   the two points
 * \returns       the angle [radians] (positive is CCW)
 */
inline float RotateAngle(const Math::Point &center, const Math::Point &p1, const Math::Point &p2)
{
    if (PointsEqual(p1, center))
        return 0;

    if (PointsEqual(p2, center))
        return 0;

    float a1 = asinf((p1.y - center.y) / Distance(p1, center));
    float a2 = asinf((p2.y - center.y) / Distance(p2, center));

    if (p1.x < center.x) a1 = PI - a1;
    if (p2.x < center.x) a2 = PI - a2;

    float a = a2 - a1;
    if (a < 0)
        a += 2.0f*PI;

    return a;
}

//! Loads view matrix from the given vectors
/**
 * \param mat      result matrix
 * \param from     origin
 * \param at       view direction
 * \param worldUp  up vector
 */
inline void LoadViewMatrix(Math::Matrix &mat, const Math::Vector &from,
                           const Math::Vector &at, const Math::Vector &worldUp)
{
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    Math::Vector view = at - from;

    float length = view.Length();
    assert(! IsZero(length) );

    // Normalize the z basis vector
    view /= length;

    // Get the dot product, and calculate the projection of the z basis
    // vector onto the up vector. The projection is the y basis vector.
    float dotProduct = DotProduct(worldUp, view);

    Math::Vector up = worldUp - dotProduct * view;

    // If this vector has near-zero length because the input specified a
    // bogus up vector, let's try a default up vector
    if ( IsZero(length = up.Length()) )
    {
        up = Math::Vector(0.0f, 1.0f, 0.0f) - view.y * view;

        // If we still have near-zero length, resort to a different axis.
        if ( IsZero(length = up.Length()) )
        {
            up = Math::Vector(0.0f, 0.0f, 1.0f) - view.z * view;

            assert(! IsZero(up.Length()) );
        }
    }

    // Normalize the y basis vector
    up /= length;

    // The x basis vector is found simply with the cross product of the y
    // and z basis vectors
    Math::Vector right = CrossProduct(up, view);

    // Start building the matrix. The first three rows contains the basis
    // vectors used to rotate the view to point at the lookat point
    mat.LoadIdentity();

    /* (1,1) */ mat.m[0 ] = right.x;
    /* (2,1) */ mat.m[1 ] = up.x;
    /* (3,1) */ mat.m[2 ] = view.x;
    /* (1,2) */ mat.m[4 ] = right.y;
    /* (2,2) */ mat.m[5 ] = up.y;
    /* (3,2) */ mat.m[6 ] = view.y;
    /* (1,3) */ mat.m[8 ] = right.z;
    /* (2,3) */ mat.m[9 ] = up.z;
    /* (3,3) */ mat.m[10] = view.z;

    // Do the translation values (rotations are still about the eyepoint)
    /* (1,4) */ mat.m[12] = -DotProduct(from, right);
    /* (2,4) */ mat.m[13] = -DotProduct(from, up);
    /* (3,4) */ mat.m[14] = -DotProduct(from, view);
}

//! Loads a perspective projection matrix
/**
 * \param mat        result matrix
 * \param fov        field of view in radians
 * \param aspect     aspect ratio (width / height)
 * \param nearPlane  distance to near cut plane
 * \param farPlane   distance to far cut plane
 */
inline void LoadProjectionMatrix(Math::Matrix &mat, float fov = Math::PI / 2.0f, float aspect = 1.0f,
                                 float nearPlane = 1.0f, float farPlane = 1000.0f)
{
    assert(fabs(farPlane - nearPlane) >= 0.01f);
    assert(fabs(sin(fov / 2)) >= 0.01f);

    float f = cosf(fov / 2.0f) / sinf(fov / 2.0f);

    mat.LoadZero();

    /* (1,1) */ mat.m[0 ] = f / aspect;
    /* (2,2) */ mat.m[5 ] = f;
    /* (3,3) */ mat.m[10] = (nearPlane + farPlane) / (nearPlane - farPlane);
    /* (4,3) */ mat.m[11] = -1.0f;
    /* (3,4) */ mat.m[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
}

//! Loads an othogonal projection matrix
/**
 * \param mat         result matrix
 * \param left,right  coordinates for left and right vertical clipping planes
 * \param bottom,top  coordinates for bottom and top horizontal clipping planes
 * \param zNear,zFar  distance to nearer and farther depth clipping planes
 */
inline void LoadOrthoProjectionMatrix(Math::Matrix &mat, float left, float right, float bottom, float top,
                                      float zNear = -1.0f, float zFar = 1.0f)
{
    mat.LoadIdentity();

    /* (1,1) */ mat.m[0 ] =  2.0f / (right - left);
    /* (2,2) */ mat.m[5 ] =  2.0f / (top - bottom);
    /* (3,3) */ mat.m[10] = -2.0f / (zFar - zNear);

    /* (1,4) */ mat.m[12] =  - (right + left) / (right - left);
    /* (2,4) */ mat.m[13] =  - (top + bottom) / (top - bottom);
    /* (3,4) */ mat.m[14] =  - (zFar + zNear) / (zFar - zNear);
}

//! Loads a translation matrix from given vector
/**
 * \param mat   result matrix
 * \param trans vector of translation
 */
inline void LoadTranslationMatrix(Math::Matrix &mat, const Math::Vector &trans)
{
    mat.LoadIdentity();
    /* (1,4) */ mat.m[12] = trans.x;
    /* (2,4) */ mat.m[13] = trans.y;
    /* (3,4) */ mat.m[14] = trans.z;
}

//! Loads a scaling matrix fom given vector
/**
 * \param mat    result matrix
 * \param scale  vector with scaling factors for X, Y, Z
 */
inline void LoadScaleMatrix(Math::Matrix &mat, const Math::Vector &scale)
{
    mat.LoadIdentity();
    /* (1,1) */ mat.m[0 ] = scale.x;
    /* (2,2) */ mat.m[5 ] = scale.y;
    /* (3,3) */ mat.m[10] = scale.z;
}

//! Loads a rotation matrix along the X axis
/**
 * \param mat    result matrix
 * \param angle  angle [radians]
 */
inline void LoadRotationXMatrix(Math::Matrix &mat, float angle)
{
    mat.LoadIdentity();
    /* (2,2) */ mat.m[5 ] =  cosf(angle);
    /* (3,2) */ mat.m[6 ] =  sinf(angle);
    /* (2,3) */ mat.m[9 ] = -sinf(angle);
    /* (3,3) */ mat.m[10] =  cosf(angle);
}

//! Loads a rotation matrix along the Y axis
/**
 * \param mat    result matrix
 * \param angle  angle [radians]
 */
inline void LoadRotationYMatrix(Math::Matrix &mat, float angle)
{
    mat.LoadIdentity();
    /* (1,1) */ mat.m[0 ] =  cosf(angle);
    /* (3,1) */ mat.m[2 ] = -sinf(angle);
    /* (1,3) */ mat.m[8 ] =  sinf(angle);
    /* (3,3) */ mat.m[10] =  cosf(angle);
}

//! Loads a rotation matrix along the Z axis
/**
 * \param mat    result matrix
 * \param angle  angle [radians]
 */
inline void LoadRotationZMatrix(Math::Matrix &mat, float angle)
{
    mat.LoadIdentity();
    /* (1,1) */ mat.m[0 ] =  cosf(angle);
    /* (2,1) */ mat.m[1 ] =  sinf(angle);
    /* (1,2) */ mat.m[4 ] = -sinf(angle);
    /* (2,2) */ mat.m[5 ] =  cosf(angle);
}

//! Loads a rotation matrix along the given axis
/**
 * \param mat    result matrix
 * \param dir    axis of rotation
 * \param angle  angle [radians]
 */
inline void LoadRotationMatrix(Math::Matrix &mat, const Math::Vector &dir, float angle)
{
    float cos = cosf(angle);
    float sin = sinf(angle);
    Math::Vector v = Normalize(dir);

    mat.LoadIdentity();

    /* (1,1) */ mat.m[0 ] = (v.x * v.x) * (1.0f - cos) + cos;
    /* (2,1) */ mat.m[1 ] = (v.x * v.y) * (1.0f - cos) - (v.z * sin);
    /* (3,1) */ mat.m[2 ] = (v.x * v.z) * (1.0f - cos) + (v.y * sin);

    /* (1,2) */ mat.m[4 ] = (v.y * v.x) * (1.0f - cos) + (v.z * sin);
    /* (2,2) */ mat.m[5 ] = (v.y * v.y) * (1.0f - cos) + cos ;
    /* (3,2) */ mat.m[6 ] = (v.y * v.z) * (1.0f - cos) - (v.x * sin);

    /* (1,3) */ mat.m[8 ] = (v.z * v.x) * (1.0f - cos) - (v.y * sin);
    /* (2,3) */ mat.m[9 ] = (v.z * v.y) * (1.0f - cos) + (v.x * sin);
    /* (3,3) */ mat.m[10] = (v.z * v.z) * (1.0f - cos) + cos;
}

//! Calculates the matrix to make three rotations in the order X, Z and Y
inline void LoadRotationXZYMatrix(Math::Matrix &mat, const Math::Vector &angles)
{
    Math::Matrix temp;
    LoadRotationXMatrix(temp, angles.x);

    LoadRotationZMatrix(mat, angles.z);
    mat = Math::MultiplyMatrices(temp, mat);

    LoadRotationYMatrix(temp, angles.y);
    mat = Math::MultiplyMatrices(temp, mat);
}

//! Calculates the matrix to make three rotations in the order Z, X and Y
inline void LoadRotationZXYMatrix(Math::Matrix &mat, const Math::Vector &angles)
{
    Math::Matrix temp;
    LoadRotationZMatrix(temp, angles.z);

    LoadRotationXMatrix(mat, angles.x);
    mat = Math::MultiplyMatrices(temp, mat);

    LoadRotationYMatrix(temp, angles.y);
    mat = Math::MultiplyMatrices(temp, mat);
}

//! Returns the distance between projections on XZ plane of two vectors
inline float DistanceProjected(const Math::Vector &a, const Math::Vector &b)
{
    return sqrtf( (a.x-b.x)*(a.x-b.x) +
                  (a.z-b.z)*(a.z-b.z) );
}

//! Returns the normal vector to a plane
/**
 * \param p1,p2,p3 points defining the plane
 */
inline Math::Vector NormalToPlane(const Math::Vector &p1, const Math::Vector &p2, const Math::Vector &p3)
{
    Math::Vector u = p3 - p1;
    Math::Vector v = p2 - p1;

    return Normalize(CrossProduct(u, v));
}

//! Returns a point on the line \a p1 - \a p2, in \a dist distance from \a p1
/**
 * \param p1,p2  line start and end
 * \param dist   scaling factor from \a p1, relative to distance between \a p1 and \a p2
 */
inline Math::Vector SegmentPoint(const Math::Vector &p1, const Math::Vector &p2, float dist)
{
    return p1 + (p2 - p1) * dist;
}

//! Returns the distance between given point and a plane
/**
 * \param p      the point
 * \param a,b,c  points defining the plane
 */
inline float DistanceToPlane(const Math::Vector &a, const Math::Vector &b,
                             const Math::Vector &c, const Math::Vector &p)
{
    Math::Vector n = NormalToPlane(a, b, c);
    float d = -(n.x*a.x + n.y*a.y + n.z*a.z);

    return fabs(n.x*p.x + n.y*p.y + n.z*p.z + d);
}

//! Checks if two planes defined by three points are the same
/**
 * \param plane1  array of three vectors defining the first plane
 * \param plane2  array of three vectors defining the second plane
 */
inline bool IsSamePlane(const Math::Vector (&plane1)[3], const Math::Vector (&plane2)[3])
{
    Math::Vector n1 = NormalToPlane(plane1[0], plane1[1], plane1[2]);
    Math::Vector n2 = NormalToPlane(plane2[0], plane2[1], plane2[2]);

    if ( fabs(n1.x-n2.x) > 0.1f ||
         fabs(n1.y-n2.y) > 0.1f ||
         fabs(n1.z-n2.z) > 0.1f )
        return false;

    float dist = DistanceToPlane(plane1[0], plane1[1], plane1[2], plane2[0]);
    if ( dist > 0.1f )
        return false;

    return true;
}

//! Calculates the intersection "i" right "of" the plane "abc" (TODO: ?)
inline bool Intersect(const Math::Vector &a, const Math::Vector &b, const Math::Vector &c,
                      const Math::Vector &d, const Math::Vector &e, Math::Vector &i)
{
    float d1 = (d.x-a.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
               (d.y-a.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
               (d.z-a.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

    float d2 = (d.x-e.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
               (d.y-e.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
               (d.z-e.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

    if (d2 == 0)
        return false;

    i.x = d.x + d1/d2*(e.x-d.x);
    i.y = d.y + d1/d2*(e.y-d.y);
    i.z = d.z + d1/d2*(e.z-d.z);

    return true;
}

//! Calculates the intersection of the straight line passing through p (x, z)
/** Line is parallel to the y axis, with the plane abc. Returns p.y. (TODO: ?) */
inline bool IntersectY(const Math::Vector &a, const Math::Vector &b, const Math::Vector &c, Math::Vector &p)
{
    float d  = (b.x-a.x)*(c.z-a.z) - (c.x-a.x)*(b.z-a.z);
    float d1 = (p.x-a.x)*(c.z-a.z) - (c.x-a.x)*(p.z-a.z);
    float d2 = (b.x-a.x)*(p.z-a.z) - (p.x-a.x)*(b.z-a.z);

    if (d == 0.0f)
        return false;

    p.y = a.y + d1/d*(b.y-a.y) + d2/d*(c.y-a.y);

    return true;
}

//! Calculates the end point
inline Math::Vector LookatPoint(const Math::Vector &eye, float angleH, float angleV, float length)
{
    Math::Vector lookat = eye;
    lookat.z += length;

    RotatePoint(eye, angleH, angleV, lookat);

    return lookat;
}

//! Transforms the point \a p by matrix \a m
/** Is equal to multiplying the matrix by the vector (of course without perspective divide). */
inline Math::Vector Transform(const Math::Matrix &m, const Math::Vector &p)
{
    return MatrixVectorMultiply(m, p);
}

//! Calculates the projection of the point \a p on a straight line \a a to \a b
/**
 * \param p      point to project
 * \param a,b    two ends of the line
 */
inline Math::Vector Projection(const Math::Vector &a, const Math::Vector &b, const Math::Vector &p)
{
    float k = DotProduct(b - a, p - a);
    k /= DotProduct(b - a, b - a);

    return a + k*(b-a);
}

//! Calculates point of view to look at a center two angles and a distance
inline Math::Vector RotateView(Math::Vector center, float angleH, float angleV, float dist)
{
    Math::Matrix mat1, mat2;
    LoadRotationZMatrix(mat1, -angleV);
    LoadRotationYMatrix(mat2, -angleH);

    Math::Matrix mat = MultiplyMatrices(mat2, mat1);

    Math::Vector eye;
    eye.x = 0.0f+dist;
    eye.y = 0.0f;
    eye.z = 0.0f;
    eye = Transform(mat, eye);

    return eye+center;
}


} // namespace Math
