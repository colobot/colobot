// math/vector.h

/* Vector struct and functions */

#pragma once

#include "const.h"

#include <cmath>

/*
 TODO

HRESULT D3DMath_VectorMatrixMultiply( D3DVECTOR& vDest, D3DVECTOR& vSrc, D3DMATRIX& mat)

float   Length(const D3DVECTOR &a, const D3DVECTOR &b);
float   Length2d(const D3DVECTOR &a, const D3DVECTOR &b);
float   Angle( D3DVECTOR u, D3DVECTOR v );
D3DVECTOR Cross( D3DVECTOR u, D3DVECTOR v );
D3DVECTOR ComputeNormal( D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3 );
D3DVECTOR Transform(const D3DMATRIX &m, D3DVECTOR p);
D3DVECTOR Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p);

D3DVECTOR SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist);

BOOL      Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i);
BOOL      IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p);


D3DVECTOR RotateView(D3DVECTOR center, float angleH, float angleV, float dist);
D3DVECTOR LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );

void      MappingObject( D3DVERTEX2* pVertices, int nb, float scale );
void      SmoothObject( D3DVERTEX2* pVertices, int nb );

float   DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &c, const D3DVECTOR &p);
BOOL      IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2);
 */

// Math module namespace
namespace Math
{

/** 4x1 Vector

  Represents an universal 4x1 vector that can be used in OpenGL and DirectX engines.
  Contains the required methods for operating on vectors.

  All methods are made inline to maximize optimization.

 */
struct Vector
{
  //! X - 1st coord
  float x;
  //! Y - 2nd coord
  float y;
  //! Z - 3rd coord
  float z;
  //! W - 4th coord
  float w;

  //! Creates an identity vector (0, 0, 0, 1)
  Vector()
  {
    LoadIdentity();
  }

  //! Creates a vector from given values
  Vector(float x, float y, float z, float w = 1.0f)
  {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  //! Loads the identity vector (0, 0, 0, 1)
  void LoadIdentity()
  {
    x = y = z = 0.0f;
    w = 1.0f;
  }

  //! Loads the zero vector (0, 0, 0, 0)
  void LoadZero()
  {
    x = y = z = w = 0.0f;
  }

  //! Returns the vector length
  float Length() const
  {
    return sqrt(x*x + y*y + z*z + w*w);
  }

  //! Normalizes the vector
  void Normalize()
  {
    
  }

  const Vector3D& operator-();
  const Vector3D& operator+=(const Vector3D &vector);
  const Vector3D& operator-=(const Vector3D &vector);
  const Vector3D& operator*=(double value);
  const Vector3D& operator/=(double value);
  
  friend Vector3D operator+(const Vector3D &left, const Vector3D &right);
  friend Vector3D operator-(const Vector3D &left, const Vector3D &right);
  friend Vector3D operator*(double left, const Vector3D &right);
  friend Vector3D operator*(const Vector3D &left, double right);
  friend Vector3D operator/(const Vector3D &left, double right);
  
  friend Vector3D crossProduct(const Vector3D &left, const Vector3D &right);
  friend double dotProduct(const Vector3D &left, const Vector3D &right);
  friend double cosAngle(const Vector3D &vector1, const Vector3D &vector2);
  friend double angle(const Vector3D &vector1, const Vector3D &vector2);
};