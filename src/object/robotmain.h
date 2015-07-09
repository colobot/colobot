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
 * \file object/robotmain.h
 * \brief CRobotMain - main class of Colobot game engine
 */

#pragma once


#include "common/global.h"
#include "common/singleton.h"

#include "graphics/engine/particle.h"

#include "object/object.h"
#include "object/drive_type.h"
#include "object/tool_type.h"
#include "object/mainmovie.h"
#include "object/mission_type.h"

#include "app/pausemanager.h"

#include <stdio.h>

enum Phase
{
    PHASE_INIT,
    PHASE_TERM,
    PHASE_NAME,
    PHASE_PERSO,
    PHASE_TRAINER,
    PHASE_DEFI,
    PHASE_MISSION,
    PHASE_FREE,
    PHASE_USER,
    PHASE_LOADING,
    PHASE_SIMUL,
    PHASE_MODEL,
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
    PHASE_WRITE,
    PHASE_READ,
    PHASE_WRITEs,
    PHASE_READs,
    PHASE_WIN,
    PHASE_LOST,
    PHASE_WELCOME1,
    PHASE_WELCOME2,
    PHASE_WELCOME3,
    PHASE_GENERIC,
};


class CController;
class CEventQueue;
class CSoundInterface;
class CLevelParserLine;
class CInput;
class CObjectManager;
class CSceneEndCondition;
class CAudioChangeCondition;

namespace Gfx {
class CEngine;
class CLightManager;
class CWater;
class CCloud;
class CLightning;
class CPlanet;
class CTerrain;
}

namespace Ui {
class CMainDialog;
class CMainShort;
class CMainMap;
class CInterface;
class CDisplayText;
class CDisplayInfo;
}

const int MAXNEWSCRIPTNAME = 20;

struct NewScriptName
{
    bool        used;
    ObjectType  type;
    char        name[40];
};


const int MAXSHOWLIMIT      = 5;
const int MAXSHOWPARTI      = 200;
const float SHOWLIMITTIME   = 20.0f;

struct ShowLimit
{
    bool            used;
    Math::Vector    pos;
    float           radius;
    int             total;
    int             parti[MAXSHOWPARTI];
    CObject*        link;
    float           duration;
    float           time;
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
    CRobotMain(CController* controller);
    virtual ~CRobotMain();

    void Create(bool loadProfile = true);

    Gfx::CCamera* GetCamera();
    Gfx::CTerrain* GetTerrain();
    Ui::CInterface* GetInterface();
    Ui::CDisplayText* GetDisplayText();

    void        CreateIni();
    void        LoadIni();

    void        ResetAfterDeviceChanged();

    void        ChangePhase(Phase phase);
    bool        ProcessEvent(Event &event);

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

    void        ChangePause(PauseType pause);

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
    bool        DeleteObject();

    void        ResetObject();
    void        ResetCreate();
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
    void        StartDisplayInfo(const char *filename, int index);
    void        StopDisplayInfo();
    char*       GetDisplayInfoName(int index);
    int         GetDisplayInfoPosition(int index);
    void        SetDisplayInfoPosition(int index, int pos);

    void        StartSuspend();
    void        StopSuspend();

    float       GetGameTime();

    void        SetFontSize(float size);
    float       GetFontSize();
    void        SetWindowPos(Math::Point pos);
    Math::Point     GetWindowPos();
    void        SetWindowDim(Math::Point dim);
    Math::Point     GetWindowDim();

    void        SetIOPublic(bool mode);
    bool        GetIOPublic();
    void        SetIOPos(Math::Point pos);
    Math::Point     GetIOPos();
    void        SetIODim(Math::Point dim);
    Math::Point     GetIODim();

    char*       GetTitle();
    char*       GetResume();
    char*       GetScriptName();
    char*       GetScriptFile();
    bool        GetTrainerPilot();
    bool        GetFixScene();
    bool        GetGlint();
    bool        GetSoluce4();
    bool        GetMovies();
    bool        GetNiceReset();
    bool        GetHimselfDamage();
    bool        GetShowSoluce();
    bool        GetSceneSoluce();
    bool        GetShowAll();
    bool        GetRadar();
    const char* GetSavegameDir();
    const char* GetPublicDir();
    const char* GetFilesDir();
    MissionType GetMissionType();

