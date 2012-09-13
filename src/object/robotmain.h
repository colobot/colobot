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

// robotmain.h

#pragma once


#include <stdio.h>

#include "common/misc.h"
//#include "old/d3dengine.h"
#include "graphics/engine/engine.h"
#include "object/object.h"
#include "object/mainmovie.h"
//#include "old/camera.h"
#include "graphics/engine/camera.h"
//#include "old/particule.h"
#include "graphics/engine/particle.h"

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


class CInstanceManager;
class CMainDialog;
class CMainShort;
class CMainMap;
class CEvent;
class CD3DEngine;
class CLight;
class CWater;
class CCloud;
class CBlitz;
class CPlanet;
class CTerrain;
class CModel;
class CInterface;
class CWindow;
class CControl;
class CDisplayText;
class CDisplayInfo;
class CSound;


struct EndTake
{
    Math::Vector    pos;
    float       dist;
    ObjectType  type;
    int         min;        // wins if>
    int         max;        // wins if <
    int         lost;       // lost if <=
    bool        bImmediat;
    char        message[100];
};


const int MAXNEWSCRIPTNAME = 20;

struct NewScriptName
{
    bool        bUsed;
    ObjectType  type;
    char        name[40];
};


const int MAXSHOWLIMIT      = 5;
const int MAXSHOWPARTI      = 200;
const float SHOWLIMITTIME   = 20.0f;

struct ShowLimit
{
    bool            bUsed;
    Math::Vector        pos;
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



class CRobotMain
{
public:
    CRobotMain(CInstanceManager* iMan);
    ~CRobotMain();

    void        CreateIni();

    void        ChangePhase(Phase phase);
    bool        EventProcess(const Event &event);

    bool        CreateShortcuts();
    void        ScenePerso();

    void        SetMovieLock(bool bLock);
    bool        GetMovieLock();
    bool        GetInfoLock();
    void        SetSatComLock(bool bLock);
    bool        GetSatComLock();
    void        SetEditLock(bool bLock, bool bEdit);
    bool        GetEditLock();
    void        SetEditFull(bool bFull);
    bool        GetEditFull();
    bool        GetFreePhoto();
    void        SetFriendAim(bool bFriend);
    bool        GetFriendAim();

    void        SetTracePrecision(float factor);
    float       GetTracePrecision();

    void        ChangePause(bool bPause);

    void        SetSpeed(float speed);
    float       GetSpeed();

    void        UpdateShortcuts();
    void        SelectHuman();
    CObject*    SearchHuman();
    CObject*    SearchToto();
    CObject*    SearchNearest(Math::Vector pos, CObject* pExclu);
    bool        SelectObject(CObject* pObj, bool bDisplayError=true);
    CObject*    GetSelectObject();
    CObject*    DeselectAll();
    bool        DeleteObject();

    void        ResetObject();
    void        ResetCreate();
    Error       CheckEndMission(bool bFrame);
    void        CheckEndMessage(char *message);
    int         GetObligatoryToken();
    char*       GetObligatoryToken(int i);
    int         IsObligatoryToken(char *token);
    bool        IsProhibitedToken(char *token);
    void        UpdateMap();
    bool        GetShowMap();

    MainMovieType GetMainMovie();

    void        FlushDisplayInfo();
    void        StartDisplayInfo(int index, bool bMovie);
    void        StartDisplayInfo(char *filename, int index);
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

    void        SetIOPublic(bool bMode);
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
    bool        GetCheatRadar();
    char*       GetSavegameDir();
    char*       GetPublicDir();
    char*       GetFilesDir();

    void        SetGamerName(char *name);
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
    void        SetShowLimit(int i, Gfx::ParticleType parti, CObject *pObj, Math::Vector pos, float radius, float duration=SHOWLIMITTIME);
    void        AdjustShowLimit(int i, Math::Vector pos);
    void        StartShowLimit();
    void        FrameShowLimit(float rTime);

    void        CompileScript(bool bSoluce);
    void        LoadOneScript(CObject *pObj, int &nbError);
    void        LoadFileScript(CObject *pObj, char* filename, int objRank, int &nbError);
    void        SaveAllScript();
    void        SaveOneScript(CObject *pObj);
    void        SaveFileScript(CObject *pObj, char* filename, int objRank);
    bool        SaveFileStack(CObject *pObj, FILE *file, int objRank);
    bool        ReadFileStack(CObject *pObj, FILE *file, int objRank);

    bool        FlushNewScriptName();
    bool        AddNewScriptName(ObjectType type, char *name);
    char*       GetNewScriptName(ObjectType type, int rank);

    void        WriteFreeParam();
    void        ReadFreeParam();

    bool        IsBusy();
    bool        IOWriteScene(char *filename, char *filecbot, char *info);
    CObject*    IOReadScene(char *filename, char *filecbot);
    void        IOWriteObject(FILE *file, CObject* pObj, char *cmd);
    CObject*    IOReadObject(char *line, char* filename, int objRank);

