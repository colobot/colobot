/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include "common/singleton.h"

#include "level/build_type.h"
#include "level/level_category.h"
#include "level/mainmovie.h"
#include "level/research_type.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/particle.h"

#include "object/drive_type.h"
#include "object/mission_type.h"
#include "object/object_type.h"
#include "object/tool_type.h"

#include <deque>
#include <stdexcept>

enum Phase
{
    PHASE_WELCOME1,
    PHASE_WELCOME2,
    PHASE_WELCOME3,
    PHASE_PLAYER_SELECT,
    PHASE_APPERANCE,
    PHASE_MAIN_MENU,
    PHASE_LEVEL_LIST,
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
};
std::string PhaseToString(Phase phase);
bool IsInSimulationConfigPhase(Phase phase);
bool IsPhaseWithWorld(Phase phase);
bool IsMainMenuPhase(Phase phase);


class CController;
class CEventQueue;
class CSoundInterface;
class CLevelParserLine;
class CInput;
class CObjectManager;
class CSceneEndCondition;
class CAudioChangeCondition;
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
}

struct NewScriptName
{
    ObjectType  type = OBJECT_NULL;
    std::string name = "";
};


const int MAXSHOWLIMIT      = 5;
const int MAXSHOWPARTI      = 200;
const float SHOWLIMITTIME   = 20.0f;

const int MAXSCENE = 999;

struct ShowLimit
{
    bool            used = false;
    Math::Vector    pos;
    float           radius = 0.0f;
    int             total = 0;
    int             parti[MAXSHOWPARTI] = {};
    CObject*        link = nullptr;
    float           duration = 0.0f;
    float           time = 0.0f;
};


const int SATCOM_HUSTON     = 0;
const int SATCOM_SAT        = 1;
const int SATCOM_OBJECT     = 2;
const int SATCOM_LOADING    = 3;
const int SATCOM_PROG       = 4;
const int SATCOM_SOLUCE     = 5;
const int SATCOM_MAX        = 6;

class CRobotMain : public CSingleton<CRobotMain>
{
public:
    CRobotMain();
    virtual ~CRobotMain();

    Gfx::CCamera* GetCamera();
    Gfx::CTerrain* GetTerrain();
    Ui::CInterface* GetInterface();
    Ui::CDisplayText* GetDisplayText();

    void        CreateConfigFile();
    void        LoadConfigFile();

    void        ResetAfterVideoConfigChanged();
    void        ReloadAllTextures();

    void        ChangePhase(Phase phase);
    bool        ProcessEvent(Event &event);
    Phase       GetPhase();

    bool        CreateShortcuts();
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
    bool        GetFreePhoto();
    void        SetFriendAim(bool friendAim);
    bool        GetFriendAim();

    void        SetTracePrecision(float factor);
    float       GetTracePrecision();

    void        SetSpeed(float speed);
    float       GetSpeed();

    void        UpdateShortcuts();
    void        SelectHuman();
    CObject*    SearchHuman();
    CObject*    SearchToto();
    CObject*    SearchNearest(Math::Vector pos, CObject* pExclu);
    bool        SelectObject(CObject* pObj, bool displayError=true);
    CObject*    GetSelectObject();
    CObject*    DeselectAll();

    void        ResetObject();
    void        UpdateAudio(bool frame);
    void        SetEndMission(Error result, float delay);
    Error       CheckEndMission(bool frame);
    Error       CheckEndMissionForGroup(std::vector<CSceneEndCondition*>& endTakes);
    int         GetObligatoryToken();
    char*       GetObligatoryToken(int i);
    int         IsObligatoryToken(const char* token);
    bool        IsProhibitedToken(const char* token);
    void        UpdateMap();
    bool        GetShowMap();

    MainMovieType GetMainMovie();

    void        FlushDisplayInfo();
    void        StartDisplayInfo(int index, bool movie);
    void        StartDisplayInfo(const std::string& filename, int index);
    void        StopDisplayInfo();
    char*       GetDisplayInfoName(int index);
    int         GetDisplayInfoPosition(int index);
    void        SetDisplayInfoPosition(int index, int pos);

