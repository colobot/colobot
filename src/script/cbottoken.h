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

/**
 * \file script/cbottoken.h
 * \brief Functions to parse some CBot-related tokens
 */

#pragma once


#include "object/object.h"



// Procedures.

extern const char* GetObjectName(ObjectType type);
extern const char* GetObjectAlias(ObjectType type);
extern std::string GetHelpFilename(ObjectType type);
extern std::string GetHelpFilename(const char *token);
extern bool IsType(const char *token);
extern bool IsFunction(const char *token);
extern const char* GetHelpText(const char *token);

