/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

/**
 * \file common/key.h
 * \brief Key-related macros and enums
 */

#pragma once


#include <SDL_keysym.h>

/* Key definitions are specially defined here so that it is clear in other parts of the code
  that these are used. It is to avoid having SDL-related enum values or #defines lying around
  unchecked. With this approach it will be easier to maintain the code later on. */

// Key symbol defined as concatenation to SDLK_...
// If need arises, it can be changed to custom function or anything else
#define KEY(x) SDLK_ ## x


// Key modifier defined as concatenation to KMOD_...
// If need arises, it can be changed to custom function or anything else
#define KEY_MOD(x) KMOD_ ## x

/**
 * \enum VirtualKmod
 * \brief Virtual key codes generated on kmod presses
 *
 * These are provided here because left and right pair of keys generate different codes.
 */
enum VirtualKmod
{
    VIRTUAL_KMOD_CTRL  = SDLK_LAST + 100, //! < control (left or right)
    VIRTUAL_KMOD_SHIFT = SDLK_LAST + 101, //! < shift (left or right)
    VIRTUAL_KMOD_ALT   = SDLK_LAST + 102, //! < alt (left or right)
    VIRTUAL_KMOD_META  = SDLK_LAST + 103  //! < win key (left or right)
};

// Just syntax sugar
// So it is the same as other macros
#define VIRTUAL_KMOD(x) VIRTUAL_KMOD_ ## x

//! Converts individual codes to virtual keys if needed
unsigned int GetVirtualKey(unsigned int key);

// Virtual key code generated on joystick button presses
// num is number of joystick button
#define VIRTUAL_JOY(num) (SDLK_LAST + 200 + num)

//! Special value for invalid key bindings
const unsigned int KEY_INVALID = SDLK_LAST + 1000;

/**
 * \enum InputSlot
 * \brief Available slots for input bindings
 * NOTE: When adding new values, remember to also update keyTable in input.cpp and their descriptions in restext.cpp
 */
enum InputSlot
{
    INPUT_SLOT_LEFT    = 0,
    INPUT_SLOT_RIGHT   = 1,
    INPUT_SLOT_UP      = 2,
    INPUT_SLOT_DOWN    = 3,
    INPUT_SLOT_GUP     = 4,
    INPUT_SLOT_GDOWN   = 5,
    INPUT_SLOT_CAMERA  = 6,
    INPUT_SLOT_DESEL   = 7,
    INPUT_SLOT_ACTION  = 8,
    INPUT_SLOT_NEAR    = 9,
    INPUT_SLOT_AWAY    = 10,
    INPUT_SLOT_NEXT    = 11,
    INPUT_SLOT_HUMAN   = 12,
    INPUT_SLOT_QUIT    = 13,
    INPUT_SLOT_HELP    = 14,
    INPUT_SLOT_PROG    = 15,
    INPUT_SLOT_VISIT   = 16,
    INPUT_SLOT_SPEED05 = 17,
    INPUT_SLOT_SPEED10 = 18,
    INPUT_SLOT_SPEED15 = 19,
    INPUT_SLOT_SPEED20 = 20,
    INPUT_SLOT_SPEED30 = 21,
    INPUT_SLOT_SPEED40 = 22,
    INPUT_SLOT_CAMERA_UP   = 23,
    INPUT_SLOT_CAMERA_DOWN = 24,
    INPUT_SLOT_PAUSE       = 25,
    
    INPUT_SLOT_MAX
};

/**
 * \enum JoyAxisSlot
 * \brief Slots for joystick axes inputs
 */
enum JoyAxisSlot
{
    JOY_AXIS_SLOT_X,
    JOY_AXIS_SLOT_Y,
    JOY_AXIS_SLOT_Z,
    
    JOY_AXIS_SLOT_MAX
};