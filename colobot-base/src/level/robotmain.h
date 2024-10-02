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
 * \file level/robotmain.h
 * \brief CRobotMain - main class of Colobot game engine
 */

#pragma once

#include "app/pausemanager.h"

#include "common/error.h"
#include "common/event.h"
#include "common/singleton.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/particle.h"

#include "level/build_type.h"
#include "level/level_category.h"
#include "level/mainmovie.h"
#include "level/research_type.h"

#include "object/drive_type.h"
#include "object/mission_type.h"
#include "object/object_type.h"
#include "object/tool_type.h"

#include <filesystem>
#include <deque>
#include <map>
#include <set>
#include <stdexcept>

#include <glm/glm.hpp>

enum Phase
{
    PHASE_WELCOME1,
    PHASE_WELCOME2,
    PHASE_WELCOME3,
    PHASE_PLAYER_SELECT,
    PHASE_APPEARANCE,
    PHASE_MAIN_MENU,
    PHASE_LEVEL_LIST,
    PHASE_MOD_LIST,
    PHASE_SIMUL,
    PHASE_SETUPd,
    PHASE_SETUPg,
    PHASE_SETUPp,
    PHASE_SETUPc,
    PHASE_SETUPs,
    PHASE_SETUPds,
    PHASE_SETUPgs,
    PHASE_SETUPps,
    PHASE_SETUPcs,
    PHASE_SETUPss,
    PHASE_WRITEs,
    PHASE_READ,
    PHASE_READs,
    PHASE_WIN,
    PHASE_LOST,
    PHASE_QUIT_SCREEN,
    PHASE_SATCOM,
};
std::string PhaseToString(Phase phase);
bool IsInSimulationConfigPhase(Phase phase);
bool IsPhaseWithWorld(Phase phase);
bool IsMainMenuPhase(Phase phase);

class CApplication;
class CEventQueue;
class CSoundInterface;
class CLevelParserLine;
class CInput;
class CObjectManager;
class CSceneEndCondition;
class CAudioChangeCondition;
class CScoreboard;
class CPlayerProfile;
class CSettings;
class COldObject;
class CPauseManager;
struct ActivePause;

namespace Gfx
{
class CEngine;
class CLightManager;
class CWater;
class CCloud;
class CLightning;
class COldModelManager;
class CPlanet;
class CTerrain;
class CModelManager;
}

namespace Ui
{
class CMainUserInterface;
class CMainShort;
class CMainMap;
class CInterface;
class CDisplayText;
class CDisplayInfo;
class CDebugMenu;
}

struct NewScriptName
{
    ObjectType  type = OBJECT_NULL;
    std::string name = "";

    NewScriptName(ObjectType type, const std::string& name) : type(type), name(name) {}
};


const int MAXSHOWLIMIT      = 5;
const int MAXSHOWPARTI      = 200;
const float SHOWLIMITTIME   = 20.0f;

const int MAXSCENE = 999;

struct ShowLimit
{
    bool            used = false;
    glm::vec3       pos;
    float           radius = 0.0f;
    int             total = 0;
    int             parti[MAXSHOWPARTI] = {};
    CObject*        link = nullptr;
    float           duration = 0.0f;
    float           time = 0.0f;
};

struct MinMax
{
    int min = -1;
    int max = -1;
};

struct Viewpoint
{
    glm::vec3    eye{};
    glm::vec3    look{};
    int             button = 13; // 13 is the camera button
};

const int SATCOM_HUSTON     = 0;
const int SATCOM_SAT        = 1;
const int SATCOM_OBJECT     = 2;
const int SATCOM_LOADING    = 3;
const int SATCOM_PROG       = 4;
const int SATCOM_SOLUCE     = 5;
const int SATCOM_MAX        = 6;

/**
 * \brief Main class managing the game world
 *
 * This is the main class of the whole game engine. It's main job is to manage main parts of the gameplay,
 * like loading levels and checking for win conditions, but it's also a place where all things that don't fit
 * elsewhere have landed.
 *
 * \todo In the future, it would be nice to refactor this class to remove as much unrelated stuff as possible
 *
 * \nosubgrouping
 */
class CRobotMain : public CSingleton<CRobotMain>
{
public:
    CRobotMain();
    virtual ~CRobotMain();

    Gfx::CCamera* GetCamera();
    Gfx::CTerrain* GetTerrain();
    Ui::CInterface* GetInterface();
    Ui::CDisplayText* GetDisplayText();
    CPauseManager* GetPauseManager();

