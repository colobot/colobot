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

// cmdtoken.h

#pragma once


// #include "old/d3denum.h"
// #include "old/d3dengine.h"
// #include "object/object.h"
#include "graphics/engine/water.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/pyro.h"



// Procedures.

extern bool Cmd(char *line, const char *token);
extern char* SearchOp(char *line, const char *op);

extern int GetInt(char *line, int rank, int def);
extern float GetFloat(char *line, int rank, float def);
extern void GetString(char *line, int rank, char *buffer);
extern ObjectType GetTypeObject(char *line, int rank, ObjectType def);
extern const char* GetTypeObject(ObjectType type);
extern Gfx::WaterType GetTypeWater(char *line, int rank, Gfx::WaterType def);
extern Gfx::EngineObjectType GetTypeTerrain(char *line, int rank, Gfx::EngineObjectType def);
extern int GetBuild(char *line, int rank);
extern int GetResearch(char *line, int rank);
extern Gfx::PyroType GetPyro(char *line, int rank);
extern Gfx::CameraType GetCamera(char *line, int rank);
extern const char* GetCamera(Gfx::CameraType type);

extern int OpInt(char *line, const char *op, int def);
extern float OpFloat(char *line, const char *op, float def);
extern void OpString(char *line, char *op, char *buffer);
extern ObjectType OpTypeObject(char *line, char *op, ObjectType def);
extern Gfx::WaterType OpTypeWater(char *line, char *op, Gfx::WaterType def);
extern Gfx::EngineObjectType OpTypeTerrain(char *line, char *op, Gfx::EngineObjectType def);
extern int OpResearch(char *line, char *op);
extern Gfx::PyroType OpPyro(char *line, char *op);
extern Gfx::CameraType OpCamera(char *line, const char *op);
extern int OpBuild(char *line, char *op);
extern Math::Vector OpPos(char *line, char *op);
extern Math::Vector OpDir(char *line, const char *op);
extern Gfx::Color OpColor(char *line, char *op, Gfx::Color def);

