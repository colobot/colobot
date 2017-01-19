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

#include "common/restext.h"

#include "graphics/engine/planet.h"
#include "graphics/engine/terrain.h"

#include "level/level_load_mode.h"
#include "object/object_manager.h"
#include "level/parser/parser.h"
#include "level/robotmain.h"
#include "level/scene_conditions.h"

#include "object/object.h"

#include <boost/optional.hpp>

#include <string>


class MinMax;
class NewScriptName;
class CLevelLoader;

typedef std::function<void(const std::string&)> WarningCallback;
typedef std::function<void(CLevelLoader*, CLevelParserLine*, LevelLoadMode)> CommandFunction;

/**
 * \brief Tool to loading and building scene from file
 */
class CLevelLoader
{
public:
    struct TerrainGenerate
    {
        int     mosaicCount = 0;
        int     brickCountPow2 = 0;
        float   brickSize = 0.0f;
        float   vision = 0.0f;
        int     depth = 0;
        float   hardness = 0.0f;
    };

    struct TerrainLevel
    {
        std::vector<int> id = {};
        float   min = 0.0f;
        float   max = 0.0f;
        float   slope = 0.0f;
        float   freq = 0.0f;
        Math::Vector center = Math::Vector(0.0f, 0.0f, 0.0f);
        float   radius = 0.0f;
    };

    struct TerrainRelief
    {
        bool        random = true;
        std::string fileName = "";
        float       scaleRelief = 0.0f;
        bool        adjustBorder = false;

        TerrainRelief()
        {
            this->random = true;
        }

        TerrainRelief(const std::string& fileName, float scaleRelief, bool adjustBorder)
        {
            this->random = false;

            this->fileName = fileName;
            this->scaleRelief = scaleRelief;
            this->adjustBorder = adjustBorder;
        }
    };

    struct SimpleTerrainTexture
    {
        std::string         fileName = "";
        std::vector<int>    table = {};
        float               width = 0.0f;
        float               height = 0.0f;
    };

    struct ComplexTerrainTexture
    {
        std::vector<Gfx::CTerrain::TerrainMaterial> materials = {};
        int defaultMaterial = 0;
        std::vector<TerrainLevel> levels = {};
    };

    struct Terrain
    {
        TerrainGenerate         generate = TerrainGenerate();
        Math::Vector            windSpeed = Math::Vector(0.0f, 0.0f, 0.0f);
        TerrainRelief           relief = TerrainRelief();
        std::string             resources = "";                                 //!< Path to image containing info about resources on map
        bool                    simpleTextureEnabled = true;                    //!< If true, use simple texture, otherwise complex texture
        SimpleTerrainTexture    simpleTexture = SimpleTerrainTexture();
        ComplexTerrainTexture   complexTexture = ComplexTerrainTexture();
        float                   maxFlyingHeight = 0.0f;                         //!< Flying limit on the entire scene
        std::vector<Gfx::CTerrain::FlyingLimit> flyingLimits = {};              //!< Flying limits on specified spots on scene
    };

    struct Water
    {
        bool            exists = false;
        Gfx::WaterType  type1 = Gfx::WaterType::WATER_TT;
        Gfx::WaterType  type2 = Gfx::WaterType::WATER_TT;
        std::string     imageFileName = "";
        Gfx::Color      diffuse = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::Color      ambient = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        float           level = 0.0f;
        float           glint = 0.0f;
        Math::Vector    eddy = Math::Vector(0.0f, 0.0f, 0.0f);
        bool            lava = false;
        Gfx::Color      color = Gfx::CWater::COLOR_REF_WATER;
        float           colorShift = 0.0f;
    };

    struct Cloud
    {
        bool        exists = false;
        std::string fileName = "";
        Gfx::Color  diffuse = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::Color  ambient = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        float       level = 0.0f;
    };

    struct Lightning
    {
        bool exists = false;
        float sleep = 0.0f;
        float delay = 0.0f;
        float magnetic = 0.0f;
        float progress = 0.0f;
    };

    struct GroundSpot
    {
        Math::Vector position = Math::Vector(0.0f, 0.0f, 0.0f);
        float radius = 0.0f;
        Gfx::Color color = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        float min = 0.0f;
        float max = 0.0f;
        float smooth = 0.0f;
    };

