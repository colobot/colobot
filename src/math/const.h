// math/const.h

/* Math constants */

#pragma once

// Math module namespace
namespace Math
{
  //! Tolerance level -- minimum accepted float value
  const float TOLERANCE = 1e-6f;

  //! PI
  const float PI = 3.14159265358979323846f;
  //! 2 * PI
  const float PI_MUL_2 = 6.28318530717958623200f;
  //! PI / 2
  const float PI_DIV_2 = 1.57079632679489655800f;
  //! PI / 4
  const float PI_DIV_4 = 0.78539816339744827900f;
  //! 1 / PI
  const float INV_PI   = 0.31830988618379069122f;

  //! Degrees to radians multiplier
  const float DEG_TO_RAD =  0.01745329251994329547f;
  //! Radians to degrees multiplier
  const FLOAT RAD_TO_DEG = 57.29577951308232286465f;
};
