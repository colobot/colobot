/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file level/level_loader.h
 */

#pragma once

#include "level/parser/parser.h"

#include <string>

/**
 * \brief List of modes used in CreateScene()
 */
enum LevelLoadMode
{
    Normal          = 1, //!< used when starting new game
    Reset           = 2, //!< used when restarting level
    LoadSavedGame   = 4  //!< used when loading saved game
};

typedef std::function<void(CLevelLoader*, CLevelParserLine*, LevelLoadMode)> CommandFunction;

/**
 * \brief Tool to loading and building scene from file
 */
class CLevelLoader
{
public:
    /**
     * \brief Constructor, should not be used from outside CLevelLoader class,
     * use CreateScene() instead.
     */
    CLevelLoader(bool solutionEnabled);

    /**
     * \brief Build scene according to loaded file
     * \param solutionEnabled If true, show solution
     * \param fixScene If true, scene will be non-interactive (cutscenes, player appearance editor)
     * \param mode See ::LevelLoadMode
     */
    static void CreateScene(bool solutionEnabled, bool fixScene, LevelLoadMode mode);

private:
    struct Command
    {
        Command(CommandFunction function, int executionConditions);

        CommandFunction function;
        int executionConditions;
    };

    void ResetScene(LevelLoadMode mode);
    void BuildScene(bool fixScene, LevelLoadMode mode);

    void ShowLoadingWarning(const std::string& message);

    void HandleCommand_Title(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Resume(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_ScriptName(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_ScriptFile(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Instructions(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Satellite(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Loading(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_HelpFile(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_SoluceFile(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EndingFile(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_MessageDelay(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_MissionTimer(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TeamName(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_CacheAudio(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_AudioChange(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Audio(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_AmbientColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_FogColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_VehicleColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_InsectColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_GreeneryColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_DeepView(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_FogStart(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_SecondTexture(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Background(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Planet(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_ForegroundName(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Level(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainGenerate(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainWind(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainRelief(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainRandomRelief(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainResource(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainWater(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainLava(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainCloud(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainBlitz(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainInitTextures(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainInit(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainMaterial(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainLevel(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_TerrainCreate(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_BeginObject(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_LevelController(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_CreateObject(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_CreateFog(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_CreateLight(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_CreateSpot(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_GroundSpot(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_WaterColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_MapColor(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_MapZoom(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_MaxFlyingHeight(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_AddFlyingHeight(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_Camera(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EndMissionTake(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EndMissionDelay(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EndMissionResearch(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_ObligatoryToken(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_ProhibitedToken(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EnableBuild(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_EnableResearch(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_DoneResearch(CLevelParserLine* line, LevelLoadMode mode);
    void HandleCommand_NewScript(CLevelParserLine* line, LevelLoadMode mode);

    static std::shared_ptr<CLevelLoader::Command> MakeCommand(CommandFunction function, int executionConditions = LevelLoadMode::Normal | LevelLoadMode::LoadSavedGame);

    static std::map<std::string,std::shared_ptr<CLevelLoader::Command>> m_commands;

    CRobotMain* m_main;
    bool        m_solutionEnabled;
    int         m_nextObjectId;
    int         m_objectsNumber;
    std::string m_backgroundPath;
    Gfx::Color  m_backgroundUp;
    Gfx::Color  m_backgroundDown;
    Gfx::Color  m_backgroundCloudUp;
    Gfx::Color  m_backgroundCloudDown;
    bool        m_backgroundFull;
    CObject*    m_selectedObject;
};


