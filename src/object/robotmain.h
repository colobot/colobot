// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file object/robotmain.h
 * \brief CRobotMain - main class of Colobot game engine
 */

#pragma once


#include "common/global.h"
#include "common/singleton.h"

#include "graphics/engine/particle.h"

#include "object/object.h"
#include "object/mainmovie.h"

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
    PHASE_TEEN,
    PHASE_USER,
    PHASE_PROTO,
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


class CEventQueue;
class CSoundInterface;

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


struct EndTake
{
    Math::Vector  pos;
    float         dist;
    ObjectType    type;
    int           min;        // wins if >
    int           max;        // wins if <
    int           lost;       // lost if <=
    float         powermin;   // wins if energy cell >=
    float         powermax;   // wins if energy cell <=
    bool          immediat;
    char          message[100];
};

struct AudioChange
{
    Math::Vector  pos;
    float         dist;
    ObjectType    type;
    int           min;        // change if >
    int           max;        // change if <
    float         powermin;   // change if energy cell >=
    float         powermax;   // change if energy cell <=
    char          music[100];
    bool          repeat;
    bool          changed;
};


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


/**
 * \struct InputBinding
 * \brief Binding for input slot
 */
struct InputBinding
{
    //! Primary and secondary bindings
    //! Can be regular key, virtual key or virtual joystick button
    unsigned int primary, secondary;

    InputBinding(unsigned int p = KEY_INVALID, unsigned int s = KEY_INVALID)
        : primary(p), secondary(s) {}
};

/**
 * \struct JoyAxisBinding
 * \brief Binding for joystick axis
 */
struct JoyAxisBinding
{
    //! Axis index or AXIS_INVALID
    int axis;
    //! True to invert axis value
    bool invert;
};

//! Invalid value for axis binding (no axis assigned)
const int AXIS_INVALID = -1;

class CRobotMain : public CSingleton<CRobotMain>
{
public:
    CRobotMain(CApplication* app);
    ~CRobotMain();

    Gfx::CCamera* GetCamera();
    Gfx::CTerrain* GetTerrain();
    Ui::CInterface* GetInterface();
    Ui::CDisplayText* GetDisplayText();

    void        CreateIni();

    //! Sets the default input bindings (key and axes)
    void        SetDefaultInputBindings();

    //! Management of input bindings
    //@{
    void        SetInputBinding(InputSlot slot, InputBinding binding);
    const InputBinding& GetInputBinding(InputSlot slot);
    //@}

    //! Management of joystick axis bindings
    //@{
    void        SetJoyAxisBinding(JoyAxisSlot slot, JoyAxisBinding binding);
    const JoyAxisBinding& GetJoyAxisBinding(JoyAxisSlot slot);
    //@}

    //! Management of joystick deadzone
    //@{
    void        SetJoystickDeadzone(float zone);
    float       GetJoystickDeadzone();
    //@}

    //! Resets tracked key states (motion vectors)
    void        ResetKeyStates();

    void        ChangePhase(Phase phase);
    bool        EventProcess(Event &event);

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

    void        ChangePause(bool pause);

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
    Error       CheckEndMission(bool frame);
    void        CheckEndMessage(const char* message);
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
    bool        GetRetroMode();

    void        SetGamerName(const char *name);
    char*       GetGamerName();
    int         GetGamerFace();
    int         GetGamerGlasses();
    bool        GetGamerOnlyHead();
    float       GetPersoAngle();

    void        StartMusic();
    void        ClearInterface();
    void        ChangeColor();

    float       SearchNearestObject(Math::Vector center, CObject *exclu);
    bool        FreeSpace(Math::Vector &center, float minRadius, float maxRadius, float space, CObject *exclu);
    float       GetFlatZoneRadius(Math::Vector center, float maxRadius, CObject *exclu);
    void        HideDropZone(CObject* metal);
    void        ShowDropZone(CObject* metal, CObject* truck);
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
    void        IOWriteObject(FILE *file, CObject* pObj, const char *cmd);
    CObject*    IOReadObject(char *line, const char* filename, int objRank);

    int         CreateSpot(Math::Vector pos, Gfx::Color color);
    
    void        SetNumericLocale();
    void        RestoreNumericLocale();

protected:
    bool        EventFrame(const Event &event);
    bool        EventObject(const Event &event);
    void        InitEye();

    void        Convert();
    void        CreateScene(bool soluce, bool fixScene, bool resetObject);

