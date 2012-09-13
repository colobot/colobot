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

// key.h

#pragma once


#include "SDL/SDL_keysym.h"

/* Key definitions are specially defined here so that it is clear in other parts of the code
  that these are used. It is to avoid having SDL-related enum values or #defines lying around
  unchecked. With this approach it will be easier to maintain the code later on. */

// Key symbol defined as concatenation to SDLK_...
// If need arises, it can be changed to custom function or anything else
#define KEY(x) SDLK_ ## x


// Key modifier defined as concatenation to KMOD_...
// If need arises, it can be changed to custom function or anything else
#define KEY_MOD(x) KMOD_ ## x