    void        SetGamerName(const char *name);
    char*       GetGamerName();
    int         GetGamerFace();
    int         GetGamerGlasses();
    bool        GetGamerOnlyHead();
    float       GetPersoAngle();
    char*       GetSceneName();
    int         GetSceneRank();

    void        StartMusic();
    void        StartPauseMusic(PauseType pause);
    void        ClearInterface();
    void        ChangeColor();

    float       SearchNearestObject(Math::Vector center, CObject *exclu);
    bool        FreeSpace(Math::Vector &center, float minRadius, float maxRadius, float space, CObject *exclu);
    float       GetFlatZoneRadius(Math::Vector center, float maxRadius, CObject *exclu);
    void        HideDropZone(CObject* metal);
    void        ShowDropZone(CObject* metal, CObject* transporter);
    void        FlushShowLimit(int i);
    void        SetShowLimit(int i, Gfx::ParticleType parti, CObject *pObj, Math::Vector pos,
                             float radius, float duration=SHOWLIMITTIME);
    void        AdjustShowLimit(int i, Math::Vector pos);
    void        StartShowLimit();
    void        FrameShowLimit(float rTime);

    void        CompileScript(bool soluce);
    void        LoadOneScript(CObject *pObj, int &nerror);
    void        LoadFileScript(CObject *pObj, const char* filename, int objRank, int &nerror);
    void        SaveAllScript();
    void        SaveOneScript(CObject *pObj);
    void        SaveFileScript(CObject *pObj, const char* filename, int objRank);
    bool        SaveFileStack(CObject *pObj, FILE *file, int objRank);
    bool        ReadFileStack(CObject *pObj, FILE *file, int objRank);

    bool        FlushNewScriptName();
    bool        AddNewScriptName(ObjectType type, char *name);
    char*       GetNewScriptName(ObjectType type, int rank);

    void        WriteFreeParam();
    void        ReadFreeParam();

    bool        IsBusy();
    bool        IOWriteScene(const char *filename, const char *filecbot, char *info);
    CObject*    IOReadScene(const char *filename, const char *filecbot);
    void        IOWriteObject(CLevelParserLine *line, CObject* obj);
    CObject*    IOReadObject(CLevelParserLine *line, const char* filename, int objRank);

    int         CreateSpot(Math::Vector pos, Gfx::Color color);

    CObject*    GetSelect();

    void        DisplayError(Error err, CObject* pObj, float time=10.0f);
    void        DisplayError(Error err, Math::Vector goal, float height=15.0f, float dist=60.0f, float time=10.0f);

    std::string& GetUserLevelName(int id);

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

protected:
    bool        EventFrame(const Event &event);
    bool        EventObject(const Event &event);
    void        InitEye();

    void        CreateScene(bool soluce, bool fixScene, bool resetObject);

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
    bool        IsSelectable(CObject* pObj);
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
    bool        TestGadgetQuantity(int rank);
    void        UpdateSpeedLabel();

    int         AutosaveRotate(bool freeOne);
    void        Autosave();


protected:
    CController*        m_ctrl;
    CApplication*       m_app;
    CObjectManager*     m_objMan;
    CEventQueue*        m_eventQueue;
    CMainMovie*         m_movie;
    Gfx::CEngine*       m_engine;
    Gfx::CParticle*     m_particle;
    Gfx::CWater*        m_water;
    Gfx::CCloud*        m_cloud;
    Gfx::CLightning*    m_lightning;
    Gfx::CPlanet*       m_planet;
    Gfx::COldModelManager* m_modelManager;
    Gfx::CLightManager* m_lightMan;
    Gfx::CTerrain*      m_terrain;
    Gfx::CCamera*       m_camera;
    Ui::CMainDialog*    m_dialog;
    Ui::CMainShort*     m_short;
    Ui::CMainMap*       m_map;
    Ui::CInterface*     m_interface;
    Ui::CDisplayText*   m_displayText;
    Ui::CDisplayInfo*   m_displayInfo;
    CSoundInterface*    m_sound;
    CPauseManager*      m_pause;
    CInput*             m_input;


    //! Time since level start, including pause and intro movie
    float           m_time;
    //! Playing time since level start
    float           m_gameTime;
    //! Playing time since level start, not dependent on simulation speed
    float           m_gameTimeAbsolute;
    