    /**
     * \name Phase management
     */
    //@{
    void        ChangePhase(Phase phase);
    bool        ProcessEvent(Event &event);
    Phase       GetPhase();
    //@}

    //! Load the scene for appearance customization
    void        ScenePerso();

    void        SetMovieLock(bool lock);
    bool        GetMovieLock();
    bool        GetInfoLock();
    void        SetSatComLock(bool lock);
    bool        GetSatComLock();
    void        SetEditLock(bool lock, bool edit);
    bool        GetEditLock();
    void        SetEditFull(bool full);
    bool        GetEditFull();
    void        SetFriendAim(bool friendAim);
    bool        GetFriendAim();

    //! \name Simulation speed management
    //@{
    void        SetSpeed(float speed);
    float       GetSpeed();
    //@}

    //! \brief Create the shortcuts at the top of the screen, if they should be visible
    //! \see CMainShort::CreateShortcuts
    bool        CreateShortcuts();
    //! \brief Update the shortcuts at the top of the screen
    //! \see CMainShort::UpdateShortcuts
    void        UpdateShortcuts();
    //! Find the astronaut (::OBJECT_HUMAN) object
    CObject*    SearchHuman();
    /**
     * \brief Select an object
     * \param obj Object to select
     * \param displayError If true and the object is currently in error state, automatically display the error message
     *
     * \note This function automatically adds objects to selection history (see PushToSelectionHistory())
     */
    bool        SelectObject(CObject* obj, bool displayError=true);
    //! Return the object that was selected at the start of the scene
    CObject*    GetSelectObject();
    //! Deselect currently selected object
    //! \return Object that was deselected
    CObject*    DeselectAll();

    void        ResetObject();
    void        UpdateAudio(bool frame);
    void        SetMissionResultFromScript(Error result, float delay);
    Error       CheckEndMission(bool frame);
    Error       ProcessEndMissionTake();
    Error       ProcessEndMissionTakeForGroup(std::vector<CSceneEndCondition*>& endTakes);
    const std::map<std::string, MinMax>& GetObligatoryTokenList();
    void        UpdateMap();
    bool        GetShowMap();

    MainMovieType GetMainMovie();

    void        FlushDisplayInfo();
    void        StartDisplayInfo(int index, bool movie);
    void        StartDisplayInfo(const std::filesystem::path& filename, int index);
    void        StopDisplayInfo();
    const std::filesystem::path& GetDisplayInfoName(int index);

    void        StartSuspend();
    void        StopSuspend();

    float       GetGameTime();

    const std::string& GetScriptName();
    const std::filesystem::path& GetScriptFile();
    bool        GetTrainerPilot();
    bool        GetPlusTrainer();
    bool        GetPlusExplorer();
    bool        GetFixScene();
    bool        GetShowSoluce();
    bool        GetSceneSoluce();
    bool        GetShowAll();
    bool        GetRadar();
    MissionType GetMissionType();

    int         GetGamerFace();
    int         GetGamerGlasses();
    bool        GetGamerOnlyHead();
    float       GetPersoAngle();

    void        SetLevel(LevelCategory cat, int chap, int rank);
    LevelCategory GetLevelCategory();
    int         GetLevelChap();
    int         GetLevelRank();
    std::filesystem::path GetCustomLevelDir();
    void        SetReadScene(const std::filesystem::path& path);
    void        UpdateChapterPassed();

    void        StartMusic();
    void        UpdatePause(PauseType pause);
    void        UpdatePauseMusic(PauseMusic music);
    void        ClearInterface();

    bool        FreeSpace(glm::vec3 &center, float minRadius, float maxRadius, float space, CObject *exclu);
    bool        FlatFreeSpace(glm::vec3 &center, float minFlat, float minRadius, float maxRadius, float space, CObject *exclu);
    //! \name In-world indicators
    //@{
    float       GetFlatZoneRadius(glm::vec3 center, float maxRadius, CObject *exclu);
    void        HideDropZone(CObject* metal);
    void        ShowDropZone(CObject* metal, CObject* transporter);
    void        FlushShowLimit(int i);
    void        SetShowLimit(int i, Gfx::ParticleType parti, CObject *obj, glm::vec3 pos,
                             float radius, float duration=SHOWLIMITTIME);
    void        StartShowLimit();
    void        FrameShowLimit(float rTime);
    //@}

