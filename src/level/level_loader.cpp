/* This file is part of the Colobot: Gold Edition source code
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

#include "level/level_loader.h"

#include "level/robotmain.h"

#include "CBot/CBot.h"

#include "app/app.h"
#include "app/input.h"
#include "app/pausemanager.h"

#include "common/config_file.h"
#include "common/event.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/cloud.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/lightning.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/planet.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/text.h"
#include "graphics/engine/water.h"

#include "graphics/model/model_manager.h"

#include "level/mainmovie.h"
#include "level/player_profile.h"
#include "level/scene_conditions.h"

#include "math/const.h"
#include "math/geometry.h"

#include "object/object.h"
#include "object/object_create_exception.h"
#include "object/object_manager.h"

#include "object/auto/auto.h"

#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motiontoto.h"

#include "object/subclass/exchange_post.h"

#include "object/task/task.h"
#include "object/task/taskbuild.h"
#include "object/task/taskmanip.h"

#include "physics/physics.h"

#include "script/cbottoken.h"
#include "script/script.h"
#include "script/scriptfunc.h"

#include "sound/sound.h"

#include "ui/debug_menu.h"
#include "ui/displayinfo.h"
#include "ui/displaytext.h"
#include "ui/maindialog.h"
#include "ui/mainmap.h"
#include "ui/mainshort.h"
#include "ui/mainui.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/map.h"
#include "ui/controls/shortcut.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

#include "ui/screen/screen_loading.h"

#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <ctime>

#include <boost/lexical_cast.hpp>

const int ALL_LOAD_LEVEL_MODES = LevelLoadMode::Normal | LevelLoadMode::Reset | LevelLoadMode::LoadSavedGame;

std::map<std::string,std::shared_ptr<CLevelLoader::Command>> CLevelLoader::m_commands =
{
        { "Title",              MakeCommand(&CLevelLoader::HandleCommand_Title) },
        { "Resume",             MakeCommand(&CLevelLoader::HandleCommand_Resume) },
        { "ScriptName",         MakeCommand(&CLevelLoader::HandleCommand_ScriptName) },
        { "ScriptFile",         MakeCommand(&CLevelLoader::HandleCommand_ScriptFile) },
        { "Instructions",       MakeCommand(&CLevelLoader::HandleCommand_Instructions) },
        { "Satellite",          MakeCommand(&CLevelLoader::HandleCommand_Satellite) },
        { "Loading",            MakeCommand(&CLevelLoader::HandleCommand_Loading) },
        { "HelpFile",           MakeCommand(&CLevelLoader::HandleCommand_HelpFile) },
        { "SoluceFile",         MakeCommand(&CLevelLoader::HandleCommand_SoluceFile) },
        { "EndingFile",         MakeCommand(&CLevelLoader::HandleCommand_EndingFile) },
        { "MessageDelay",       MakeCommand(&CLevelLoader::HandleCommand_MessageDelay) },
        { "MissionTimer",       MakeCommand(&CLevelLoader::HandleCommand_MissionTimer, ALL_LOAD_LEVEL_MODES) },
        { "TeamName",           MakeCommand(&CLevelLoader::HandleCommand_TeamName, ALL_LOAD_LEVEL_MODES) },
        { "CacheAudio",         MakeCommand(&CLevelLoader::HandleCommand_CacheAudio) },
        { "AudioChange",        MakeCommand(&CLevelLoader::HandleCommand_AudioChange) },
        { "Audio",              MakeCommand(&CLevelLoader::HandleCommand_Audio) },
        { "AmbientColor",       MakeCommand(&CLevelLoader::HandleCommand_AmbientColor) },
        { "FogColor",           MakeCommand(&CLevelLoader::HandleCommand_FogColor) },
        { "VehicleColor",       MakeCommand(&CLevelLoader::HandleCommand_VehicleColor) },
        { "InsectColor",        MakeCommand(&CLevelLoader::HandleCommand_InsectColor) },
        { "GreeneryColor",      MakeCommand(&CLevelLoader::HandleCommand_GreeneryColor) },
        { "DeepView",           MakeCommand(&CLevelLoader::HandleCommand_DeepView) },
        { "FogStart",           MakeCommand(&CLevelLoader::HandleCommand_FogStart) },
        { "SecondTexture",      MakeCommand(&CLevelLoader::HandleCommand_SecondTexture) },
        { "Background",         MakeCommand(&CLevelLoader::HandleCommand_Background) },
        { "Planet",             MakeCommand(&CLevelLoader::HandleCommand_Planet) },
        { "ForegroundName",     MakeCommand(&CLevelLoader::HandleCommand_ForegroundName) },
        { "Level",              MakeCommand(&CLevelLoader::HandleCommand_Level) },
        { "TerrainGenerate",    MakeCommand(&CLevelLoader::HandleCommand_TerrainGenerate) },
        { "TerrainWind",        MakeCommand(&CLevelLoader::HandleCommand_TerrainWind) },
        { "TerrainRelief",      MakeCommand(&CLevelLoader::HandleCommand_TerrainRelief) },
        { "TerrainRandomRelief",MakeCommand(&CLevelLoader::HandleCommand_TerrainRandomRelief) },
        { "TerrainResource",    MakeCommand(&CLevelLoader::HandleCommand_TerrainResource) },
        { "TerrainWater",       MakeCommand(&CLevelLoader::HandleCommand_TerrainWater) },
        { "TerrainLava",        MakeCommand(&CLevelLoader::HandleCommand_TerrainLava) },
        { "TerrainCloud",       MakeCommand(&CLevelLoader::HandleCommand_TerrainCloud) },
        { "TerrainBlitz",       MakeCommand(&CLevelLoader::HandleCommand_TerrainBlitz) },
        { "TerrainInitTextures",MakeCommand(&CLevelLoader::HandleCommand_TerrainInitTextures) },
        { "TerrainInit",        MakeCommand(&CLevelLoader::HandleCommand_TerrainInit) },
        { "TerrainMaterial",    MakeCommand(&CLevelLoader::HandleCommand_TerrainMaterial) },
        { "TerrainLevel",       MakeCommand(&CLevelLoader::HandleCommand_TerrainLevel) },
        { "TerrainCreate",      MakeCommand(&CLevelLoader::HandleCommand_TerrainCreate) },
        { "BeginObject",        MakeCommand(&CLevelLoader::HandleCommand_BeginObject, ALL_LOAD_LEVEL_MODES) },
        { "LevelController",    MakeCommand(&CLevelLoader::HandleCommand_LevelController, LevelLoadMode::Normal | LevelLoadMode::Reset) },
        { "CreateObject",       MakeCommand(&CLevelLoader::HandleCommand_CreateObject, LevelLoadMode::Normal | LevelLoadMode::Reset) },
        { "CreateFog",          MakeCommand(&CLevelLoader::HandleCommand_CreateFog) },
        { "CreateLight",        MakeCommand(&CLevelLoader::HandleCommand_CreateLight) },
        { "CreateSpot",         MakeCommand(&CLevelLoader::HandleCommand_CreateSpot) },
        { "GroundSpot",         MakeCommand(&CLevelLoader::HandleCommand_GroundSpot) },
        { "WaterColor",         MakeCommand(&CLevelLoader::HandleCommand_WaterColor) },
        { "MapColor",           MakeCommand(&CLevelLoader::HandleCommand_MapColor) },
        { "MapZoom",            MakeCommand(&CLevelLoader::HandleCommand_MapZoom) },
        { "MaxFlyingHeight",    MakeCommand(&CLevelLoader::HandleCommand_MaxFlyingHeight) },
        { "AddFlyingHeight",    MakeCommand(&CLevelLoader::HandleCommand_AddFlyingHeight) },
        { "Camera",             MakeCommand(&CLevelLoader::HandleCommand_Camera, ALL_LOAD_LEVEL_MODES) },
        { "EndMissionTake",     MakeCommand(&CLevelLoader::HandleCommand_EndMissionTake) },
        { "EndMissionDelay",    MakeCommand(&CLevelLoader::HandleCommand_EndMissionDelay) },
        { "EndMissionResearch", MakeCommand(&CLevelLoader::HandleCommand_EndMissionResearch) },
        { "ObligatoryToken",    MakeCommand(&CLevelLoader::HandleCommand_ObligatoryToken) },
        { "ProhibitedToken",    MakeCommand(&CLevelLoader::HandleCommand_ProhibitedToken) },
        { "EnableBuild",        MakeCommand(&CLevelLoader::HandleCommand_EnableBuild) },
        { "EnableResearch",     MakeCommand(&CLevelLoader::HandleCommand_EnableResearch) },
        { "DoneResearch",       MakeCommand(&CLevelLoader::HandleCommand_DoneResearch, LevelLoadMode::Normal) },
        { "NewScript",          MakeCommand(&CLevelLoader::HandleCommand_NewScript) }
};

CLevelLoader::CLevelLoader(bool solutionEnabled)
{
    m_main = CRobotMain::GetInstancePointer();
    m_solutionEnabled = solutionEnabled;
    m_nextObjectId = 0;
    m_objectsNumber = -1;
    m_backgroundPath = "";
    m_backgroundUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    m_backgroundDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    m_backgroundCloudUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    m_backgroundCloudDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
    m_backgroundFull = false;
    m_selectedObject = nullptr;
}

CLevelLoader::Command::Command(CommandFunction function, int executionConditions)
{
    this->function = function;
    this->executionConditions = executionConditions;
}

void CLevelLoader::CreateScene(bool solutionEnabled, bool fixScene, LevelLoadMode mode)
{
    std::unique_ptr<CLevelLoader> levelLoader = MakeUnique<CLevelLoader>(solutionEnabled);

    levelLoader->ResetScene(mode);

    levelLoader->BuildScene(fixScene, mode);
}

void CLevelLoader::ResetScene(LevelLoadMode mode)
{
    m_main->m_base = nullptr;

    if (mode != LevelLoadMode::Reset)
    {
        m_main->m_build = 0;
        m_main->m_researchDone.clear();  // no research done
        m_main->m_researchDone[0] = 0;
        m_main->m_researchEnable = 0;

        m_main->ResetUnit();

        m_main->FlushDisplayInfo();
        m_main->m_terrain->FlushMaterials();
        m_main->m_audioTrack = "";
        m_main->m_audioRepeat = true;
        m_main->m_satcomTrack  = "";
        m_main->m_satcomRepeat = true;
        m_main->m_editorTrack  = "";
        m_main->m_editorRepeat = true;
        m_main->m_displayText->SetDelay(1.0f);
        m_main->m_displayText->SetEnable(true);
        m_main->m_immediatSatCom = false;
        m_main->m_lockedSatCom = false;
        m_main->m_endingWin = "";
        m_main->m_endingLost = "";
        m_main->m_audioChange.clear();
        m_main->m_endTake.clear();
        m_main->m_endTakeImmediat = false;
        m_main->m_endTakeResearch = 0;
        m_main->m_endTakeWinDelay = 2.0f;
        m_main->m_endTakeLostDelay = 2.0f;
        m_main->m_globalMagnifyDamage = 1.0f;
        m_main->m_obligatoryTokens.clear();
        m_main->m_mapShow = true;
        m_main->m_mapImage = false;
        m_main->m_mapFilename[0] = 0;

        m_main->m_controller = nullptr;

        m_main->ResetColors();

        m_main->m_engine->SetAmbientColor(Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f), 0);
        m_main->m_engine->SetAmbientColor(Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f), 1);
        m_main->m_engine->SetFogColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), 0);
        m_main->m_engine->SetFogColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f), 1);
        m_main->m_engine->SetDeepView(1000.0f, 0);
        m_main->m_engine->SetDeepView(1000.0f, 1);
        m_main->m_engine->SetFogStart(0.75f, 0);
        m_main->m_engine->SetFogStart(0.75f, 1);
        m_main->m_engine->SetSecondTexture("");
        m_main->m_engine->SetForegroundName("");

        GetResource(RES_TEXT, RT_SCRIPT_NEW, m_main->m_scriptName);
        m_main->m_scriptFile = "";

        m_main->m_missionType   = MISSION_NORMAL;
        m_main->m_codeBattleInit = false;
        m_main->m_codeBattleStarted = false;

        m_main->m_teamNames.clear();

        m_main->m_missionResult = ERR_MISSION_NOTERM;
        m_main->m_missionResultFromScript = false;
    }

    // NOTE: Reset timer always, even when only resetting object positions
    m_main->m_missionTimerEnabled = false;
    m_main->m_missionTimerStarted = false;
    m_main->m_missionTimer = 0.0f;
}

void CLevelLoader::BuildScene(bool fixScene, LevelLoadMode mode)
{
    m_main->m_fixScene = fixScene;

    try
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.05f, RT_LOADING_PROCESSING);
        GetLogger()->Info("Loading level: %s\n", m_main->m_levelFile.c_str());
        CLevelParser levelParser(m_main->m_levelFile);
        levelParser.SetLevelPaths(m_main->m_levelCategory, m_main->m_levelChap, m_main->m_levelRank);
        levelParser.Load();
        m_objectsNumber = levelParser.CountLines("CreateObject");
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.1f, RT_LOADING_LEVEL_SETTINGS);

        for (auto& line : levelParser.GetLines())
        {
            CLevelLoader::Command* command = m_commands[line->GetCommand()].get();

            if (command == nullptr)
            {
                throw CLevelParserException("Unknown command: '" + line->GetCommand() + "' in " + line->GetLevelFilename() + ":" + boost::lexical_cast<std::string>(line->GetLineNumber()));
            }

            if ( !(command->executionConditions & mode) ) continue;

            command->function(this, line.get(), mode);
        }

        m_main->m_ui->GetLoadingScreen()->SetProgress(1.0f, RT_LOADING_FINISHED);
        if (m_main->m_ui->GetLoadingScreen()->IsVisible())
        {
            // Force render of the "Loading finished" screen
            // TODO: For some reason, rendering of the first frame after the simulation starts is very slow
            // We're doing this because it looks weird when the progress bar is finished but it still says "Loading programs"
            m_main->m_app->Render();
        }

        if ( mode != LevelLoadMode::Reset )
        {
            m_main->m_engine->SetBackground(m_backgroundPath,
                                            m_backgroundUp,
                                            m_backgroundDown,
                                            m_backgroundCloudUp,
                                            m_backgroundCloudDown,
                                            m_backgroundFull);

            switch (m_main->m_levelCategory)
            {
            case LevelCategory::Missions:
                m_main->m_playerProfile->SetFreeGameResearchUnlock(m_main->m_playerProfile->GetFreeGameResearchUnlock() | m_main->m_researchDone[0]);
                m_main->m_playerProfile->SetFreeGameBuildUnlock(m_main->m_playerProfile->GetFreeGameBuildUnlock() | m_main->m_build);
                break;
            case LevelCategory::FreeGame:
                m_main->m_researchDone[0] = m_main->m_playerProfile->GetFreeGameResearchUnlock();
                m_main->m_build = m_main->m_playerProfile->GetFreeGameBuildUnlock();
                m_main->m_build &= ~BUILD_RESEARCH;
                m_main->m_build &= ~BUILD_LABO;
                m_main->m_build |= BUILD_FACTORY;
                m_main->m_build |= BUILD_GFLAT;
                m_main->m_build |= BUILD_FLAG;
                break;
            default:
                break;
            }

            m_main->m_short->SetMode(false);  // vehicles?
        }

        m_main->m_map->ShowMap(m_main->m_mapShow);
        m_main->m_map->UpdateMap();
        // TODO: m_main->m_engine->TimeInit(); ??
        m_main->m_input->ResetKeyStates();
        m_main->m_time = 0.0f;
        m_main->m_gameTime = 0.0f;
        m_main->m_gameTimeAbsolute = 0.0f;
        m_main->m_autosaveLast = 0.0f;
        m_main->m_infoUsed = 0;

        m_main->m_selectObject = m_selectedObject;

        if (m_main->m_base == nullptr &&  // no m_main base?
            !m_main->m_fixScene)          // interractive scene?
        {
            CObject* obj = m_selectedObject;
            if (m_selectedObject == nullptr)
                obj = m_main->SearchHuman();

            if (obj != nullptr)
            {
                assert(obj->Implements(ObjectInterfaceType::Controllable));
                m_main->SelectObject(obj);
                m_main->m_camera->SetControllingObject(obj);
                m_main->m_camera->SetType(dynamic_cast<CControllableObject*>(obj)->GetCameraType());
            }
        }

        if (m_main->m_fixScene)
            m_main->m_camera->SetType(Gfx::CAM_TYPE_SCRIPT);

        if (mode == LevelLoadMode::LoadSavedGame && m_selectedObject != nullptr)  // loading file?
        {
            Math::Vector pos = m_selectedObject->GetPosition();
            m_main->m_camera->Init(pos, pos, 0.0f);

            m_main->SelectObject(m_selectedObject);
            m_main->m_camera->SetControllingObject(m_selectedObject);

            m_main->m_beginSatCom = true;  // message already displayed
        }
    }
    catch (...)
    {
        m_main->m_sceneReadPath = "";
        throw;
    }
    m_main->m_sceneReadPath = "";

    if (m_main->m_app->GetSceneTestMode())
        m_main->m_eventQueue->AddEvent(Event(EVENT_QUIT));

    m_main->m_ui->ShowLoadingScreen(false);
    if (m_main->m_missionType == MISSION_CODE_BATTLE)
    {
        m_main->CreateCodeBattleInterface();
    }
    m_main->CreateShortcuts();
}

void CLevelLoader::ShowLoadingWarning(const std::string& message)
{
    GetLogger()->Warn("%s\n", message.c_str());
    CRobotMain::GetInstancePointer()->m_ui->GetDialog()->StartInformation("Level loading warning", "This level contains problems. It may stop working in future versions of the game.", message);
}

void CLevelLoader::HandleCommand_Title(CLevelParserLine* line, LevelLoadMode mode)
{
    // empty
}

void CLevelLoader::HandleCommand_Resume(CLevelParserLine* line, LevelLoadMode mode)
{
    // empty
}

void CLevelLoader::HandleCommand_ScriptName(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_scriptName = line->GetParam("text")->AsString();
}

void CLevelLoader::HandleCommand_ScriptFile(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_scriptFile = line->GetParam("name")->AsString();
}

void CLevelLoader::HandleCommand_Instructions(CLevelParserLine* line, LevelLoadMode mode)
{
    strcpy(m_main->m_infoFilename[SATCOM_HUSTON], line->GetParam("name")->AsPath("help/%lng%").c_str());

    m_main->m_immediatSatCom = line->GetParam("immediat")->AsBool(false);
    m_main->m_beginSatCom = m_main->m_lockedSatCom = line->GetParam("lock")->AsBool(false);
    if (m_main->m_app->GetSceneTestMode()) m_main->m_immediatSatCom = false;
}

void CLevelLoader::HandleCommand_Satellite(CLevelParserLine* line, LevelLoadMode mode)
{
    strcpy(m_main->m_infoFilename[SATCOM_SAT], line->GetParam("name")->AsPath("help/%lng%").c_str());
}

void CLevelLoader::HandleCommand_Loading(CLevelParserLine* line, LevelLoadMode mode)
{
    strcpy(m_main->m_infoFilename[SATCOM_LOADING], line->GetParam("name")->AsPath("help/%lng%").c_str());
}

void CLevelLoader::HandleCommand_HelpFile(CLevelParserLine* line, LevelLoadMode mode)
{
    strcpy(m_main->m_infoFilename[SATCOM_PROG], line->GetParam("name")->AsPath("help/%lng%").c_str());
}

void CLevelLoader::HandleCommand_SoluceFile(CLevelParserLine* line, LevelLoadMode mode)
{
    strcpy(m_main->m_infoFilename[SATCOM_SOLUCE], line->GetParam("name")->AsPath("help/%lng%").c_str());
}

void CLevelLoader::HandleCommand_EndingFile(CLevelParserLine* line, LevelLoadMode mode)
{
    auto Process = [&](const std::string& type) -> std::string
    {
        if (line->GetParam(type)->IsDefined())
        {
            try
            {
                int rank = boost::lexical_cast<int>(line->GetParam(type)->GetValue());
                if (rank >= 0)
                {
                    // TODO: Fix default levels and add a future removal warning
                    GetLogger()->Warn("This level is using deprecated way of defining %1$s scene. Please change the %1$s= parameter in EndingFile from %2$d to \"levels/other/%1$s%2$03d.txt\".\n", type.c_str(), rank);
                    std::stringstream ss;
                    ss << "levels/other/" << type << std::setfill('0') << std::setw(3) << rank << ".txt";
                    return ss.str();
                }
                else
                {
                    // TODO: Fix default levels and add a future removal warning
                    GetLogger()->Warn("This level is using deprecated way of defining %1$s scene. Please remove the %1$s= parameter in EndingFile.\n", type.c_str());
                    return "";
                }

            }
            catch (boost::bad_lexical_cast &e)
            {
                return line->GetParam(type)->AsPath("levels");
            }
        }
        return "";
    };

    m_main->m_endingWin = Process("win");
    m_main->m_endingLost = Process("lost");
}

void CLevelLoader::HandleCommand_MessageDelay(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_displayText->SetDelay(line->GetParam("factor")->AsFloat());
}

void CLevelLoader::HandleCommand_MissionTimer(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_missionTimerEnabled = line->GetParam("enabled")->AsBool();
    if (!line->GetParam("program")->AsBool(false))
    {
        m_main->m_missionTimerStarted = true;
    }
}

void CLevelLoader::HandleCommand_TeamName(CLevelParserLine* line, LevelLoadMode mode)
{
    int team = line->GetParam("team")->AsInt();
    std::string name = line->GetParam("name")->AsString();
    m_main->m_teamNames[team] = name;
}

void CLevelLoader::HandleCommand_CacheAudio(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string filename = line->GetParam("filename")->AsPath("music");
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, filename);
    m_main->m_sound->CacheMusic(filename);
}

void CLevelLoader::HandleCommand_AudioChange(CLevelParserLine* line, LevelLoadMode mode)
{
    auto audioChange = MakeUnique<CAudioChangeCondition>();
    audioChange->Read(line);
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, audioChange->music);
    m_main->m_sound->CacheMusic(audioChange->music);
    m_main->m_audioChange.push_back(std::move(audioChange));

    if (!line->GetParam("pos")->IsDefined() || !line->GetParam("dist")->IsDefined())
    {
        ShowLoadingWarning("The defaults for pos= and dist= are going to change, specify them explicitly. See issue #759 (https://git.io/vVBzH)");
    }
}

void CLevelLoader::HandleCommand_Audio(CLevelParserLine* line, LevelLoadMode mode)
{
    if (line->GetParam("track")->IsDefined())
    {
        if (line->GetParam("filename")->IsDefined())
            throw CLevelParserException("You can't use track and filename at the same time");

        GetLogger()->Warn("Using track= is deprecated. Please replace this with filename=\n");
        int trackid = line->GetParam("track")->AsInt();
        if (trackid != 0)
        {
            std::stringstream filenameStr;
            filenameStr << "music/music" << std::setfill('0') << std::setw(3) << trackid << ".ogg";
            m_main->m_audioTrack = filenameStr.str();
        }
        else
        {
            m_main->m_audioTrack = "";
        }
    }
    else
    {
        if (line->GetParam("filename")->IsDefined())
        {
            m_main->m_audioTrack = line->GetParam("filename")->AsPath("music");
        }
        else
        {
            m_main->m_audioTrack = "";
        }
    }
    if (!m_main->m_audioTrack.empty())
    {
        m_main->m_audioRepeat = line->GetParam("repeat")->AsBool(true);
    }

    if (line->GetParam("satcom")->IsDefined())
    {
        m_main->m_satcomTrack = line->GetParam("satcom")->AsPath("music");
        m_main->m_satcomRepeat = line->GetParam("satcomRepeat")->AsBool(true);
    }
    else
    {
        m_main->m_satcomTrack = "";
    }

    if (line->GetParam("editor")->IsDefined())
    {
        m_main->m_editorTrack = line->GetParam("editor")->AsPath("music");
        m_main->m_editorRepeat = line->GetParam("editorRepeat")->AsBool(true);
    }
    else
    {
        m_main->m_editorTrack = "";
    }

    if (!m_main->m_audioTrack.empty())
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_main->m_audioTrack);
        m_main->m_sound->CacheMusic(m_main->m_audioTrack);
    }
    if (!m_main->m_satcomTrack.empty())
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_main->m_satcomTrack);
        m_main->m_sound->CacheMusic(m_main->m_satcomTrack);
    }
    if (!m_main->m_editorTrack.empty())
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.15f, RT_LOADING_MUSIC, m_main->m_editorTrack);
        m_main->m_sound->CacheMusic(m_main->m_editorTrack);
    }
}

void CLevelLoader::HandleCommand_AmbientColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetAmbientColor(line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
    m_main->m_engine->SetAmbientColor(line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
}

void CLevelLoader::HandleCommand_FogColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetFogColor(line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 0);
    m_main->m_engine->SetFogColor(line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)), 1);
}

void CLevelLoader::HandleCommand_VehicleColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_colorNewBot[line->GetParam("team")->AsInt(0)] = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_InsectColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_colorNewAlien = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_GreeneryColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_colorNewGreen = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_DeepView(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetDeepView(line->GetParam("air")->AsFloat(500.0f)*g_unit, 0, false);
    m_main->m_engine->SetDeepView(line->GetParam("water")->AsFloat(100.0f)*g_unit, 1, false);
}

void CLevelLoader::HandleCommand_FogStart(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetFogStart(line->GetParam("air")->AsFloat(0.5f), 0);
    m_main->m_engine->SetFogStart(line->GetParam("water")->AsFloat(0.5f), 1);
}

void CLevelLoader::HandleCommand_SecondTexture(CLevelParserLine* line, LevelLoadMode mode)
{
    if (line->GetParam("rank")->IsDefined())
    {
        char tex[20] = { 0 };
        sprintf(tex, "dirty%.2d.png", line->GetParam("rank")->AsInt());
        m_main->m_engine->SetSecondTexture(tex);
    }
    else
    {
        m_main->m_engine->SetSecondTexture("../" + line->GetParam("texture")->AsPath("textures"));
    }
}

void CLevelLoader::HandleCommand_Background(CLevelParserLine* line, LevelLoadMode mode)
{
    if (line->GetParam("image")->IsDefined())
        m_backgroundPath = line->GetParam("image")->AsPath("textures");
    m_backgroundUp = line->GetParam("up")->AsColor(m_backgroundUp);
    m_backgroundDown = line->GetParam("down")->AsColor(m_backgroundDown);
    m_backgroundCloudUp = line->GetParam("cloudUp")->AsColor(m_backgroundCloudUp);
    m_backgroundCloudDown = line->GetParam("cloudDown")->AsColor(m_backgroundCloudDown);
    m_backgroundFull = line->GetParam("full")->AsBool(m_backgroundFull);
}

void CLevelLoader::HandleCommand_Planet(CLevelParserLine* line, LevelLoadMode mode)
{
    Math::Vector    ppos, uv1, uv2;

    ppos  = line->GetParam("pos")->AsPoint();
    uv1   = line->GetParam("uv1")->AsPoint();
    uv2   = line->GetParam("uv2")->AsPoint();
    m_main->m_planet->Create(line->GetParam("mode")->AsPlanetType(),
                    Math::Point(ppos.x, ppos.z),
                    line->GetParam("dim")->AsFloat(0.2f),
                    line->GetParam("speed")->AsFloat(0.0f),
                    line->GetParam("dir")->AsFloat(0.0f),
                    line->GetParam("image")->AsPath("textures"),
                    Math::Point(uv1.x, uv1.z),
                    Math::Point(uv2.x, uv2.z),
                    line->GetParam("image")->AsPath("textures").find("planet") != std::string::npos // TODO: add transparent op or modify textures
    );
}

void CLevelLoader::HandleCommand_ForegroundName(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetForegroundName(line->GetParam("image")->AsPath("textures"));
}

void CLevelLoader::HandleCommand_Level(CLevelParserLine* line, LevelLoadMode mode)
{
    g_unit = line->GetParam("unitScale")->AsFloat(4.0f);
    m_main->m_engine->SetTracePrecision(line->GetParam("traceQuality")->AsFloat(1.0f));
    m_main->m_shortCut = line->GetParam("shortcut")->AsBool(true);

    m_main->m_missionType = line->GetParam("type")->AsMissionType(MISSION_NORMAL);
    m_main->m_globalMagnifyDamage = line->GetParam("magnifyDamage")->AsFloat(1.0f);
}

void CLevelLoader::HandleCommand_TerrainGenerate(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f, RT_LOADING_TERRAIN);
    m_main->m_terrain->Generate(line->GetParam("mosaic")->AsInt(20),
                        line->GetParam("brick")->AsInt(3),
                        line->GetParam("size")->AsFloat(20.0f),
                        line->GetParam("vision")->AsFloat(500.0f)*g_unit,
                        line->GetParam("depth")->AsInt(2),
                        line->GetParam("hard")->AsFloat(0.5f));
}

void CLevelLoader::HandleCommand_TerrainWind(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_terrain->SetWind(line->GetParam("speed")->AsPoint());
}

void CLevelLoader::HandleCommand_TerrainRelief(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(1.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RELIEF);
    m_main->m_terrain->LoadRelief(
        line->GetParam("image")->AsPath("textures"),
        line->GetParam("factor")->AsFloat(1.0f),
        line->GetParam("border")->AsBool(true));
}

void CLevelLoader::HandleCommand_TerrainRandomRelief(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(1.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RELIEF);
    m_main->m_terrain->RandomizeRelief();
}

void CLevelLoader::HandleCommand_TerrainResource(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(2.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_RES);
    m_main->m_terrain->LoadResources(line->GetParam("image")->AsPath("textures"));
}

void CLevelLoader::HandleCommand_TerrainWater(CLevelParserLine* line, LevelLoadMode mode)
{
    Math::Vector pos;
    pos.x = line->GetParam("moveX")->AsFloat(0.0f);
    pos.y = line->GetParam("moveY")->AsFloat(0.0f);
    pos.z = pos.x;
    m_main->m_water->Create(line->GetParam("air")->AsWaterType(Gfx::WATER_TT),
                    line->GetParam("water")->AsWaterType(Gfx::WATER_TT),
                    line->GetParam("image")->AsPath("textures"),
                    line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                    line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                    line->GetParam("level")->AsFloat(100.0f)*g_unit,
                    line->GetParam("glint")->AsFloat(1.0f),
                    pos);
    m_main->m_colorNewWater = line->GetParam("color")->AsColor(m_main->m_colorNewWater);
    m_main->m_colorShiftWater = line->GetParam("brightness")->AsFloat(0.0f);
}

void CLevelLoader::HandleCommand_TerrainLava(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_water->SetLava(line->GetParam("mode")->AsBool());
}

void CLevelLoader::HandleCommand_TerrainCloud(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string path = "";
    if (line->GetParam("image")->IsDefined())
        path = line->GetParam("image")->AsPath("textures");
    m_main->m_cloud->Create(path,
                    line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                    line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f)),
                    line->GetParam("level")->AsFloat(500.0f)*g_unit);
}

void CLevelLoader::HandleCommand_TerrainBlitz(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_lightning->Create(line->GetParam("sleep")->AsFloat(0.0f),
                        line->GetParam("delay")->AsFloat(3.0f),
                        line->GetParam("magnetic")->AsFloat(50.0f)*g_unit);
}

void CLevelLoader::HandleCommand_TerrainInitTextures(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(3.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_TEX);
    std::string name = "../" + line->GetParam("image")->AsPath("textures");
    if (name.find(".") == std::string::npos)
        name += ".png";
    unsigned int dx = line->GetParam("dx")->AsInt(1);
    unsigned int dy = line->GetParam("dy")->AsInt(1);

    int tt[100]; //TODO: I have no idea how TerrainInitTextures works, but maybe we shuld remove the limit to 100?
    if (dx*dy > 100)
        throw CLevelParserException("In TerrainInitTextures: dx*dy must be <100");
    if (line->GetParam("table")->IsDefined())
    {
        auto& table = line->GetParam("table")->AsArray();

        if (table.size() > dx*dy)
            throw CLevelParserException("In TerrainInitTextures: table size must be dx*dy");

        for (unsigned int i = 0; i < dx*dy; i++)
        {
            if (i >= table.size())
            {
                tt[i] = 0;
            }
            else
            {
                tt[i] = table[i]->AsInt();
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < dx*dy; i++)
        {
            tt[i] = 0;
        }
    }

    m_main->m_terrain->InitTextures(name.c_str(), tt, dx, dy);
}

void CLevelLoader::HandleCommand_TerrainInit(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_terrain->InitMaterials(line->GetParam("id")->AsInt(1));
}

void CLevelLoader::HandleCommand_TerrainMaterial(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string name = line->GetParam("image")->AsPath("textures");
    if (name.find(".") == std::string::npos)
        name += ".png";
    name = "../" + name;

    m_main->m_terrain->AddMaterial(line->GetParam("id")->AsInt(0),
                        name.c_str(),
                        Math::Point(line->GetParam("u")->AsFloat(),
                                    line->GetParam("v")->AsFloat()),
                        line->GetParam("up")->AsInt(),
                        line->GetParam("right")->AsInt(),
                        line->GetParam("down")->AsInt(),
                        line->GetParam("left")->AsInt(),
                        line->GetParam("hard")->AsFloat(0.5f));
}

void CLevelLoader::HandleCommand_TerrainLevel(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(3.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_TEX);
    int id[50]; //TODO: I have no idea how TerrainLevel works, but maybe we should remove the limit to 50?
    if (line->GetParam("id")->IsDefined())
    {
        auto& idArray = line->GetParam("id")->AsArray();

        if (idArray.size() > 50)
            throw CLevelParserException("In TerrainLevel: id array size must be < 50");

        unsigned int i = 0;
        while (i < 50)
        {
            id[i] = idArray[i]->AsInt();
            i++;
            if (i >= idArray.size()) break;
        }
        id[i] = 0;
    }

    m_main->m_terrain->GenerateMaterials(id,
                                line->GetParam("min")->AsFloat(0.0f)*g_unit,
                                line->GetParam("max")->AsFloat(100.0f)*g_unit,
                                line->GetParam("slope")->AsFloat(5.0f),
                                line->GetParam("freq")->AsFloat(100.0f),
                                line->GetParam("center")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                                line->GetParam("radius")->AsFloat(0.0f)*g_unit);
}

void CLevelLoader::HandleCommand_TerrainCreate(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.2f+(4.f/5.f)*0.05f, RT_LOADING_TERRAIN, RT_LOADING_TERRAIN_GEN);
    m_main->m_terrain->CreateObjects();
}

void CLevelLoader::HandleCommand_BeginObject(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->InitEye();
    m_main->SetMovieLock(false);

    if (mode != LevelLoadMode::Reset)
        m_main->ChangeColor();  // changes the colors of texture

    if (!m_main->m_sceneReadPath.empty())  // loading file ?
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.25f, RT_LOADING_OBJECTS_SAVED);
        m_selectedObject = m_main->IOReadScene(m_main->m_sceneReadPath + "/data.sav", m_main->m_sceneReadPath + "/cbot.run");
    }
    else
    {
        m_main->m_ui->GetLoadingScreen()->SetProgress(0.25f, RT_LOADING_OBJECTS);
    }
}

void CLevelLoader::HandleCommand_LevelController(CLevelParserLine* line, LevelLoadMode mode)
{
    if (m_main->m_controller != nullptr)
    {
        throw CLevelParserException("There can be only one LevelController in the level");
    }

    m_main->m_controller = m_main->m_objMan->CreateObject(Math::Vector(0.0f, 0.0f, 0.0f), 0.0f, OBJECT_CONTROLLER);
    assert(m_main->m_controller->Implements(ObjectInterfaceType::Programmable));
    assert(m_main->m_controller->Implements(ObjectInterfaceType::ProgramStorage));

    if (line->GetParam("script")->IsDefined())
    {
        CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(m_main->m_controller);
        Program* program = programStorage->AddProgram();
        programStorage->ReadProgram(program, line->GetParam("script")->AsPath("ai"));
        program->readOnly = true;
        dynamic_cast<CProgrammableObject*>(m_main->m_controller)->RunProgram(program);
    }
}

void CLevelLoader::HandleCommand_CreateObject(CLevelParserLine* line, LevelLoadMode mode)
{
    ObjectCreateParams params = CObject::ReadCreateParams(line);

    float objectProgress = static_cast<float>(m_nextObjectId) / static_cast<float>(m_objectsNumber);
    std::string details = StrUtils::ToString<int>(m_nextObjectId+1)+" / "+StrUtils::ToString<int>(m_objectsNumber);
    #if DEV_BUILD
    // Object categories may spoil the level a bit, so hide them in release builds
    details += ": "+CLevelParserParam::FromObjectType(params.type);
    #endif
    m_main->m_ui->GetLoadingScreen()->SetProgress(0.25f+objectProgress*0.75f, RT_LOADING_OBJECTS, details);

    try
    {
        CObject* obj = m_main->m_objMan->CreateObject(params);
        obj->Read(line);

        if (m_main->m_fixScene && obj->GetType() == OBJECT_HUMAN)
        {
            assert(obj->Implements(ObjectInterfaceType::Movable));
            CMotion* motion = dynamic_cast<CMovableObject*>(obj)->GetMotion();
            if (m_main->m_phase == PHASE_WIN ) motion->SetAction(MHS_WIN,  0.4f);
            if (m_main->m_phase == PHASE_LOST) motion->SetAction(MHS_LOST, 0.5f);
        }

        if (obj->Implements(ObjectInterfaceType::Controllable) && line->GetParam("select")->AsBool(false))
            m_selectedObject = obj;

        if (obj->GetType() == OBJECT_BASE)
            m_main->m_base = obj;

        if (obj->Implements(ObjectInterfaceType::ProgramStorage))
        {
            CProgramStorageObject* programStorage = dynamic_cast<CProgramStorageObject*>(obj);

            if (obj->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject*>(obj)->GetSelectable() && obj->GetType() != OBJECT_HUMAN)
            {
                programStorage->SetProgramStorageIndex(m_nextObjectId);
            }

            char categoryChar = GetLevelCategoryDir(m_main->m_levelCategory)[0];
            programStorage->LoadAllProgramsForLevel(
                line,
                m_main->m_playerProfile->GetSaveFile(StrUtils::Format("%c%.3d%.3d", categoryChar, m_main->m_levelChap, m_main->m_levelRank)),
                m_solutionEnabled
            );
        }
    }
    catch (const CObjectCreateException& e)
    {
        GetLogger()->Error("Error loading level object: %s\n", e.what());
        throw;
    }

    m_nextObjectId++;
}

void CLevelLoader::HandleCommand_CreateFog(CLevelParserLine* line, LevelLoadMode mode)
{
    Gfx::ParticleType type = static_cast<Gfx::ParticleType>(Gfx::PARTIFOG0+(line->GetParam("type")->AsInt()));
    Math::Vector pos = line->GetParam("pos")->AsPoint()*g_unit;
    float height = line->GetParam("height")->AsFloat(1.0f)*g_unit;
    float ddim = line->GetParam("dim")->AsFloat(50.0f)*g_unit;
    float delay = line->GetParam("delay")->AsFloat(2.0f);
    m_main->m_terrain->AdjustToFloor(pos);
    pos.y += height;
    Math::Point dim;
    dim.x = ddim;
    dim.y = dim.x;
    m_main->m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, type, delay, 0.0f, 0.0f);
}

void CLevelLoader::HandleCommand_CreateLight(CLevelParserLine* line, LevelLoadMode mode)
{
    Gfx::EngineObjectType  type;

    int lightRank = m_main->CreateLight(line->GetParam("dir")->AsPoint(),
                                line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

    type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);

    if (type == Gfx::ENG_OBJTYPE_TERRAIN)
    {
        m_main->m_lightMan->SetLightPriority(lightRank, Gfx::LIGHT_PRI_HIGHEST);
        m_main->m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);
    }

    if (type == Gfx::ENG_OBJTYPE_QUARTZ)
        m_main->m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_QUARTZ);

    if (type == Gfx::ENG_OBJTYPE_METAL)
        m_main->m_lightMan->SetLightIncludeType(lightRank, Gfx::ENG_OBJTYPE_METAL);

    if (type == Gfx::ENG_OBJTYPE_FIX)
        m_main->m_lightMan->SetLightExcludeType(lightRank, Gfx::ENG_OBJTYPE_TERRAIN);
}

void CLevelLoader::HandleCommand_CreateSpot(CLevelParserLine* line, LevelLoadMode mode)
{
    Gfx::EngineObjectType  type;

    int rankLight = m_main->CreateSpot(line->GetParam("pos")->AsPoint()*g_unit,
                            line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f)));

    type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);
    if (type == Gfx::ENG_OBJTYPE_TERRAIN)
        m_main->m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);

    if (type == Gfx::ENG_OBJTYPE_QUARTZ)
        m_main->m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_QUARTZ);

    if (type == Gfx::ENG_OBJTYPE_METAL)
        m_main->m_lightMan->SetLightIncludeType(rankLight, Gfx::ENG_OBJTYPE_METAL);

    if (type == Gfx::ENG_OBJTYPE_FIX)
        m_main->m_lightMan->SetLightExcludeType(rankLight, Gfx::ENG_OBJTYPE_TERRAIN);
}

void CLevelLoader::HandleCommand_GroundSpot(CLevelParserLine* line, LevelLoadMode mode)
{
    int rank = m_main->m_engine->CreateGroundSpot();
    if (rank != -1)
    {
        m_main->m_engine->SetObjectGroundSpotPos(rank, line->GetParam("pos")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit);
        m_main->m_engine->SetObjectGroundSpotRadius(rank, line->GetParam("radius")->AsFloat(10.0f)*g_unit);
        m_main->m_engine->SetObjectGroundSpotColor(rank, line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
        m_main->m_engine->SetObjectGroundSpotSmooth(rank, line->GetParam("smooth")->AsFloat(1.0f));
        m_main->m_engine->SetObjectGroundSpotMinMax(rank, line->GetParam("min")->AsFloat(0.0f)*g_unit,
                                            line->GetParam("max")->AsFloat(0.0f)*g_unit);
    }
}

void CLevelLoader::HandleCommand_WaterColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_engine->SetWaterAddColor(line->GetParam("color")->AsColor());
}

void CLevelLoader::HandleCommand_MapColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_map->FloorColorMap(line->GetParam("floor")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)),
                        line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f)));
    m_main->m_mapShow = line->GetParam("show")->AsBool(true);
    m_main->m_map->SetToy(line->GetParam("toyIcon")->AsBool(false));
    m_main->m_mapImage = line->GetParam("image")->AsBool(false);
    if (m_main->m_mapImage)
    {
        Math::Vector offset;
        strcpy(m_main->m_mapFilename, line->GetParam("filename")->AsPath("textures").c_str());
        offset = line->GetParam("offset")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f));
        m_main->m_map->SetFixParam(line->GetParam("zoom")->AsFloat(1.0f),
                        offset.x, offset.z,
                        line->GetParam("angle")->AsFloat(0.0f)*Math::PI/180.0f,
                        line->GetParam("mode")->AsInt(0),
                        line->GetParam("debug")->AsBool(false));
    }
}

void CLevelLoader::HandleCommand_MapZoom(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_map->ZoomMap(line->GetParam("factor")->AsFloat(2.0f));
    m_main->m_map->MapEnable(line->GetParam("enable")->AsBool(true));
}

void CLevelLoader::HandleCommand_MaxFlyingHeight(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_terrain->SetFlyingMaxHeight(line->GetParam("max")->AsFloat(280.0f)*g_unit);
}

void CLevelLoader::HandleCommand_AddFlyingHeight(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_terrain->AddFlyingLimit(line->GetParam("center")->AsPoint()*g_unit,
                            line->GetParam("extRadius")->AsFloat(20.0f)*g_unit,
                            line->GetParam("intRadius")->AsFloat(10.0f)*g_unit,
                            line->GetParam("maxHeight")->AsFloat(200.0f));
}

void CLevelLoader::HandleCommand_Camera(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_camera->Init(line->GetParam("eye")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                line->GetParam("lookat")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit,
                mode == LevelLoadMode::Reset ? 0.0f : line->GetParam("delay")->AsFloat(0.0f));

    if (line->GetParam("fadeIn")->AsBool(false))
        m_main->m_camera->StartOver(Gfx::CAM_OVER_EFFECT_FADEIN_WHITE, Math::Vector(0.0f, 0.0f, 0.0f), 1.0f);
}

void CLevelLoader::HandleCommand_EndMissionTake(CLevelParserLine* line, LevelLoadMode mode)
{
    auto endTake = MakeUnique<CSceneEndCondition>();
    endTake->Read(line);
    if (endTake->immediat)
        m_main->m_endTakeImmediat = true;
    m_main->m_endTake.push_back(std::move(endTake));

    if (!line->GetParam("pos")->IsDefined() || !line->GetParam("dist")->IsDefined())
    {
        ShowLoadingWarning("The defaults for pos= and dist= are going to change, specify them explicitly. See issue #759 (https://git.io/vVBzH)");
    }
}

void CLevelLoader::HandleCommand_EndMissionDelay(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_endTakeWinDelay  = line->GetParam("win")->AsFloat(2.0f);
    m_main->m_endTakeLostDelay = line->GetParam("lost")->AsFloat(2.0f);
}

void CLevelLoader::HandleCommand_EndMissionResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_endTakeResearch |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_ObligatoryToken(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string token = line->GetParam("text")->AsString();
    if (!line->GetParam("min")->IsDefined() && !line->GetParam("max")->IsDefined())
        GetLogger()->Warn("ObligatoryToken without specifying min/max is provided only for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" min=1\n", token.c_str());
    if (m_main->m_obligatoryTokens.count(token))
        throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

    m_main->m_obligatoryTokens[token].min = line->GetParam("min")->AsInt(line->GetParam("max")->IsDefined() ? -1 : 1); // BACKWARDS COMPATIBILITY: if neither min or max are defined, default to min=1
    m_main->m_obligatoryTokens[token].max = line->GetParam("max")->AsInt(-1);
    if (m_main->m_obligatoryTokens[token].min >= 0 && m_main->m_obligatoryTokens[token].max >= 0 && m_main->m_obligatoryTokens[token].min > m_main->m_obligatoryTokens[token].max)
    {
        throw CLevelParserException("Incorrect ObligatoryToken specification - min cannot be greater than max");
    }
}

void CLevelLoader::HandleCommand_ProhibitedToken(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string token = line->GetParam("text")->AsString();
    GetLogger()->Warn("ProhibitedToken is only provided for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" max=0\n", token.c_str());
    if (m_main->m_obligatoryTokens.count(token))
        throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

    m_main->m_obligatoryTokens[token].min = -1;
    m_main->m_obligatoryTokens[token].max = 0;
}

void CLevelLoader::HandleCommand_EnableBuild(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_build |= line->GetParam("type")->AsBuildFlag();
}

void CLevelLoader::HandleCommand_EnableResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_researchEnable |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_DoneResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_researchDone[0] |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_NewScript(CLevelParserLine* line, LevelLoadMode mode)
{
    m_main->m_newScriptName.push_back(NewScriptName(line->GetParam("type")->AsObjectType(OBJECT_NULL),
                                                    const_cast<char*>(line->GetParam("name")->AsPath("ai").c_str())));
}

std::shared_ptr<CLevelLoader::Command> CLevelLoader::MakeCommand(CommandFunction function, int executionConditions)
{
    return std::make_shared<CLevelLoader::Command>(function, executionConditions);
}
