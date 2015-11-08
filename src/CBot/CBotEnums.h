/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

#pragma once

// Modules inlcude

// Local include

// Global include

/*! \brief CBotType Defines known types. This types are modeled on Java types.
 * Do not change the order of elements
 */
enum CBotType
{
    CBotTypVoid         = 0,
    CBotTypByte         = 1,                //n
    CBotTypShort        = 2,                //n
    CBotTypChar         = 3,                //n
    CBotTypInt          = 4,
    CBotTypLong         = 5,                //n
    CBotTypFloat        = 6,
    CBotTypDouble       = 7,                //n
    CBotTypBoolean      = 8,
    CBotTypString       = 9,

    CBotTypArrayPointer = 10,                // array of variables
    CBotTypArrayBody    = 11,                // same but creates an instance

    CBotTypPointer      = 12,                // pointer to an instance
    CBotTypNullPointer  = 13,                // null pointer is special
    CBotTypClass        = 15,
    CBotTypIntrinsic    = 16                // instance of a class intrinsic
};

//! \brief CBotGet Different modes for GetPosition.
enum CBotGet
{
    GetPosExtern = 1,
    GetPosNom    = 2,
    GetPosParam  = 3,
    GetPosBloc   = 4
};