    void        SaveAllScript();
    void        SaveOneScript(CObject *obj);
    bool        SaveFileStack(CObject *obj, std::ostream &ostr);
    bool        ReadFileStack(CObject *obj, std::istream &istr);

    //! Return list of scripts to load to robot created in BotFactory
    std::vector<std::string> GetNewScriptNames(ObjectType type);

    //! Return the scoreboard manager
    //! Note: this may return nullptr if the scoreboard is not enabled!
    CScoreboard* GetScoreboard();

    void        SelectPlayer(std::string playerName);
    CPlayerProfile* GetPlayerProfile();

    /**
     * \name Saved game read/write
     */
    //@{
    bool        IOIsBusy();
    bool        IOWriteScene(const std::filesystem::path& filename,
                    const std::filesystem::path& filecbot,
                    const std::filesystem::path& filescreenshot,
                    const std::string& info, bool emergencySave = false);
    void        IOWriteSceneFinished();
    CObject*    IOReadScene(const std::filesystem::path& filename,
                    const std::filesystem::path& filecbot);
    void        IOWriteObject(CLevelParserLine *line, CObject* obj,
                    const std::filesystem::path& programDir, int objRank);
    CObject*    IOReadObject(CLevelParserLine *line,
                    const std::filesystem::path& programDir,
                    const std::string& objCounterText,
                    float objectProgress, int objRank = -1);
    //@}

    int         CreateSpot(glm::vec3 pos, Gfx::Color color);

    //! Find the currently selected object
    CObject*    GetSelect();

    void        DisplayError(Error err, CObject* pObj, float time=10.0f);
    void        DisplayError(Error err, glm::vec3 goal, float height=15.0f, float dist=60.0f, float time=10.0f);

    void        UpdateCustomLevelList();
    std::filesystem::path GetCustomLevelName(int id);
    const std::vector<std::string>& GetCustomLevelList();

    //! Returns true if the game is on the loading screen
    bool        IsLoading();

    void        StartMissionTimer();

    /**
     * \name Autosave management
     */
    //@{
    void        SetAutosave(bool enable);
    bool        GetAutosave();
    void        SetAutosaveInterval(int interval);
    int         GetAutosaveInterval();
    void        SetAutosaveSlots(int slots);
    int         GetAutosaveSlots();
    //@}

    //! Enable mode where completing mission closes the game
    void        SetExitAfterMission(bool exit);

    //! Load saved game (used by command line argument)
    void        LoadSaveFromDirName(const std::filesystem::path& gameDir);

    //! Returns true if player can interact with things manually
    bool        CanPlayerInteract();

    /**
     * \name Team definition management
     */
    //@{
    //! Returns team name for the given team id
    const std::string& GetTeamName(int id);

    //! Returns true if team-specific colored texture is available
    bool        IsTeamColorDefined(int id);
    //@}

    /**
     * \name EnableBuild/EnableResearch/DoneResearch
     * Management of enabled buildings, enabled researches, and completed researches
     */
    //@{
    /**
     * \brief Get enabled buildings
     * \return Bitmask of BuildType values
     */
    int         GetEnableBuild();
    /**
     * \brief Set enabled buildings
     * \param enableBuild Bitmask of BuildType values
     */
    void        SetEnableBuild(int enableBuild);

    /**
     * \brief Get enabled researches
     * \return Bitmask of ResearchType values
     */
    int         GetEnableResearch();
    /**
     * \brief Set enabled researches
     * \param enableResearch Bitmask of ResearchType values
     */
    void        SetEnableResearch(int enableResearch);
    /**
     * \brief Get done researches
     * \param team Team to get researches for
     * \return Bitmask of ResearchType values
     */
    int         GetDoneResearch(int team = 0);
    /**
     * \brief Set done researches
     * \param doneResearch Bitmask of ResearchType values
     * \param team Team to set researches for
     */
    void        SetDoneResearch(int doneResearch, int team = 0);

    //! \brief Check if the given building is enabled
    bool        IsBuildingEnabled(BuildType type);
    //! \brief Check if the given building is enabled
    bool        IsBuildingEnabled(ObjectType type);
    //! \brief Check if the given research is enabled
    bool        IsResearchEnabled(ResearchType type);
    //! \brief Check if the given research is done
    bool        IsResearchDone(ResearchType type, int team);
    //! \brief Mark given research as done
    void        MarkResearchDone(ResearchType type, int team);

