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

#include "level/level_loader.h"
#include "level/mainmovie.h"
#include "level/player_profile.h"
#include "level/scene_conditions.h"

#include "level/parser/parser.h"

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

CLevelLoader::Result::Result()
{
    unit = CRobotMain::UNIT;

    GetResource(RES_TEXT, RT_SCRIPT_NEW, scriptName);

    newBotColors = { {0, CRobotMain::COLOR_REF_BOT} };
    newAlienColor = CRobotMain::COLOR_REF_ALIEN;
    newGreenColor = CRobotMain::COLOR_REF_GREEN;

    infoFilenames.resize(SatCom::MAX, "");
    infoFilenames[SatCom::Object] = "objects.txt";
}

CLevelLoader::CLevelLoader(WarningCallback warningCllback)
{
    m_warningCallback =  warningCllback;
}

CLevelLoader::Result CLevelLoader::LoadScene(CLevelReadOnlyParser* sceneParser, LevelLoadMode mode, WarningCallback warningCallback,
                                    CLevelReadOnlyParser* savedSceneParser)
{
    Validate(sceneParser);

    auto levelLoader = std::unique_ptr<CLevelLoader>(new CLevelLoader(warningCallback));

    for (auto& line : sceneParser->GetLines())
    {
        CLevelLoader::Command* command = m_commands[line->GetCommand()].get();

        if (command == nullptr)
        {
            throw CLevelParserException("Unknown command: '" + line->GetCommand() + "' in " + line->GetLevelFilename() + ":" + boost::lexical_cast<std::string>(line->GetLineNumber()));
        }

        if ( !(command->executionConditions & mode) ) continue;

        command->function(levelLoader.get(), line.get(), mode);
    }

    if (mode == LevelLoadMode::LoadSavedGame)
    {
        levelLoader->ReadScene(savedSceneParser);
    }

    return std::move(levelLoader->m_result);
}

void CLevelLoader::SetExecutionStack(const CObjectContainerProxy& objects, const std::string& filecbot)
{
    FILE* file = CBot::fOpen((CResourceManager::GetSaveLocation() + "/" + filecbot).c_str(), "rb");
    if (file != nullptr)
    {
        long version;
        CBot::fRead(&version, sizeof(long), 1, file);  // version of COLOBOT
        if (version == 1)
        {
            CBot::fRead(&version, sizeof(long), 1, file);  // version of CBOT
            if (version == CBot::CBotProgram::GetVersion())
            {
                for (CObject* obj : objects)
                {
                    if (obj->GetType() == OBJECT_TOTO) continue;
                    if (IsObjectBeingTransported(obj)) continue;
                    if (obj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject*>(obj)->IsDying()) continue;

                    if (!ReadFileStack(obj, file)) break;
                }
            }
        }
        CBot::CBotClass::RestoreStaticState(file);
        CBot::fClose(file);
    }
}

void CLevelLoader::Validate(CLevelReadOnlyParser* parser)
{
    if (parser->CountLines("LevelController") > 1)
    {
        throw CLevelParserException("There can be only one LevelController in the level");
    }
}

void CLevelLoader::ShowLoadingWarning(const std::string& message)
{
    m_warningCallback(message);
}

CLevelLoader::ObjectCreate CLevelLoader::ReadObject(CLevelParserLine *line)
{
    ObjectCreateParams params = CObject::ReadCreateParams(line);
    params.id = line->GetParam("id")->AsInt();
    params.power = -1.0f;

    return ObjectCreate(params, line);
}

