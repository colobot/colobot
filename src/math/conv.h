/* math/conv.h

 Temporary conversion functions for D3DVECTOR and FPOINT */

#pragma once

#define STRICT
#define D3D_OVERLOADS
#include <d3d.h>

#include "vector.h"

inline D3DVECTOR V_TO_D3D(Math::Vector vec)
{
  return D3DVECTOR(vec.x, vec.y, vec.z);
}

inline Math::Vector D3D_TO_V(D3DVECTOR vec)
{
  return Math::Vector(vec.x, vec.y, vec.z);
}