    void        StartSuspend();
    void        StopSuspend();

    float       GetGameTime();

    char*       GetTitle();
    char*       GetResume();
    char*       GetScriptName();
    char*       GetScriptFile();
    bool        GetTrainerPilot();
    bool        GetFixScene();
    bool        GetInterfaceGlint();
    bool        GetSoluce4();
    bool        GetMovies();
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
    std::string GetCustomLevelDir();
    void        SetReadScene(std::string path);
    void        UpdateChapterPassed();

    void        StartMusic();
    void        UpdatePause(PauseType pause);
    void        ClearInterface();
    void        ChangeColor();

    float       SearchNearestObject(Math::Vector center, CObject *exclu);
    bool        FreeSpace(Math::Vector &center, float minRadius, float maxRadius, float space, CObject *exclu);
    bool        FlatFreeSpace(Math::Vector &center, float minFlat, float minRadius, float maxRadius, float space, CObject *exclu);
    float       GetFlatZoneRadius(Math::Vector center, float maxRadius, CObject *exclu);
    void        HideDropZone(CObject* metal);
    void        ShowDropZone(CObject* metal, CObject* transporter);
    void        FlushShowLimit(int i);
    void        SetShowLimit(int i, Gfx::ParticleType parti, CObject *pObj, Math::Vector pos,
                             float radius, float duration=SHOWLIMITTIME);
    void        StartShowLimit();
    void        FrameShowLimit(float rTime);

    void        SaveAllScript();
    void        SaveOneScript(CObject *pObj);
    bool        SaveFileStack(CObject *pObj, FILE *file, int objRank);
    bool        ReadFileStack(CObject *pObj, FILE *file, int objRank);

    void        FlushNewScriptName();
    void        AddNewScriptName(ObjectType type, const std::string& name);
    std::string GetNewScriptName(ObjectType type, int rank);

    void        SelectPlayer(std::string playerName);
    CPlayerProfile* GetPlayerProfile();

    bool        IOIsBusy();
    bool        IOWriteScene(std::string filename, std::string filecbot, std::string filescreenshot, const std::string& info, bool emergencySave = false);
    void        IOWriteSceneFinished();
    CObject*    IOReadScene(std::string filename, std::string filecbot);
    void        IOWriteObject(CLevelParserLine *line, CObject* obj, const std::string& programDir, int objRank);
    CObject*    IOReadObject(CLevelParserLine *line, const std::string& programDir, const std::string& objCounterText, float objectProgress, int objRank = -1);

    int         CreateSpot(Math::Vector pos, Gfx::Color color);

    CObject*    GetSelect();

    void        DisplayError(Error err, CObject* pObj, float time=10.0f);
    void        DisplayError(Error err, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f);

    void        UpdateCustomLevelList();
    std::string GetCustomLevelName(int id);
    const std::vector<std::string>& GetCustomLevelList();

    //! Returns true if the game is on the loading screen
    bool        IsLoading();

    void        StartMissionTimer();

    void        SetAutosave(bool enable);
    bool        GetAutosave();
    void        SetAutosaveInterval(int interval);
    int         GetAutosaveInterval();
    void        SetAutosaveSlots(int slots);
    int         GetAutosaveSlots();

    //! Enable mode where completing mission closes the game
    void        SetExitAfterMission(bool exit);

    //! Returns true if player can interact with things manually
    bool        CanPlayerInteract();

    //! Returns team name for the given team id
    const std::string& GetTeamName(int id);

    //! Returns true if team-specific colored texture is available
    bool        IsTeamColorDefined(int id);

    //! Get/set enabled buildings
    //@{
    int         GetEnableBuild();
    void        SetEnableBuild(int enableBuild);
    //@}
    //! Get/set enabled researches
    //@{
    int         GetEnableResearch();
    void        SetEnableResearch(int enableResearch);
    //@}
    //! Get/set done researches
    //@{
    int         GetDoneResearch(int team);
    void        SetDoneResearch(int doneResearch, int team);
    //@}