    Math::Vector LookatPoint(Math::Vector eye, float angleH, float angleV, float length);
    CObject*    CreateObject(Math::Vector pos, float angle, float zoom,
                             float height, ObjectType type, float power=1.0f,
                             bool trainer=false, bool toy=false, int option=0);
    int         CreateLight(Math::Vector direction, Gfx::Color color);
    void        HiliteClear();
    void        HiliteObject(Math::Point pos);
    void        HiliteFrame(float rTime);
    void        CreateTooltip(Math::Point pos, const char* text);
    void        ClearTooltip();
    CObject*    DetectObject(Math::Point pos);
    void        ChangeCamera();
    void        RemoteCamera(float pan, float zoom, float rTime);
    void        KeyCamera(EventType event, unsigned int key);
    void        AbortMovie();
    bool        IsSelectable(CObject* pObj);
    void        SelectOneObject(CObject* pObj, bool displayError=true);
    void        HelpObject();
    bool        DeselectObject();
    void        DeleteAllObjects();
    void        UpdateInfoText();
    CObject*    SearchObject(ObjectType type);
    CObject*    GetSelect();
    void        StartDisplayVisit(EventType event);
    void        FrameVisit(float rTime);
    void        StopDisplayVisit();
    void        ExecuteCmd(char *cmd);
    bool        TestGadgetQuantity(int rank);
    void        UpdateSpeedLabel();
    

protected:
    CApplication*       m_app;
    CEventQueue*        m_eventQueue;
    CMainMovie*         m_movie;
    Gfx::CEngine*       m_engine;
    Gfx::CParticle*     m_particle;
    Gfx::CWater*        m_water;
    Gfx::CCloud*        m_cloud;
    Gfx::CLightning*    m_lightning;
    Gfx::CPlanet*       m_planet;
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

    //! Bindings for user inputs
    InputBinding    m_inputBindings[INPUT_SLOT_MAX];
    JoyAxisBinding  m_joyAxisBindings[JOY_AXIS_SLOT_MAX];
    float           m_joystickDeadzone;
    //! Motion vector set by keyboard or joystick buttons
    Math::Vector    m_keyMotion;
    //! Motion vector set by joystick axes
    Math::Vector    m_joyMotion;


    float           m_time;
    float           m_gameTime;
    float           m_checkEndTime;
    float           m_winDelay;
    float           m_lostDelay;
    bool            m_fixScene;        // scene fixed, no interraction
    bool            m_base;        // OBJECT_BASE exists in mission
    Math::Point     m_lastMousePos;
    CObject*        m_selectObject;

    Phase           m_phase;
    int             m_cameraRank;
    Gfx::Color      m_color;
    bool            m_freePhoto;
    bool            m_cmdEdit;
    bool            m_showPos;
    bool            m_selectInsect;
    bool            m_showSoluce;
    bool            m_showAll;
    bool            m_cheatRadar;
    bool            m_audioRepeat;
    bool            m_shortCut;
    std::string     m_audioTrack;
    int             m_delayWriteMessage;
    int             m_movieInfoIndex;

    //Level Checker flags
    bool            m_beginObject;
    bool            m_terrainGenerate;
    bool            m_terrainInitTextures;
    bool            m_terrainInit;
    bool            m_terrainCreate;

    int             m_version;         // Mission file version
    bool            m_retroStyle;      // Retro
    bool            m_immediatSatCom;  // SatCom immediately?
    bool            m_beginSatCom;     // messages SatCom poster?
    bool            m_lockedSatCom;    // SatCom locked?
    bool            m_movieLock;       // movie in progress?
    bool            m_satComLock;      // call of SatCom is possible?
    bool            m_editLock;        // edition in progress?
    bool            m_editFull;        // edition in full screen?
    bool            m_pause;       // simulation paused
    bool            m_hilite;
    bool            m_trainerPilot;    // remote trainer?
    bool            m_suspend;
    bool            m_friendAim;
    bool            m_resetCreate;
    bool            m_mapShow;
    bool            m_mapImage;
    char            m_mapFilename[100];

    Math::Point     m_tooltipPos;
    char            m_tooltipName[100];
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

    int             m_endTakeTotal;
    EndTake         m_endTake[10];
    long            m_endTakeResearch;
    float           m_endTakeWinDelay;
    float           m_endTakeLostDelay;
    
    int             m_audioChangeTotal;
    AudioChange     m_audioChange[10];

    int             m_obligatoryTotal;
    char            m_obligatoryToken[100][20];
    int             m_prohibitedTotal;
    char            m_prohibitedToken[100][20];

    char            m_gamerName[100];

    int             m_freeBuild;        // constructible buildings
    int             m_freeResearch;     // researches possible

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
    
    std::string     m_oldLocale;
};