    //! \brief Returns a color associated with a team or vehicle color if none defined
    const Gfx::Color& GetTeamColor(int team);
    //! \brief Returns vehicle color
    const Gfx::Color& GetVehicleColor();
    //! \brief Returns alien color
    const Gfx::Color& GetAlienColor();
    //! \brief Returns plant color
    const Gfx::Color& GetGreeneryColor();
    //! \brief Returns water color
    const Gfx::Color& GetWaterColor();

    /**
     * \brief Check if all requirements to build this object are met (EnableBuild + DoneResearch)
     * \return true if the building can be built, false otherwise
     * \see CanBuildError() for a version which returns a specific reason for the build being denied
     */
    inline bool CanBuild(ObjectType type, int team)
    {
        return CanBuildError(type, team) == ERR_OK;
    }
    /**
     * \brief Check if all requirements to build this object are met (EnableBuild + DoneResearch)
     * \return One of Error values - ::ERR_OK if the building can be built, ::ERR_BUILD_DISABLED or ::ERR_BUILD_RESEARCH otherwise
     * \see CanBuild() for a version which returns a boolean
     */
    Error       CanBuildError(ObjectType type, int team);

    /**
     * \brief Check if all requirements to build this object in BotFactory are met (DoneResearch)
     * \return true if the robot can be built, false otherwise
     * \see CanFactoryError() for a version which returns a specific reason for the build being denied
     */
    inline bool CanFactory(ObjectType type, int team)
    {
        return CanFactoryError(type, team) == ERR_OK;
    }
    /**
     * \brief Check if all requirements to build this object in BotFactory are met (DoneResearch)
     * \return One of Error values - ::ERR_OK if the robot can be built, ::ERR_BUILD_DISABLED or ::ERR_BUILD_RESEARCH otherwise
     * \see CanFactory() for a version which returns a boolean
     */
    Error       CanFactoryError(ObjectType type, int team);
    //@}

    void        CutObjectLink(CObject* object);

    //! Returns global magnifyDamage setting
    float       GetGlobalMagnifyDamage();

    //! Returns global NuclearCell capacity Setting
    float       GetGlobalNuclearCapacity();
    //! Returns global PowerCell capacity setting
    float       GetGlobalCellCapacity();

    void        StartDetectEffect(COldObject* object, CObject* target);

    //! Enable crash sphere debug rendering
    void SetDebugCrashSpheres(bool draw);
    //! Check if crash sphere debug rendering is enabled
    bool GetDebugCrashSpheres();

    //! Returns a set of all team IDs in the current level
    std::set<int> GetAllTeams();
    //! Returns a set of all team IDs in the current level that are still active
    std::set<int> GetAllActiveTeams();

protected:
    bool        EventFrame(const Event &event);
    bool        EventObject(const Event &event);
    void        InitEye();

    void        ShowSaveIndicator(bool show);

    void        CreateScene(bool soluce, bool fixScene, bool resetObject);
    void        ResetCreate();

    void        LevelLoadingError(const std::string& error, const std::runtime_error& exception, Phase exitPhase = PHASE_LEVEL_LIST);

    int         CreateLight(glm::vec3 direction, Gfx::Color color);
    void        HiliteClear();
    void        HiliteObject(const glm::vec2& pos);
    void        HiliteFrame(float rTime);
    void        CreateTooltip(const glm::vec2& pos, const std::string& text);
    void        ClearTooltip();
    CObject*    DetectObject(const glm::vec2& pos);
    void        ChangeCamera();
    void        AbortMovie();
    //! \brief Select an object, without deselecting the previous one
    void        SelectOneObject(CObject* obj, bool displayError=true);
    void        HelpObject();
    //! \brief Switch to previous object
    //! \see PopFromSelectionHistory()
    bool        DeselectObject();
    void        DeleteAllObjects();
    void        UpdateInfoText();
    void        StartDisplayVisit(EventType event);
    void        FrameVisit(float rTime);
    void        StopDisplayVisit();
    void        ExecuteCmd(const std::string& cmd);
    void        UpdateSpeedLabel();

    void        AutosaveRotate();
    void        Autosave();
    void        QuickSave();
    void        QuickLoad();
    bool        DestroySelectedObject();
    void        PushToSelectionHistory(CObject* obj);
    CObject*    PopFromSelectionHistory();