void CLevelLoader::ReadScene(CLevelReadOnlyParser* parser)
{
    boost::optional<ObjectCreate> cargo = boost::optional<ObjectCreate>();
    boost::optional<ObjectCreate> power = boost::optional<ObjectCreate>();
    for (auto& line : parser->GetLines())
    {
        if (line->GetCommand() == "Map")
            m_result.map.defaultZoom = line->GetParam("zoom")->AsFloat();

        if (line->GetCommand() == "DoneResearch")
            m_result.researchDone[0] = line->GetParam("bits")->AsInt();

        if (line->GetCommand() == "BlitzMode")
        {
            m_result.lightning.sleep = line->GetParam("sleep")->AsFloat();
            m_result.lightning.delay = line->GetParam("delay")->AsFloat();
            m_result.lightning.magnetic = line->GetParam("magnetic")->AsFloat()*m_result.unit;
            m_result.lightning.progress = line->GetParam("progress")->AsFloat();
        }

        if (line->GetCommand() == "CreateFret")
        {
            cargo = ReadObject(line.get());
        }

        if (line->GetCommand() == "CreatePower")
        {
            power = ReadObject(line.get());
        }

        if (line->GetCommand() == "CreateObject")
        {
            ObjectCreate obj = ReadObject(line.get());
            LoadedObjectCreate loadedObject = LoadedObjectCreate(obj, cargo, power);

            m_result.loadedObjects.push_back(loadedObject);

            cargo.reset();
            power.reset();
        }
    }
}

bool CLevelLoader::ReadFileStack(CObject *obj, FILE *file)
{
    if (! obj->Implements(ObjectInterfaceType::Programmable)) return true;

    CProgrammableObject* programmable = dynamic_cast<CProgrammableObject*>(obj);

    ObjectType type = obj->GetType();
    if (type == OBJECT_HUMAN) return true;

    return programmable->ReadStack(file);
}

std::shared_ptr<CLevelLoader::Command> CLevelLoader::MakeCommand(CommandFunction function, int executionConditions)
{
    return std::make_shared<CLevelLoader::Command>(function, executionConditions);
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
    m_result.scriptName = line->GetParam("text")->AsString();
}

void CLevelLoader::HandleCommand_ScriptFile(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.scriptFile = line->GetParam("name")->AsString();
}

void CLevelLoader::HandleCommand_Instructions(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.infoFilenames[SatCom::Huston] = line->GetParam("name")->AsPath("help/%lng%");
    m_result.immediatSatcom = line->GetParam("immediat")->AsBool(false);
    m_result.beginSatCom = line->GetParam("lock")->AsBool(false);
}

void CLevelLoader::HandleCommand_Satellite(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.infoFilenames[SatCom::Sat] = line->GetParam("name")->AsPath("help/%lng%");
}

void CLevelLoader::HandleCommand_Loading(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.infoFilenames[SatCom::Loading] = line->GetParam("name")->AsPath("help/%lng%");
}

void CLevelLoader::HandleCommand_HelpFile(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.infoFilenames[SatCom::Prog] = line->GetParam("name")->AsPath("help/%lng%");
}

void CLevelLoader::HandleCommand_SoluceFile(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.infoFilenames[SatCom::Solution] = line->GetParam("name")->AsPath("help/%lng%");
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

    m_result.mission.endingWin = Process("win");
    m_result.mission.endingLost = Process("lost");
}

void CLevelLoader::HandleCommand_MessageDelay(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.messageDelay = line->GetParam("factor")->AsFloat();
}

void CLevelLoader::HandleCommand_MissionTimer(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.mission.missionTimerEnabled = line->GetParam("enabled")->AsBool();
    m_result.mission.missionTimerStarted = !line->GetParam("program")->AsBool(false);
}

void CLevelLoader::HandleCommand_TeamName(CLevelParserLine* line, LevelLoadMode mode)
{
    int team = line->GetParam("team")->AsInt();
    std::string name = line->GetParam("name")->AsString();
    m_result.teamNames[team] = name;
}

void CLevelLoader::HandleCommand_CacheAudio(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string filename = line->GetParam("filename")->AsPath("music");
    m_result.audio.cacheAudioFileNames.push_back(filename);
}