    struct Fog
    {
        Gfx::ParticleType type = Gfx::ParticleType::PARTIFOG0;
        Math::Vector    position = Math::Vector(0.0f, 0.0f, 0.0f);
        float           height = 0.0f;
        float           delay = 0.0f;
        Math::Point     dim = Math::Point(0.0f, 0.0f);
    };

    struct Light
    {
        Math::Vector    direction = Math::Vector(0.0f, 0.0f, 0.0f);
        Gfx::Color      color = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::EngineObjectType type = Gfx::EngineObjectType::ENG_OBJTYPE_NULL;
    };

    struct LightSpot
    {
        Math::Vector    position = Math::Vector(0.0f, 0.0f, 0.0f);
        Gfx::Color      color = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::EngineObjectType type = Gfx::EngineObjectType::ENG_OBJTYPE_NULL;
    };

    struct Map
    {
        bool            enable = true;
        bool            show = true;
        bool            toyIcon = false;
        float           defaultZoom = 2.0f;
        Gfx::Color      floorColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::Color      waterColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        bool            imageEnable = false;
        std::string     imageFileName = "";
        float           imageZoom = 0.0f;
        float           imageOffsetX = 0.0f;
        float           imageOffsetY = 0.0f;
        float           imageAngle = 0.0f;
        int             imageMode = 0;
        bool            imageDebug = false;
    };

    struct Audio
    {
        std::vector<std::string> cacheAudioFileNames = {};
        std::vector<std::unique_ptr<CAudioChangeCondition>> audioChangeConditions = {};
        std::string         audioTrack = "";
        bool                audioRepeat = true;
        std::string         satcomAudioTrack = "";
        bool                satcomAudioRepeat = true;
        std::string         editorAudioTrack = "";
        bool                editorAudioRepeat = true;
    };

    struct Camera
    {
        Math::Vector    eyePosition = Math::Vector(0.0f, 0.0f, 0.0f);
        Math::Vector    lookAt = Math::Vector(0.0f, 0.0f, 0.0f);
        float           delay = 0.0f;
        bool            overlayEffectOnStart = false;
    };

    struct Mission
    {
        std::vector<std::unique_ptr<CSceneEndCondition>> endConditions = {};
        bool            endTakeImmediat = false;
        float           winDelay = 2.0f;
        float           lostDelay = 2.0f;
        int             requiredResearch = 0;
        std::map<std::string, MinMax> obligatoryTokens = {};
        bool            missionTimerEnabled = false;
        bool            missionTimerStarted = false;
        MissionType     missionType = MISSION_NORMAL;
        std::string     endingWin = "";                 //!< scene after win
        std::string     endingLost = "";                //!< scene after lost
    };

    //! Used to creating object in ::LevelLoadMode::Normal and ::LevelLoadMode::Reset mode
    struct ObjectCreate
    {
        ObjectCreate(ObjectCreateParams params, CLevelParserLine* line)
            : params(params), line(line)
        {

        }

        ObjectCreateParams params;
        CLevelParserLine* line;
    };

    //! Used to creating object in ::LevelLoadMode::LoadSavedGame mode
    struct LoadedObjectCreate
    {
        LoadedObjectCreate(ObjectCreate object, boost::optional<ObjectCreate> cargo, boost::optional<ObjectCreate> power)
            : object(object), cargo(cargo), power(power)
        {
            this->object = object;
            this->cargo = cargo;
            this->power = power;
        }

        ObjectCreate object;
        boost::optional<ObjectCreate> cargo;
        boost::optional<ObjectCreate> power;
    };

    //! Result of execution CLevelLoader::LoadScene()
    struct Result
    {
        Result();
        Result(Result &&) = default;
        Result& operator=(const Result &) = delete;
        Result(const Result &) = delete;

        float                       unit;

        Terrain                     terrain = Terrain();
        Water                       water = Water();
        Cloud                       cloud = Cloud();
        Lightning                   lightning = Lightning();
        std::vector<GroundSpot>     groundSpots = std::vector<GroundSpot>();
        std::vector<Fog>            fog = std::vector<Fog>();
        std::vector<Light>          light = std::vector<Light>();
        std::vector<LightSpot>      lightSpots = std::vector<LightSpot>();

        std::vector<ObjectCreate>   objects = {};                               //!< objects in ::LevelLoadMode::Normal and ::LevelLoadMode::Reset mode
        std::vector<LoadedObjectCreate> loadedObjects = {};                     //!< objects in ::LevelLoadMode::LoadSavedGame mode