    //! \name Code battle interface
    //@{
    void        CreateCodeBattleInterface();
    void        UpdateCodeBattleInterface();
    void        ApplyCodeBattleInterface();
    void        DestroyCodeBattleInterface();
    void        SetCodeBattleSpectatorMode(bool mode);
    //@}

    void        UpdateDebugCrashSpheres();

    //! Adds element to the beginning of command history
    void        PushToCommandHistory(std::string cmd);
    //! Returns next/previous element from command history and updates index
    //@{
    std::string    GetNextFromCommandHistory();
    std::string    GetPreviousFromCommandHistory();
    //@}

protected:
    CApplication*       m_app = nullptr;
    CEventQueue*        m_eventQueue = nullptr;
    Gfx::CEngine*       m_engine = nullptr;
    Gfx::CParticle*     m_particle = nullptr;
    Gfx::CWater*        m_water = nullptr;
    Gfx::CCloud*        m_cloud = nullptr;
    Gfx::CLightning*    m_lightning = nullptr;
    Gfx::CPlanet*       m_planet = nullptr;
    Gfx::COldModelManager* m_oldModelManager = nullptr;
    Gfx::CLightManager* m_lightMan = nullptr;
    CSoundInterface*    m_sound = nullptr;
    CInput*             m_input = nullptr;
    std::unique_ptr<CObjectManager> m_objMan;
    std::unique_ptr<CMainMovie> m_movie;
    std::unique_ptr<CPauseManager> m_pause;
    std::unique_ptr<Gfx::CModelManager> m_modelManager;
    std::unique_ptr<Gfx::CTerrain> m_terrain;
    std::unique_ptr<Gfx::CCamera> m_camera;
    std::unique_ptr<Ui::CMainUserInterface> m_ui;
    std::unique_ptr<Ui::CMainShort> m_short;
    std::unique_ptr<Ui::CMainMap> m_map;
    std::unique_ptr<Ui::CInterface> m_interface;
    std::unique_ptr<Ui::CDisplayInfo> m_displayInfo;
    std::unique_ptr<Ui::CDisplayText> m_displayText;
    std::unique_ptr<Ui::CDebugMenu> m_debugMenu;
    std::unique_ptr<CSettings> m_settings;

    //! Progress of loaded player
    std::unique_ptr<CPlayerProfile> m_playerProfile;


    //! Time since level start, including pause and intro movie
    float           m_time = 0.0f;
    //! Playing time since level start
    float           m_gameTime = 0.0f;
    //! Playing time since level start, not dependent on simulation speed
    float           m_gameTimeAbsolute = 0.0f;

    LevelCategory   m_levelCategory;
    int             m_levelChap = 0;
    int             m_levelRank = 0;
    //! if set, loads this file instead of building from category/chap/rank
    std::filesystem::path m_levelFile = "";
    std::filesystem::path m_sceneReadPath = "";

    float           m_winDelay = 0.0f;
    float           m_lostDelay = 0.0f;
    bool            m_fixScene = false;        // scene fixed, no interraction
    CObject*        m_base = nullptr;        // OBJECT_BASE exists in mission
    CObject*        m_selectObject = nullptr;

    Phase           m_phase = PHASE_WELCOME1;
    ActivePause*    m_userPause = nullptr;
    ActivePause*    m_focusPause = nullptr;
    ActivePause*    m_freePhotoPause = nullptr;
    bool            m_cmdEdit = false;
    ActivePause*    m_cmdEditPause = nullptr;
    bool            m_cheatSelectInsect = false;
    bool            m_cheatShowSoluce = false;
    bool            m_cheatAllMission = false;
    bool            m_cheatRadar = false;
    bool            m_shortCut = false;
    std::filesystem::path m_audioTrack;
    bool            m_audioRepeat = false;
    std::filesystem::path m_satcomTrack;
    bool            m_satcomRepeat = false;
    std::filesystem::path m_editorTrack;
    bool            m_editorRepeat = false;
    int             m_movieInfoIndex = 0;

    CObject*        m_controller = nullptr;