void CLevelLoader::HandleCommand_AudioChange(CLevelParserLine* line, LevelLoadMode mode)
{
    auto audioChange = MakeUnique<CAudioChangeCondition>();
    audioChange->Read(line);
    m_result.audio.cacheAudioFileNames.push_back(audioChange->music);
    m_result.audio.audioChangeConditions.push_back(std::move(audioChange));

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
            m_result.audio.audioTrack = filenameStr.str();
        }
        else
        {
            m_result.audio.audioTrack = "";
        }
    }
    else
    {
        if (line->GetParam("filename")->IsDefined())
        {
            m_result.audio.audioTrack = line->GetParam("filename")->AsPath("music");
        }
        else
        {
            m_result.audio.audioTrack = "";
        }
    }
    if (!m_result.audio.audioTrack.empty())
    {
        m_result.audio.audioRepeat = line->GetParam("repeat")->AsBool(true);
    }

    if (line->GetParam("satcom")->IsDefined())
    {
        m_result.audio.satcomAudioTrack = line->GetParam("satcom")->AsPath("music");
        m_result.audio.satcomAudioRepeat = line->GetParam("satcomRepeat")->AsBool(true);
    }
    else
    {
        m_result.audio.satcomAudioTrack = "";
    }

    if (line->GetParam("editor")->IsDefined())
    {
        m_result.audio.editorAudioTrack = line->GetParam("editor")->AsPath("music");
        m_result.audio.editorAudioRepeat = line->GetParam("editorRepeat")->AsBool(true);
    }
    else
    {
        m_result.audio.editorAudioTrack = "";
    }

    if (!m_result.audio.audioTrack.empty())
    {
        m_result.audio.cacheAudioFileNames.push_back(m_result.audio.audioTrack);
    }
    if (!m_result.audio.satcomAudioTrack.empty())
    {
        m_result.audio.cacheAudioFileNames.push_back(m_result.audio.satcomAudioTrack);
    }
    if (!m_result.audio.editorAudioTrack.empty())
    {
        m_result.audio.cacheAudioFileNames.push_back(m_result.audio.editorAudioTrack);
    }
}

void CLevelLoader::HandleCommand_AmbientColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.airAmbientColor = line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
    m_result.waterAmbientColor = line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_FogColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.airFogColor = line->GetParam("air")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
    m_result.waterFogColor = line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_VehicleColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.newBotColors[line->GetParam("team")->AsInt(0)] = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_InsectColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.newAlienColor = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_GreeneryColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.newGreenColor = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
}

void CLevelLoader::HandleCommand_DeepView(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.airDeepView = line->GetParam("air")->AsFloat(500.0f)*m_result.unit;
    m_result.waterDeepView = line->GetParam("water")->AsFloat(100.0f)*m_result.unit;
}

void CLevelLoader::HandleCommand_FogStart(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.airFogStart = line->GetParam("air")->AsFloat(0.5f);
    m_result.waterFogStart = line->GetParam("water")->AsFloat(0.5f);
}

void CLevelLoader::HandleCommand_SecondTexture(CLevelParserLine* line, LevelLoadMode mode)
{
    if (line->GetParam("rank")->IsDefined())
    {
        char tex[20] = { 0 };
        sprintf(tex, "dirty%.2d.png", line->GetParam("rank")->AsInt());
        m_result.secondTexture = tex;
    }
    else
    {
        m_result.secondTexture = "../" + line->GetParam("texture")->AsPath("textures");
    }
}

void CLevelLoader::HandleCommand_Background(CLevelParserLine* line, LevelLoadMode mode)
{
    if (line->GetParam("image")->IsDefined())
        m_result.backgroundPath = line->GetParam("image")->AsPath("textures");
    m_result.backgroundUp = line->GetParam("up")->AsColor(m_result.backgroundUp);
    m_result.backgroundDown = line->GetParam("down")->AsColor(m_result.backgroundDown);
    m_result.backgroundCloudUp = line->GetParam("cloudUp")->AsColor(m_result.backgroundCloudUp);
    m_result.backgroundCloudDown = line->GetParam("cloudDown")->AsColor(m_result.backgroundCloudDown);
    m_result.backgroundFull = line->GetParam("full")->AsBool(m_result.backgroundFull);
}