        Map                         map = Map();
        Audio                       audio = Audio();
        Camera                      camera = Camera();
        Mission                     mission = Mission();

        int                         buildEnabled = 0;
        int                         researchEnabled = 0;
        std::map<int, int>          researchDone = { {0, 0} };

        std::vector<NewScriptName>  newScriptNames = {};
        std::string                 scriptName = "";
        std::string                 scriptFile = "";

        bool                        immediatSatcom = false;
        bool                        beginSatCom = false;
        std::vector<std::string>    infoFilenames = {};

        float                       airDeepView = 1000.0f;
        float                       waterDeepView = 1000.0f;
        float                       airFogStart = 0.75f;
        float                       waterFogStart = 0.75f;

        Gfx::Color                  airAmbientColor = Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f);
        Gfx::Color                  waterAmbientColor = Gfx::Color(0.5f, 0.5f, 0.5f, 0.5f);
        Gfx::Color                  airFogColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::Color                  waterFogColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        std::map<int, Gfx::Color>   newBotColors = {};
        Gfx::Color                  newAlienColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        Gfx::Color                  newGreenColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);
        std::string                 backgroundPath = "";
        Gfx::Color                  backgroundUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
        Gfx::Color                  backgroundDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
        Gfx::Color                  backgroundCloudUp = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
        Gfx::Color                  backgroundCloudDown = Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f);
        bool                        backgroundFull = false;

        std::map<int, std::string>  teamNames = {};
        float                       messageDelay = 1.0f;                        //!< how long message is displayed, 2.0f means twice longer than normal
        std::string                 secondTexture = "";                         //!< dirt on robots and buildings
        std::vector<Gfx::CPlanet::Planet> planets = {};                         //!< planets on the sky and in the space while cutscenes
        std::string                 foregroundName = "";                        //!< lens flare effect
        float                       traceQuality = 1.0f;                        //!< see Gfx::CEngine::SetTracePrecision()
        bool                        shortcut = true;
        float                       magnifyDamage = 1.0f;
        Gfx::Color                  waterAddColor = Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f);     //!< underwater camera effect
    };

    /**
     * \brief Read and process data from parser and return informations about scene in convenient form
     * \param sceneParser Contains information about scene
     * \param mode See ::LevelLoadMode
     * \param warningCallback Executed when loader want show warning
     * \param savedSceneParser Contains information about loaded game file, used only with LevelLoadMode::LoadSavedGame mode
     */
    static Result       LoadScene(CLevelReadOnlyParser* sceneParser, LevelLoadMode mode, WarningCallback warningCallback,
                                CLevelReadOnlyParser* savedSceneParser = nullptr);
    //! Restore stacks of execution from file
    static void         SetExecutionStack(const CObjectContainerProxy& objects, const std::string& filecbot);

private:
    struct Command
    {
        Command(CommandFunction function, int executionConditions)
        {
            this->function = function;
            this->executionConditions = executionConditions;
        }

        CommandFunction function;
        int executionConditions;
    };

    CLevelLoader(WarningCallback warningCllback);

    //! Sketchily check if parser is ill-formed, if true, throw error
    static void         Validate(CLevelReadOnlyParser* parser);

    void                ShowLoadingWarning(const std::string& message);

    //! Read object from saved game file
    ObjectCreate        ReadObject(CLevelParserLine *line);
    //! Read saved game file
    void                ReadScene(CLevelReadOnlyParser* parser);
    //! Restore stack of execution of object from file
    static bool         ReadFileStack(CObject *obj, FILE *file);

    /**
     * \biref Add handling of ::CLevelParserLine command while execution of CLevelLoader::LoadScene()
     * \param function Function that handles command, should has following form
     * \code
     *      void HandleCommand_%NAME_OF_COMMAND%(CLevelParserLine* line, LevelLoadMode mode);
     * \endcode
     * \param executionConditions Specifies for which modes command is processed, see ::LevelLoadMode
     */
    static std::shared_ptr<CLevelLoader::Command> MakeCommand(CommandFunction function, int executionConditions = LevelLoadMode::Normal | LevelLoadMode::LoadSavedGame);

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

private:
    //! List of commands created by CLevelLoader::MakeCommand()
    static std::map<std::string,std::shared_ptr<CLevelLoader::Command>> m_commands;

    //! Information about scene which will be returned by CLevelLoader::LoadScene()
    Result          m_result;

    WarningCallback     m_warningCallback;
};


