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
 * \file object/level/parserparam.h
 * \brief Value of command argument in level file
 */

#pragma once

#include <object/object.h>
#include <graphics/core/color.h>
#include <graphics/engine/water.h>
#include <graphics/engine/pyro.h>
#include <math/point.h>

#include <string>

class CLevelParserLine;

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
    CLevelParserParam(Math::Point value);
    CLevelParserParam(ObjectType value);
    CLevelParserParam(Gfx::CameraType value);
    CLevelParserParam(const std::vector<CLevelParserParam*>& value);
    //@}
    //! Create param from string
    CLevelParserParam(std::string name, std::string value);
    //! Create empty parser param
    CLevelParserParam(std::string name, bool empty);
    
    ~CLevelParserParam();
    
    //! Get value (throws exception if not found or unable to process)
    //@{
    int AsInt();
    float AsFloat();
    std::string AsString();
    bool AsBool();
    std::string AsPath(const std::string defaultDir);
    Gfx::Color AsColor();
    Math::Vector AsPoint();
    ObjectType AsObjectType();
    DriveType AsDriveType();
    ToolType AsToolType();
    Gfx::WaterType AsWaterType();
    Gfx::EngineObjectType AsTerrainType();
    int AsBuildFlag();
    int AsResearchFlag();
    Gfx::PyroType AsPyroType();
    Gfx::CameraType AsCameraType();
    const std::vector<CLevelParserParam*>& AsArray();
    //@}
    
    //! Get value (returns default if not found, throws exception if unable to process)
    //@{
    int AsInt(int def);
    float AsFloat(float def);
    std::string AsString(std::string def);
    bool AsBool(bool def);
    std::string AsPath(const std::string defaultDir, std::string def);
    Gfx::Color AsColor(Gfx::Color def);
    Math::Vector AsPoint(Math::Vector def);
    ObjectType AsObjectType(ObjectType def);
    DriveType AsDriveType(DriveType def);
    ToolType AsToolType(ToolType def);
    Gfx::WaterType AsWaterType(Gfx::WaterType def);
    Gfx::EngineObjectType AsTerrainType(Gfx::EngineObjectType def);
    int AsBuildFlag(int def);
    int AsResearchFlag(int def);
    Gfx::PyroType AsPyroType(Gfx::PyroType def);
    Gfx::CameraType AsCameraType(Gfx::CameraType def);
    //@}
    
    //! Set line this param is part of
    void SetLine(CLevelParserLine* line);
    //! Get line this param is part of
    CLevelParserLine* GetLine();
    
    std::string GetName();
    std::string GetValue();
    bool IsDefined();
    
    static std::string InjectLevelDir(std::string path, const std::string defaultDir);
    
private:
    void ParseArray();
    
    template<typename T> T Cast(std::string value, std::string requestedType);
    template<typename T> T Cast(std::string requestedType);
    
    std::string ToPath(std::string path, const std::string defaultDir);
    ObjectType ToObjectType(std::string value);
    DriveType ToDriveType(std::string value);
    ToolType ToToolType(std::string value);
    Gfx::WaterType ToWaterType(std::string value);
    Gfx::EngineObjectType ToTerrainType(std::string value);
    int ToBuildFlag(std::string value);
    int ToResearchFlag(std::string value);
    Gfx::PyroType ToPyroType(std::string value);
    Gfx::CameraType ToCameraType(std::string value);
    
    const std::string FromObjectType(ObjectType value);
    const std::string FromCameraType(Gfx::CameraType value);
    
private:
    CLevelParserLine* m_line;
    bool m_empty;
    std::string m_name;
    std::string m_value;
    std::vector<CLevelParserParam*> m_array;
};