void CLevelLoader::HandleCommand_Planet(CLevelParserLine* line, LevelLoadMode mode)
{
    Math::Vector    ppos, uv1, uv2;

    ppos  = line->GetParam("pos")->AsPoint();
    uv1   = line->GetParam("uv1")->AsPoint();
    uv2   = line->GetParam("uv2")->AsPoint();

    Gfx::CPlanet::Planet planet;

    planet.type = line->GetParam("mode")->AsPlanetType();
    planet.start = Math::Point(ppos.x, ppos.z);
    planet.angle = Math::Point(ppos.x, ppos.z);
    planet.dim   = line->GetParam("dim")->AsFloat(0.2f);
    planet.speed = line->GetParam("speed")->AsFloat(0.0f);
    planet.dir   = line->GetParam("dir")->AsFloat(0.0f);
    planet.name = line->GetParam("image")->AsPath("textures");
    planet.uv1   = Math::Point(uv1.x, uv1.z);
    planet.uv2   = Math::Point(uv2.x, uv2.z);
    planet.transparent = line->GetParam("image")->AsPath("textures").find("planet") != std::string::npos; // TODO: add transparent op or modify textures

    m_result.planets.push_back(planet);
}

void CLevelLoader::HandleCommand_ForegroundName(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.foregroundName = line->GetParam("image")->AsPath("textures");
}

void CLevelLoader::HandleCommand_Level(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.unit = line->GetParam("unitScale")->AsFloat(CRobotMain::UNIT);
    m_result.traceQuality = line->GetParam("traceQuality")->AsFloat(1.0f);
    m_result.shortcut = line->GetParam("shortcut")->AsBool(true);

    m_result.mission.missionType = line->GetParam("type")->AsMissionType(MISSION_NORMAL);
    m_result.magnifyDamage = line->GetParam("magnifyDamage")->AsFloat(1.0f);
}

void CLevelLoader::HandleCommand_TerrainGenerate(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.generate.mosaicCount = line->GetParam("mosaic")->AsInt(20);
    m_result.terrain.generate.brickCountPow2 =  line->GetParam("brick")->AsInt(3),
    m_result.terrain.generate.brickSize = line->GetParam("size")->AsFloat(20.0f),
    m_result.terrain.generate.vision = line->GetParam("vision")->AsFloat(500.0f)*m_result.unit,
    m_result.terrain.generate.depth = line->GetParam("depth")->AsInt(2),
    m_result.terrain.generate.hardness = line->GetParam("hard")->AsFloat(0.5f);
}

void CLevelLoader::HandleCommand_TerrainWind(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.windSpeed = line->GetParam("speed")->AsPoint();
}

void CLevelLoader::HandleCommand_TerrainRelief(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.relief = TerrainRelief(
        line->GetParam("image")->AsPath("textures"),
        line->GetParam("factor")->AsFloat(1.0f),
        line->GetParam("border")->AsBool(true));
}

void CLevelLoader::HandleCommand_TerrainRandomRelief(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.relief = TerrainRelief();
}

void CLevelLoader::HandleCommand_TerrainResource(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.resources = line->GetParam("image")->AsPath("textures");
}

void CLevelLoader::HandleCommand_TerrainWater(CLevelParserLine* line, LevelLoadMode mode)
{
    Math::Vector pos;
    pos.x = line->GetParam("moveX")->AsFloat(0.0f);
    pos.y = line->GetParam("moveY")->AsFloat(0.0f);
    pos.z = pos.x;
    m_result.water.exists = true;
    m_result.water.type1 = line->GetParam("air")->AsWaterType(Gfx::WATER_TT);
    m_result.water.type2 = line->GetParam("water")->AsWaterType(Gfx::WATER_TT);
    m_result.water.imageFileName = line->GetParam("image")->AsPath("textures");
    m_result.water.diffuse = line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_result.water.ambient = line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_result.water.level = line->GetParam("level")->AsFloat(100.0f)*m_result.unit;
    m_result.water.glint = line->GetParam("glint")->AsFloat(1.0f);
    m_result.water.eddy = pos;
    m_result.water.color = line->GetParam("color")->AsColor(m_result.water.color);
    m_result.water.colorShift = line->GetParam("brightness")->AsFloat(0.0f);
}

void CLevelLoader::HandleCommand_TerrainLava(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.water.lava = line->GetParam("mode")->AsBool();
}