    //! Returns true if the given building is enabled
    //@{
    bool        IsBuildingEnabled(BuildType type);
    bool        IsBuildingEnabled(ObjectType type);
    //@}
    //! Returns true if the given research is enabled
    bool        IsResearchEnabled(ResearchType type);
    //! Returns true if the given research is done
    bool        IsResearchDone(ResearchType type, int team);
    //! Marks research as done
    void        MarkResearchDone(ResearchType type, int team);

    //! Retruns true if all requirements to build this object are met (EnableBuild + DoneResearch)
    //@{
    bool        CanBuild(ObjectType type, int team);
    Error       CanBuildError(ObjectType type, int team);
    //@}

    //! Retruns true if all requirements to create this object in BotFactory are met (DoneResearch)
    //@{
    bool        CanFactory(ObjectType type, int team);
    Error       CanFactoryError(ObjectType type, int team);
    //@}

    void        RemoveFromSelectionHistory(CObject* object);

    //! Returns global magnifyDamage setting
    float       GetGlobalMagnifyDamage();

    void        StartDetectEffect(COldObject* object, CObject* target);

    bool        IsSelectable(CObject* pObj);

protected:
    bool        EventFrame(const Event &event);
    bool        EventObject(const Event &event);
    void        InitEye();

    void        ShowSaveIndicator(bool show);

    void        CreateScene(bool soluce, bool fixScene, bool resetObject);
    void        ResetCreate();

    void        LevelLoadingError(const std::string& error, const std::runtime_error& exception, Phase exitPhase = PHASE_LEVEL_LIST);

    Math::Vector LookatPoint(Math::Vector eye, float angleH, float angleV, float length);
    int         CreateLight(Math::Vector direction, Gfx::Color color);
    void        HiliteClear();
    void        HiliteObject(Math::Point pos);
    void        HiliteFrame(float rTime);
    void        CreateTooltip(Math::Point pos, const std::string& text);
    void        ClearTooltip();
    CObject*    DetectObject(Math::Point pos);
    void        ChangeCamera();
    void        RemoteCamera(float pan, float zoom, float rTime);
    void        KeyCamera(EventType event, InputSlot key);
    void        AbortMovie();
    void        SelectOneObject(CObject* pObj, bool displayError=true);
    void        HelpObject();
    bool        DeselectObject();
    void        DeleteAllObjects();
    void        UpdateInfoText();
    CObject*    SearchObject(ObjectType type);
    void        StartDisplayVisit(EventType event);
    void        FrameVisit(float rTime);
    void        StopDisplayVisit();
    void        ExecuteCmd(char *cmd);
    void        UpdateSpeedLabel();

    int         AutosaveRotate(bool freeOne);
    void        Autosave();
    bool        DestroySelectedObject();
    void        PushToSelectionHistory(CObject* obj);
    CObject*    PopFromSelectionHistory();

    void        CreateCodeBattleInterface();
    void        DestroyCodeBattleInterface();
    void        SetCodeBattleSpectatorMode(bool mode);


protected:
    CController*        m_ctrl = nullptr;
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
    CPauseManager*      m_pause = nullptr;
    CInput*             m_input = nullptr;
    std::unique_ptr<CObjectManager> m_objMan;
    std::unique_ptr<CMainMovie> m_movie;
    std::unique_ptr<Gfx::CModelManager> m_modelManager;
    std::unique_ptr<Gfx::CTerrain> m_terrain;
    std::unique_ptr<Gfx::CCamera> m_camera;
    std::unique_ptr<Ui::CMainUserInterface> m_ui;
    std::unique_ptr<Ui::CMainShort> m_short;
    std::unique_ptr<Ui::CMainMap> m_map;
    std::unique_ptr<Ui::CInterface> m_interface;
    std::unique_ptr<Ui::CDisplayInfo> m_displayInfo;
    std::unique_ptr<Ui::CDisplayText> m_displayText;
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
    std::string     m_sceneReadPath;

    float           m_winDelay = 0.0f;
    float           m_lostDelay = 0.0f;
    bool            m_fixScene = false;        // scene fixed, no interraction
    CObject*        m_base = nullptr;        // OBJECT_BASE exists in mission
    CObject*        m_selectObject = nullptr;

