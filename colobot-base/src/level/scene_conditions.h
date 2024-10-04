/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file level/scene_conditions.h
 * \brief Classes for managing conditions in scene files
 */

#pragma once

#include "common/error.h"
#include "common/global.h"

#include "object/drive_type.h"
#include "object/object_type.h"
#include "object/tool_type.h"

#include <glm/glm.hpp>

#include <filesystem>

class CLevelParserLine;
class CObject;

/**
 * \class CObjectCondition
 * \brief Base object condition structure
 */
class CObjectCondition
{
public:
    glm::vec3     pos = glm::vec3(0.0f, 0.0f, 0.0f) * g_unit;
    float         dist = 8.0f*g_unit;
    ObjectType    type = OBJECT_NULL;
    float         powermin = -1;  // wins if energy cell >=
    float         powermax = 100; // wins if energy cell <=
    ToolType      tool = ToolType::Other;
    DriveType     drive = DriveType::Other;
    bool          countTransported = true;
    int           team = 0;

    //! Read from line in scene file
    virtual void Read(CLevelParserLine* line);

    //! Checks if this condition is met
    bool CheckForObject(CObject* obj);

    //! Count all object matching the conditions
    int CountObjects();
};

/**
 * \class CSceneCondition
 * \brief Base scene condition structure
 */
class CSceneCondition : public CObjectCondition
{
public:
    int           min = 1;        // wins if >
    int           max = 9999;     // wins if <

    //! Read from line in scene file
    void Read(CLevelParserLine* line) override;

    //! Checks if this condition is met
    bool Check();
};

/**
 * \class CSceneEndCondition
 * \brief Scene end condition
 */
class CSceneEndCondition final : public CSceneCondition
{
public:
    int winTeam = 0;

    int lost = -1; // lost if <=

    //! If this is true, the mission ends as soon as this requirement is met, without having to complete the others
    bool immediat = false;

    //! Read from line in scene file
    void Read(CLevelParserLine* line) override;

    //! Checks if lost condition is met
    bool CheckLost();

    //! Get mission result
    Error GetMissionResult();
};

/**
 * \class CAudioChangeCondition
 * \brief Audio change condition
 */
class CAudioChangeCondition final : public CSceneCondition
{
public:
    std::filesystem::path music = "";
    bool repeat = true;

    bool changed = false;

    //! Read from line in scene file
    void Read(CLevelParserLine* line) override;
};