void CLevelLoader::HandleCommand_TerrainCloud(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string path = "";
    if (line->GetParam("image")->IsDefined())
        path = line->GetParam("image")->AsPath("textures");
    m_result.cloud.exists = true;
    m_result.cloud.fileName = path;
    m_result.cloud.diffuse = line->GetParam("diffuse")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_result.cloud.ambient = line->GetParam("ambient")->AsColor(Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_result.cloud.level = line->GetParam("level")->AsFloat(500.0f)*m_result.unit;
}

void CLevelLoader::HandleCommand_TerrainBlitz(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.lightning.exists = true;
    m_result.lightning.sleep = line->GetParam("sleep")->AsFloat(0.0f);
    m_result.lightning.delay = line->GetParam("delay")->AsFloat(3.0f);
    m_result.lightning.magnetic = line->GetParam("magnetic")->AsFloat(50.0f)*m_result.unit;
}

void CLevelLoader::HandleCommand_TerrainInitTextures(CLevelParserLine* line, LevelLoadMode mode)
{
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

    m_result.terrain.simpleTextureEnabled = true;
    m_result.terrain.simpleTexture.fileName = name;
    m_result.terrain.simpleTexture.table = std::vector<int>(tt, tt + sizeof(tt) / sizeof(tt[0]));
    m_result.terrain.simpleTexture.width = dx;
    m_result.terrain.simpleTexture.height = dy;
}

void CLevelLoader::HandleCommand_TerrainInit(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.simpleTextureEnabled = false;
    m_result.terrain.complexTexture.defaultMaterial = line->GetParam("id")->AsInt(1);
}

void CLevelLoader::HandleCommand_TerrainMaterial(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string name = line->GetParam("image")->AsPath("textures");
    if (name.find(".") == std::string::npos)
        name += ".png";
    name = "../" + name;

    Gfx::CTerrain::TerrainMaterial tm;
    tm.texName  = name.c_str();
    tm.id       = line->GetParam("id")->AsInt(0);
    tm.uv       = Math::Point(line->GetParam("u")->AsFloat(),
                             line->GetParam("v")->AsFloat());
    tm.mat[0]   = line->GetParam("up")->AsInt();
    tm.mat[1]   = line->GetParam("right")->AsInt();
    tm.mat[2]   = line->GetParam("down")->AsInt();
    tm.mat[3]   = line->GetParam("left")->AsInt();
    tm.hardness = line->GetParam("hard")->AsFloat(0.5f);

    m_result.terrain.complexTexture.materials.push_back(tm);
}

void CLevelLoader::HandleCommand_TerrainLevel(CLevelParserLine* line, LevelLoadMode mode)
{
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

    CLevelLoader::TerrainLevel terrainLevel;
    terrainLevel.id = std::vector<int>(id, id + sizeof(id) / sizeof(id[0]));
    terrainLevel.min = line->GetParam("min")->AsFloat(0.0f)*m_result.unit;
    terrainLevel.max = line->GetParam("max")->AsFloat(100.0f)*m_result.unit;
    terrainLevel.slope = line->GetParam("slope")->AsFloat(5.0f);
    terrainLevel.freq = line->GetParam("freq")->AsFloat(100.0f);
    terrainLevel.center = line->GetParam("center")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*m_result.unit;
    terrainLevel.radius = line->GetParam("radius")->AsFloat(0.0f)*m_result.unit;

    m_result.terrain.complexTexture.levels.push_back(terrainLevel);
}

void CLevelLoader::HandleCommand_TerrainCreate(CLevelParserLine* line, LevelLoadMode mode)
{
    // empty
}

void CLevelLoader::HandleCommand_BeginObject(CLevelParserLine* line, LevelLoadMode mode)
{
    // empty
}

void CLevelLoader::HandleCommand_LevelController(CLevelParserLine* line, LevelLoadMode mode)
{
    ObjectCreateParams params;
    params.pos = Math::Vector(0.0f, 0.0f, 0.0f);
    params.angle = 0.0f;
    params.type = OBJECT_CONTROLLER;

    m_result.objects.push_back(ObjectCreate(params, line));
}

void CLevelLoader::HandleCommand_CreateObject(CLevelParserLine* line, LevelLoadMode mode)
{
    ObjectCreateParams params = CObject::ReadCreateParams(line);

    m_result.objects.push_back(ObjectCreate(params, line));
}

void CLevelLoader::HandleCommand_CreateFog(CLevelParserLine* line, LevelLoadMode mode)
{
    Fog fog;
    fog.type = static_cast<Gfx::ParticleType>(Gfx::PARTIFOG0+(line->GetParam("type")->AsInt()));
    fog.position = line->GetParam("pos")->AsPoint()*m_result.unit;
    fog.height = line->GetParam("height")->AsFloat(1.0f)*m_result.unit;
    float ddim = line->GetParam("dim")->AsFloat(50.0f)*m_result.unit;
    fog.dim.x = ddim;
    fog.dim.y = ddim;
    fog.delay = line->GetParam("delay")->AsFloat(2.0f);

    m_result.fog.push_back(fog);
}

void CLevelLoader::HandleCommand_CreateLight(CLevelParserLine* line, LevelLoadMode mode)
{
    Light light;
    light.direction = line->GetParam("dir")->AsPoint();
    light.color = line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f));
    light.type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);

    m_result.light.push_back(light);
}