    Phase           m_phase = PHASE_WELCOME1;
    ActivePause*    m_userPause = nullptr;
    int             m_cameraRank = 0;
    Gfx::Color      m_color;
    bool            m_freePhoto = false;
    ActivePause*    m_freePhotoPause = nullptr;
    bool            m_cmdEdit = false;
    ActivePause*    m_cmdEditPause = nullptr;
    bool            m_selectInsect = false;
    bool            m_showSoluce = false;
    bool            m_showAll = false;
    bool            m_cheatRadar = false;
    bool            m_shortCut = false;
    std::string     m_audioTrack;
    bool            m_audioRepeat = false;
    std::string     m_satcomTrack;
    bool            m_satcomRepeat = false;
    std::string     m_editorTrack;
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
    bool            m_trainerPilot = false;    // remote trainer?
    bool            m_friendAim = false;
    bool            m_resetCreate = false;
    bool            m_mapShow = false;
    bool            m_mapImage = false;
    char            m_mapFilename[100] = {};

    ActivePause*    m_suspend = nullptr;
    Gfx::CameraType m_suspendInitCamera = Gfx::CAM_TYPE_NULL;

    Math::Point     m_tooltipPos;
    std::string     m_tooltipName;
    float           m_tooltipTime = 0.0f;

    char            m_infoFilename[SATCOM_MAX][100] = {}; // names of text files
    CObject*        m_infoObject = nullptr;
    int             m_infoIndex = 0;
    int             m_infoPos[SATCOM_MAX] = {};
    int             m_infoUsed = 0;
    ActivePause*    m_satcomMoviePause = nullptr;

    char            m_title[100] = {};
    char            m_resume[500] = {};
    char            m_scriptName[100] = {};
    char            m_scriptFile[100] = {};
    int             m_endingWinRank = 0;
    int             m_endingLostRank = 0;
    bool            m_winTerminate = false;

    float           m_globalMagnifyDamage = 0.0f;

    bool            m_exitAfterMission = false;

    bool            m_codeBattleInit = false;
    bool            m_codeBattleStarted = false;
    //! Code battle spectator mode, hides object UI, changes camera to CAM_TYPE_PLANE and allows for switching to free camera by clicking outside of any object
    bool            m_codeBattleSpectator = true;

    std::map<int, std::string> m_teamNames;

    std::vector<NewScriptName> m_newScriptName;

    float           m_cameraPan = 0.0f;
    float           m_cameraZoom = 0.0f;

    EventType       m_visitLast = EVENT_NULL;
    CObject*        m_visitObject = nullptr;
    CObject*        m_visitArrow = nullptr;
    float           m_visitTime = 0.0f;
    float           m_visitParticle = 0.0f;
    Math::Vector    m_visitPos;
    Math::Vector    m_visitPosArrow;
    ActivePause*    m_visitPause = nullptr;

    std::vector<std::unique_ptr<CSceneEndCondition>> m_endTake;
    long            m_endTakeResearch = 0;
    float           m_endTakeWinDelay = 0.0f;
    float           m_endTakeLostDelay = 0.0f;

    std::vector<std::unique_ptr<CAudioChangeCondition>> m_audioChange;

    int             m_obligatoryTotal = 0;
    char            m_obligatoryToken[100][20] = {};
    int             m_prohibitedTotal = 0;
    char            m_prohibitedToken[100][20] = {};

    //! Enabled buildings
    int             m_build = 0;
    //! Available researches
    long            m_researchEnable = 0;
    //! Done researches for each team
    std::map<int, int>  m_researchDone;

    Error           m_missionResult = ERR_OK;

    ShowLimit       m_showLimit[MAXSHOWLIMIT];

    Gfx::Color      m_colorRefBot;
    std::map<int, Gfx::Color> m_colorNewBot;
    Gfx::Color      m_colorRefAlien;
    Gfx::Color      m_colorNewAlien;
    Gfx::Color      m_colorRefGreen;
    Gfx::Color      m_colorNewGreen;
    Gfx::Color      m_colorRefWater;
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
};