    int         CreateSpot(Math::Vector pos, Gfx::Color color);

protected:
    bool        EventFrame(const Event &event);
    bool        EventObject(const Event &event);
    void        InitEye();

    void        Convert();
    void        CreateScene(bool bSoluce, bool bFixScene, bool bResetObject);

    void        CreateModel();
    Math::Vector    LookatPoint( Math::Vector eye, float angleH, float angleV, float length );
    CObject*    CreateObject(Math::Vector pos, float angle, float zoom, float height, ObjectType type, float power=1.0f, bool bTrainer=false, bool bToy=false, int option=0);
    int         CreateLight(Math::Vector direction, Gfx::Color color);
    void        HighlightClear();
    void        HighlightObject(Math::Point pos);
    void        HighlightFrame(float rTime);
    void        CreateTooltip(Math::Point pos, char* text);
    void        ClearTooltip();
    CObject*    DetectObject(Math::Point pos);
    void        ChangeCamera();
    void        RemoteCamera(float pan, float zoom, float rTime);
    void        KeyCamera(EventType event, long param);
    void        AbortMovie();
    bool        IsSelectable(CObject* pObj);
    void        SelectOneObject(CObject* pObj, bool bDisplayError=true);
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

protected:
    CInstanceManager* m_iMan;
    CMainMovie*     m_movie;
    CMainDialog*    m_dialog;
    CMainShort*     m_short;
    CMainMap*       m_map;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    Gfx::CParticle*     m_particule;
    CWater*         m_water;
    CCloud*         m_cloud;
    CBlitz*         m_blitz;
    CPlanet*        m_planet;
    CLight*         m_light;
    CTerrain*       m_terrain;
    CModel*         m_model;
    CInterface*     m_interface;
    CCamera*        m_camera;
    CDisplayText*   m_displayText;
    CDisplayInfo*   m_displayInfo;
    CSound*         m_sound;

    float           m_time;
    float           m_gameTime;
    float           m_checkEndTime;
    float           m_winDelay;
    float           m_lostDelay;
    bool            m_bFixScene;        // scene fixed, no interraction
    bool            m_bBase;        // OBJECT_BASE exists in mission
    Math::Point         m_lastMousePos;
    CObject*        m_selectObject;

    Phase           m_phase;
    int             m_cameraRank;
    Gfx::Color      m_color;
    bool            m_bFreePhoto;
    bool            m_bCmdEdit;
    bool            m_bShowPos;
    bool            m_bSelectInsect;
    bool            m_bShowSoluce;
    bool            m_bShowAll;
    bool            m_bCheatRadar;
    bool            m_bAudioRepeat;
    bool            m_bShortCut;
    int             m_audioTrack;
    int             m_delayWriteMessage;
    int             m_movieInfoIndex;

    bool            m_bImmediatSatCom;  // SatCom immediately?
    bool            m_bBeginSatCom;     // messages SatCom poster?
    bool            m_bMovieLock;       // movie in progress?
    bool            m_bSatComLock;      // call of SatCom is possible?
    bool            m_bEditLock;        // edition in progress?
    bool            m_bEditFull;        // edition in full screen?
    bool            m_bPause;       // simulation paused
    bool            m_bHighlight;
    bool            m_bTrainerPilot;    // remote trainer?
    bool            m_bSuspend;
    bool            m_bFriendAim;
    bool            m_bResetCreate;
    bool            m_bMapShow;
    bool            m_bMapImage;
    char            m_mapFilename[100];

    Math::Point         m_tooltipPos;
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
    bool            m_bWinTerminate;

    float           m_fontSize;
    Math::Point         m_windowPos;
    Math::Point         m_windowDim;

    bool            m_IOPublic;
    Math::Point         m_IOPos;
    Math::Point         m_IODim;

    NewScriptName   m_newScriptName[MAXNEWSCRIPTNAME];

    float           m_cameraPan;
    float           m_cameraZoom;

    EventType        m_visitLast;
    CObject*        m_visitObject;
    CObject*        m_visitArrow;
    float           m_visitTime;
    float           m_visitParticule;
    Math::Vector        m_visitPos;
    Math::Vector        m_visitPosArrow;

    int             m_endTakeTotal;
    EndTake         m_endTake[10];
    long            m_endTakeResearch;
    float           m_endTakeWinDelay;
    float           m_endTakeLostDelay;

    int             m_obligatoryTotal;
    char            m_obligatoryToken[100][20];
    int             m_prohibitedTotal;
    char            m_prohibitedToken[100][20];

    char            m_gamerName[100];

    long            m_freeBuild;        // constructible buildings
    long            m_freeResearch;     // researches possible

    ShowLimit       m_showLimit[MAXSHOWLIMIT];

    Gfx::Color   m_colorRefBot;
    Gfx::Color   m_colorNewBot;
    Gfx::Color   m_colorRefAlien;
    Gfx::Color   m_colorNewAlien;
    Gfx::Color   m_colorRefGreen;
    Gfx::Color   m_colorNewGreen;
    Gfx::Color   m_colorRefWater;
    Gfx::Color   m_colorNewWater;
    float           m_colorShiftWater;
};

