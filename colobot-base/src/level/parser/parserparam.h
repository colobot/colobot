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
 * \file level/parser/parserparam.h
 * \brief Value of command argument in level file
 */

#pragma once

#include "graphics/core/color.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/planet_type.h"
#include "graphics/engine/pyro_type.h"
#include "graphics/engine/water.h"

#include "level/scoreboard.h"

#include "object/drive_type.h"
#include "object/mission_type.h"
#include "object/object_type.h"
#include "object/tool_type.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

class CLevelParserLine;

class CLevelParserParam;
using CLevelParserParamUPtr = std::unique_ptr<CLevelParserParam>;
using CLevelParserParamVec = std::vector<CLevelParserParamUPtr>;

namespace Gfx
{
class CEngine;

enum EngineObjectType : unsigned char;
}

class CLevelParserParam
{
public:
    //! Create param with given value
    //@{
    CLevelParserParam(int value);
    CLevelParserParam(float value);
    CLevelParserParam(std::string value);
    CLevelParserParam(bool value);
    CLevelParserParam(Gfx::Color value);
    CLevelParserParam(glm::vec2 value);
    CLevelParserParam(glm::vec3 value);
    CLevelParserParam(ObjectType value);
    CLevelParserParam(Gfx::CameraType value);
    CLevelParserParam(const std::filesystem::path& value);
    CLevelParserParam(CLevelParserParamVec&& array);
    //@}
    //! Create param from string
    CLevelParserParam(std::string name, std::string value);
    //! Create empty parser param
    CLevelParserParam(std::string name, bool empty);

    //! Get value (throws exception if not found or unable to process)
    //@{
    int AsInt();
    float AsFloat();
    std::string AsString();
    bool AsBool();
    std::filesystem::path AsPath(const std::filesystem::path& defaultDir);
    Gfx::Color AsColor();
    glm::vec3 AsPoint();
    ObjectType AsObjectType();
    DriveType AsDriveType();
    ToolType AsToolType();
    Gfx::WaterType AsWaterType();
    Gfx::EngineObjectType AsTerrainType();
    int AsBuildFlag();
    int AsResearchFlag();
    CScoreboard::SortType AsSortType();
    Gfx::PyroType AsPyroType();
    Gfx::CameraType AsCameraType();
    MissionType AsMissionType();
    const CLevelParserParamVec& AsArray();
    Gfx::PlanetType AsPlanetType();
    //@}

    //! Get value (returns default if not found, throws exception if unable to process)
    //@{
    int AsInt(int def);
    float AsFloat(float def);
    std::string AsString(std::string def);
    bool AsBool(bool def);
    std::filesystem::path AsPath(const std::filesystem::path& defaultDir, const std::filesystem::path& def);
    Gfx::Color AsColor(Gfx::Color def);
    glm::vec3 AsPoint(glm::vec3 def);
    ObjectType AsObjectType(ObjectType def);
    DriveType AsDriveType(DriveType def);
    ToolType AsToolType(ToolType def);
    Gfx::WaterType AsWaterType(Gfx::WaterType def);
    Gfx::EngineObjectType AsTerrainType(Gfx::EngineObjectType def);
    int AsBuildFlag(int def);
    int AsResearchFlag(int def);
    CScoreboard::SortType AsSortType(CScoreboard::SortType def);
    Gfx::PyroType AsPyroType(Gfx::PyroType def);
    Gfx::CameraType AsCameraType(Gfx::CameraType def);
    MissionType AsMissionType(MissionType def);
    //@}

    //! Set line this param is part of
    void SetLine(CLevelParserLine* line);
    //! Get line this param is part of
    CLevelParserLine* GetLine();

    std::string GetName();
    std::string GetValue();
    bool IsDefined();

    static const std::string FromObjectType(ObjectType value);

private:
    void ParseArray();
    void LoadArray();

    template<typename T> T Cast(const std::string& value, const std::string& requestedType);
    template<typename T> T Cast(const std::string& requestedType);

    std::filesystem::path ToPath(const std::string& path, const std::filesystem::path& defaultDir);
    ObjectType ToObjectType(std::string value);
    DriveType ToDriveType(std::string value);
    ToolType ToToolType(std::string value);
    Gfx::WaterType ToWaterType(std::string value);
    Gfx::EngineObjectType ToTerrainType(std::string value);
    int ToBuildFlag(std::string value);
    int ToResearchFlag(std::string value);
    CScoreboard::SortType ToSortType(std::string value);
    Gfx::PyroType ToPyroType(std::string value);
    Gfx::CameraType ToCameraType(std::string value);
    MissionType ToMissionType(std::string value);

    const std::string FromCameraType(Gfx::CameraType value);

private:
    CLevelParserLine* m_line = nullptr;
    bool m_empty = false;
    std::string m_name;
    std::string m_value;
    CLevelParserParamVec m_array;
};