void CLevelLoader::HandleCommand_CreateSpot(CLevelParserLine* line, LevelLoadMode mode)
{
    LightSpot lightSpot;
    lightSpot.position = line->GetParam("pos")->AsPoint()*m_result.unit;
    lightSpot.color = line->GetParam("color")->AsColor(Gfx::Color(0.5f, 0.5f, 0.5f, 1.0f));
    lightSpot.type = line->GetParam("type")->AsTerrainType(Gfx::ENG_OBJTYPE_NULL);

    m_result.lightSpots.push_back(lightSpot);
}

void CLevelLoader::HandleCommand_GroundSpot(CLevelParserLine* line, LevelLoadMode mode)
{
    GroundSpot spot;
    spot.position = line->GetParam("pos")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*m_result.unit;
    spot.radius = line->GetParam("radius")->AsFloat(10.0f)*m_result.unit;
    spot.color = line->GetParam("color")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
    spot.smooth = line->GetParam("smooth")->AsFloat(1.0f);
    spot.min = line->GetParam("min")->AsFloat(0.0f)*m_result.unit;
    spot.max = line->GetParam("max")->AsFloat(0.0f)*m_result.unit;

    m_result.groundSpots.push_back(spot);
}

void CLevelLoader::HandleCommand_WaterColor(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.waterAddColor = line->GetParam("color")->AsColor();
}

void CLevelLoader::HandleCommand_MapColor(CLevelParserLine* line, LevelLoadMode mode)
{
    Map map;
    map.floorColor = line->GetParam("floor")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
    map.waterColor = line->GetParam("water")->AsColor(Gfx::Color(0.533f, 0.533f, 0.533f, 0.533f));
    map.show = line->GetParam("show")->AsBool(true);
    map.toyIcon = line->GetParam("toyIcon")->AsBool(false);
    map.imageEnable = line->GetParam("image")->AsBool(false);
    if (map.imageEnable)
    {
        map.imageFileName = line->GetParam("filename")->AsPath("textures");
        Math::Vector offset = line->GetParam("offset")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f));
        map.imageOffsetX = offset.x;
        map.imageOffsetY = offset.z;
        map.imageZoom = line->GetParam("zoom")->AsFloat(1.0f);
        map.imageAngle = line->GetParam("angle")->AsFloat(0.0f)*Math::PI/180.0f;
        map.imageMode = line->GetParam("mode")->AsInt(0);
        map.imageDebug = line->GetParam("debug")->AsBool(false);
    }

    m_result.map = map;
}

void CLevelLoader::HandleCommand_MapZoom(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.map.defaultZoom = line->GetParam("factor")->AsFloat(2.0f);
    m_result.map.enable = line->GetParam("enable")->AsBool(true);
}

void CLevelLoader::HandleCommand_MaxFlyingHeight(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.terrain.maxFlyingHeight = line->GetParam("max")->AsFloat(280.0f)*m_result.unit;
}

