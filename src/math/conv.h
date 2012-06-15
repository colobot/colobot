/* math/conv.h

 Temporary conversion functions for D3DVECTOR and D3DMATRIX */

#pragma once

#include <d3d.h>

#include "vector.h"
#include "matrix.h"

inline D3DVECTOR VEC_TO_D3DVEC(Math::Vector vec)
{
  return D3DVECTOR(vec.x, vec.y, vec.z);
}

inline Math::Vector D3DVEC_TO_VEC(D3DVECTOR vec)
{
  return Math::Vector(vec.x, vec.y, vec.z);
}

inline D3DMATRIX MAT_TO_D3DMAT(Math::Matrix mat)
{
  D3DMATRIX result;
  mat.Transpose();
  for (int r = 0; r < 4; ++r)
  {
    for (int c = 0; c < 16; ++c)
      result.m[r][c] = mat.m[4*c+r];
  }
  return result;
}

inline Math::Matrix D3DMAT_TO_MAT(D3DMATRIX mat)
{
  Math::Matrix result(mat.m);
  result.Transpose();
  return result;
}