    MissionType     m_missionType = MISSION_NORMAL;
    bool            m_immediatSatCom = false;  // SatCom immediately?
    bool            m_beginSatCom = false;     // messages SatCom poster?
    bool            m_lockedSatCom = false;    // SatCom locked?
    bool            m_movieLock = false;       // movie in progress?
    bool            m_satComLock = false;      // call of SatCom is possible?
    bool            m_editLock = false;        // edition in progress?
    bool            m_editFull = false;        // edition in full screen?
    bool            m_hilite = false;
    bool            m_cheatTrainerPilot = false;    // remote trainer?
    bool            m_friendAim = false;
    bool            m_resetCreate = false;
    bool            m_mapShow = false;
    bool            m_mapImage = false;
    std::filesystem::path m_mapFilename;

    ActivePause*    m_suspend = nullptr;

    glm::vec2       m_tooltipPos;
    std::string     m_tooltipName;
    float           m_tooltipTime = 0.0f;

    std::array<std::filesystem::path, SATCOM_MAX> m_infoFilename; // names of text files
    CObject*        m_infoObject = nullptr;
    int             m_infoUsed = 0;
    ActivePause*    m_satcomMoviePause = nullptr;

    std::string     m_scriptName = "";
    std::filesystem::path m_scriptFile = "";
    std::filesystem::path m_endingWin = "";
    std::filesystem::path m_endingLost = "";
    bool            m_winTerminate = false;

    float           m_globalMagnifyDamage = 0.0f;

    float           m_globalNuclearCapacity = 10.0f;
    float           m_globalCellCapacity = 1.0f;

    bool            m_exitAfterMission = false;

    bool            m_codeBattleInit = false;
    bool            m_codeBattleStarted = false;
    //! Code battle spectator mode, hides object UI, changes camera to CAM_TYPE_PLANE and allows for switching to free camera by clicking outside of any object
    bool            m_codeBattleSpectator = true;

    std::map<int, std::string> m_teamNames;

    std::vector<NewScriptName> m_newScriptName;

    EventType       m_visitLast = EVENT_NULL;
    CObject*        m_visitObject = nullptr;
    CObject*        m_visitArrow = nullptr;
    float           m_visitTime = 0.0f;
    float           m_visitParticle = 0.0f;
    glm::vec3       m_visitPos;
    glm::vec3       m_visitPosArrow;
    ActivePause*    m_visitPause = nullptr;

    std::vector<std::unique_ptr<CSceneEndCondition>> m_endTake;
    //! If true, the mission ends immediately after completing the requirements without requiring SpaceShip takeoff
    bool            m_endTakeImmediat = false;
    long            m_endTakeResearch = 0;
    float           m_endTakeTimeout = -1.0f;
    bool            m_endTakeTeamImmediateWin = false;
    float           m_endTakeWinDelay = 0.0f;
    float           m_endTakeLostDelay = 0.0f;
    //! Set to true for teams that have already finished
    std::map<int, bool> m_teamFinished;

    std::vector<std::unique_ptr<CAudioChangeCondition>> m_audioChange;

    //! The scoreboard
    //! If the scoreboard is not enabled for this level, this will be null
    std::unique_ptr<CScoreboard> m_scoreboard;

    std::map<std::string, MinMax> m_obligatoryTokens;

    //! Enabled buildings
    int             m_build = 0;
    //! Available researches
    long            m_researchEnable = 0;
    //! Done researches for each team
    std::map<int, int>  m_researchDone;

    Error           m_missionResult = ERR_OK;
    //! true if m_missionResult has been set by LevelController script, this disables normal EndMissionTake processing
    bool            m_missionResultFromScript = false;

    ShowLimit       m_showLimit[MAXSHOWLIMIT];

    std::map<int, Gfx::Color> m_colorNewBot;
    Gfx::Color      m_colorNewAlien;
    Gfx::Color      m_colorNewGreen;
    Gfx::Color      m_colorNewWater;
    float           m_colorShiftWater = 0.0f;

    bool            m_missionTimerEnabled = false;
    bool            m_missionTimerStarted = false;
    float           m_missionTimer = 0.0f;

    bool            m_autosave = false;
    int             m_autosaveInterval = 0;
    int             m_autosaveSlots = 0;
    float           m_autosaveLast = 0.0f;

    int             m_shotSaving = 0;

    std::deque<CObject*> m_selectionHistory;
    bool            m_debugCrashSpheres;

    //! Cheat console command history
    std::deque<std::string> m_commandHistory;
    //! Index of currently selected element in command history
    int             m_commandHistoryIndex;

    //! Vector of available viewpoints
    std::vector<Viewpoint> m_viewpoints;
};