void CLevelLoader::HandleCommand_AddFlyingHeight(CLevelParserLine* line, LevelLoadMode mode)
{
    Gfx::CTerrain::FlyingLimit fl;
    fl.center = line->GetParam("center")->AsPoint()*m_result.unit;
    fl.extRadius = line->GetParam("extRadius")->AsFloat(20.0f)*m_result.unit;
    fl.intRadius = line->GetParam("intRadius")->AsFloat(10.0f)*m_result.unit;
    fl.maxHeight = line->GetParam("maxHeight")->AsFloat(200.0f);
}

void CLevelLoader::HandleCommand_Camera(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.camera.eyePosition = line->GetParam("eye")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*m_result.unit;
    m_result.camera.lookAt = line->GetParam("lookat")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*m_result.unit;
    m_result.camera.delay = (mode == LevelLoadMode::Reset ? 0.0f : line->GetParam("delay")->AsFloat(0.0f));
    m_result.camera.overlayEffectOnStart = line->GetParam("fadeIn")->AsBool(false);
}

void CLevelLoader::HandleCommand_EndMissionTake(CLevelParserLine* line, LevelLoadMode mode)
{
    auto endTake = MakeUnique<CSceneEndCondition>();
    endTake->Read(line);
    if ( endTake->immediat )
        m_result.mission.endTakeImmediat = true;
    m_result.mission.endConditions.push_back(std::move(endTake));

    if (!line->GetParam("pos")->IsDefined() || !line->GetParam("dist")->IsDefined())
    {
        ShowLoadingWarning("The defaults for pos= and dist= are going to change, specify them explicitly. See issue #759 (https://git.io/vVBzH)");
    }
}

void CLevelLoader::HandleCommand_EndMissionDelay(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.mission.lostDelay  = line->GetParam("win")->AsFloat(2.0f);
    m_result.mission.winDelay = line->GetParam("lost")->AsFloat(2.0f);
}

void CLevelLoader::HandleCommand_EndMissionResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.mission.requiredResearch |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_ObligatoryToken(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string token = line->GetParam("text")->AsString();
    if (!line->GetParam("min")->IsDefined() && !line->GetParam("max")->IsDefined())
        GetLogger()->Warn("ObligatoryToken without specifying min/max is provided only for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" min=1\n", token.c_str());
    if (m_result.mission.obligatoryTokens.count(token))
        throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

    int min = line->GetParam("min")->AsInt(line->GetParam("max")->IsDefined() ? -1 : 1); // BACKWARDS COMPATIBILITY: if neither min or max are defined, default to min=1
    int max = line->GetParam("max")->AsInt(-1);


    if (min >= 0 && max >= 0 && min > max)
    {
        throw CLevelParserException("Incorrect ObligatoryToken specification - min cannot be greater than max");
    }

    m_result.mission.obligatoryTokens[token].min = min;
    m_result.mission.obligatoryTokens[token].max = max;
}

void CLevelLoader::HandleCommand_ProhibitedToken(CLevelParserLine* line, LevelLoadMode mode)
{
    std::string token = line->GetParam("text")->AsString();
    GetLogger()->Warn("ProhibitedToken is only provided for backwards compatibility - instead, do this: ObligatoryToken text=\"%s\" max=0\n", token.c_str());
    if (m_result.mission.obligatoryTokens.count(token))
        throw CLevelParserException("Incorrect ObligatoryToken specification - you cannot define a token twice");

    m_result.mission.obligatoryTokens[token].min = -1;
    m_result.mission.obligatoryTokens[token].max = 0;
}

void CLevelLoader::HandleCommand_EnableBuild(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.buildEnabled |= line->GetParam("type")->AsBuildFlag();
}

void CLevelLoader::HandleCommand_EnableResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.researchEnabled |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_DoneResearch(CLevelParserLine* line, LevelLoadMode mode)
{
    m_result.researchDone[0] |= line->GetParam("type")->AsResearchFlag();
}

void CLevelLoader::HandleCommand_NewScript(CLevelParserLine* line, LevelLoadMode mode)
{
    NewScriptName newScriptName(line->GetParam("type")->AsObjectType(OBJECT_NULL),
                                line->GetParam("name")->AsPath("ai"));

    m_result.newScriptNames.push_back(newScriptName);
}