    float           m_winDelay;
    float           m_lostDelay;
    bool            m_fixScene;        // scene fixed, no interraction
    CObject*        m_base;        // OBJECT_BASE exists in mission
    Math::Point     m_lastMousePos;
    CObject*        m_selectObject;

    Phase           m_phase;
    int             m_cameraRank;
    Gfx::Color      m_color;
    bool            m_freePhoto;
    bool            m_cmdEdit;
    bool            m_selectInsect;
    bool            m_showSoluce;
    bool            m_showAll;
    bool            m_cheatRadar;
    bool            m_shortCut;
    std::string     m_audioTrack;
    bool            m_audioRepeat;
    std::string     m_satcomTrack;
    bool            m_satcomRepeat;
    std::string     m_editorTrack;
    bool            m_editorRepeat;
    int             m_delayWriteMessage;
    int             m_movieInfoIndex;

    CObject*        m_controller;

    MissionType     m_missionType;
    bool            m_immediatSatCom;  // SatCom immediately?
    bool            m_beginSatCom;     // messages SatCom poster?
    bool            m_lockedSatCom;    // SatCom locked?
    bool            m_movieLock;       // movie in progress?
    bool            m_satComLock;      // call of SatCom is possible?
    bool            m_editLock;        // edition in progress?
    bool            m_editFull;        // edition in full screen?
    bool            m_hilite;
    bool            m_trainerPilot;    // remote trainer?
    bool            m_suspend;
    bool            m_friendAim;
    bool            m_resetCreate;
    bool            m_mapShow;
    bool            m_mapImage;
    char            m_mapFilename[100];

    Math::Point     m_tooltipPos;
    std::string     m_tooltipName;
    float           m_tooltipTime;

    char            m_infoFilename[SATCOM_MAX][100]; // names of text files
    CObject*        m_infoObject;
    int             m_infoIndex;
    int             m_infoPos[SATCOM_MAX];
    int             m_infoUsed;

    char            m_title[100];
    char            m_resume[500];
    char            m_scriptName[100];
    char            m_scriptFile[100];
    int             m_endingWinRank;
    int             m_endingLostRank;
    bool            m_winTerminate;

    bool            m_exitAfterMission;

    bool            m_codeBattleInit;
    bool            m_codeBattleStarted;

    std::map<int, std::string> m_teamNames;

    float           m_fontSize;
    Math::Point     m_windowPos;
    Math::Point     m_windowDim;

    bool            m_IOPublic;
    Math::Point     m_IOPos;
    Math::Point     m_IODim;

    NewScriptName   m_newScriptName[MAXNEWSCRIPTNAME];

    float           m_cameraPan;
    float           m_cameraZoom;

    EventType       m_visitLast;
    CObject*        m_visitObject;
    CObject*        m_visitArrow;
    float           m_visitTime;
    float           m_visitParticle;
    Math::Vector    m_visitPos;
    Math::Vector    m_visitPosArrow;

    std::vector<std::unique_ptr<CSceneEndCondition>> m_endTake;
    long            m_endTakeResearch;
    float           m_endTakeWinDelay;
    float           m_endTakeLostDelay;

    std::vector<std::unique_ptr<CAudioChangeCondition>> m_audioChange;

    int             m_obligatoryTotal;
    char            m_obligatoryToken[100][20];
    int             m_prohibitedTotal;
    char            m_prohibitedToken[100][20];

    std::string     m_gamerName;

    int             m_freeBuild;        // constructible buildings
    int             m_freeResearch;     // researches possible

    Error           m_missionResult;

    ShowLimit       m_showLimit[MAXSHOWLIMIT];

    Gfx::Color      m_colorRefBot;
    Gfx::Color      m_colorNewBot;
    Gfx::Color      m_colorRefAlien;
    Gfx::Color      m_colorNewAlien;
    Gfx::Color      m_colorRefGreen;
    Gfx::Color      m_colorNewGreen;
    Gfx::Color      m_colorRefWater;
    Gfx::Color      m_colorNewWater;
    float           m_colorShiftWater;

    bool            m_missionTimerEnabled;
    bool            m_missionTimerStarted;
    float           m_missionTimer;

    bool            m_autosave;
    int             m_autosaveInterval;
    int             m_autosaveSlots;
    float           m_autosaveLast;
};

