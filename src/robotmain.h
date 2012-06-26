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

#ifndef _ROBOTMAIN_H_
#define _ROBOTMAIN_H_


#include "d3dengine.h"
#include "struct.h"
#include "object.h"
#include "mainmovie.h"
#include "camera.h"
#include "particule.h"


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


typedef struct
{
    D3DVECTOR   pos;
    float       dist;
    ObjectType  type;
    int         min;        // wins if>
    int         max;        // wins if <
    int         lost;       // lost if <=
    BOOL        bImmediat;
    char        message[100];
}
EndTake;


#define MAXNEWSCRIPTNAME    20

typedef struct
{
    BOOL        bUsed;
    ObjectType  type;
    char        name[40];
}
NewScriptName;


#define MAXSHOWLIMIT    5
#define MAXSHOWPARTI    200
#define SHOWLIMITTIME   20.0f

typedef struct
{
    BOOL            bUsed;
    D3DVECTOR       pos;
    float           radius;
    int             total;
    int             parti[MAXSHOWPARTI];
    CObject*        link;
    float           duration;
    float           time;
}
ShowLimit;


#define SATCOM_HUSTON   0
#define SATCOM_SAT      1
#define SATCOM_OBJECT   2
#define SATCOM_LOADING  3
#define SATCOM_PROG     4
#define SATCOM_SOLUCE   5
#define SATCOM_MAX      6



class CRobotMain
{
public:
    CRobotMain(CInstanceManager* iMan);
    ~CRobotMain();

    void        CreateIni();

    void        ChangePhase(Phase phase);
    BOOL        EventProcess(const Event &event);

    BOOL        CreateShortcuts();
    void        ScenePerso();

    void        SetMovieLock(BOOL bLock);
    BOOL        RetMovieLock();
    BOOL        RetInfoLock();
    void        SetSatComLock(BOOL bLock);
    BOOL        RetSatComLock();
    void        SetEditLock(BOOL bLock, BOOL bEdit);
    BOOL        RetEditLock();
    void        SetEditFull(BOOL bFull);
    BOOL        RetEditFull();
    BOOL        RetFreePhoto();
    void        SetFriendAim(BOOL bFriend);
    BOOL        RetFriendAim();

    void        SetTracePrecision(float factor);
    float       RetTracePrecision();

    void        ChangePause(BOOL bPause);

    void        SetSpeed(float speed);
    float       RetSpeed();

    void        UpdateShortcuts();
    void        SelectHuman();
    CObject*    SearchHuman();
    CObject*    SearchToto();
    CObject*    SearchNearest(D3DVECTOR pos, CObject* pExclu);
    BOOL        SelectObject(CObject* pObj, BOOL bDisplayError=TRUE);
    CObject*    RetSelectObject();
    CObject*    DeselectAll();
    BOOL        DeleteObject();

    void        ResetObject();
    void        ResetCreate();
    Error       CheckEndMission(BOOL bFrame);
    void        CheckEndMessage(char *message);
    int         RetObligatoryToken();
    char*       RetObligatoryToken(int i);
    int         IsObligatoryToken(char *token);
    BOOL        IsProhibitedToken(char *token);
    void        UpdateMap();
    BOOL        RetShowMap();

    MainMovieType RetMainMovie();

    void        FlushDisplayInfo();
    void        StartDisplayInfo(int index, BOOL bMovie);
    void        StartDisplayInfo(char *filename, int index);
    void        StopDisplayInfo();
    char*       RetDisplayInfoName(int index);
    int         RetDisplayInfoPosition(int index);
    void        SetDisplayInfoPosition(int index, int pos);

    void        StartSuspend();
    void        StopSuspend();

    float       RetGameTime();

    void        SetFontSize(float size);
    float       RetFontSize();
    void        SetWindowPos(FPOINT pos);
    FPOINT      RetWindowPos();
    void        SetWindowDim(FPOINT dim);
    FPOINT      RetWindowDim();

    void        SetIOPublic(BOOL bMode);
    BOOL        RetIOPublic();
    void        SetIOPos(FPOINT pos);
    FPOINT      RetIOPos();
    void        SetIODim(FPOINT dim);
    FPOINT      RetIODim();

    char*       RetTitle();
    char*       RetResume();
    char*       RetScriptName();
    char*       RetScriptFile();
    BOOL        RetTrainerPilot();
    BOOL        RetFixScene();
    BOOL        RetGlint();
    BOOL        RetSoluce4();
    BOOL        RetMovies();
    BOOL        RetNiceReset();
    BOOL        RetHimselfDamage();
    BOOL        RetShowSoluce();
    BOOL        RetSceneSoluce();
    BOOL        RetShowAll();
    BOOL        RetCheatRadar();
    char*       RetSavegameDir();
    char*       RetPublicDir();
    char*       RetFilesDir();

    void        SetGamerName(char *name);
    char*       RetGamerName();
    int         RetGamerFace();
    int         RetGamerGlasses();
    BOOL        RetGamerOnlyHead();
    float       RetPersoAngle();

    void        StartMusic();
    void        ClearInterface();
    void        ChangeColor();

    float       SearchNearestObject(D3DVECTOR center, CObject *exclu);
    BOOL        FreeSpace(D3DVECTOR &center, float minRadius, float maxRadius, float space, CObject *exclu);
    float       RetFlatZoneRadius(D3DVECTOR center, float maxRadius, CObject *exclu);
    void        HideDropZone(CObject* metal);
    void        ShowDropZone(CObject* metal, CObject* truck);
    void        FlushShowLimit(int i);
    void        SetShowLimit(int i, ParticuleType parti, CObject *pObj, D3DVECTOR pos, float radius, float duration=SHOWLIMITTIME);
    void        AdjustShowLimit(int i, D3DVECTOR pos);
    void        StartShowLimit();
    void        FrameShowLimit(float rTime);

    void        CompileScript(BOOL bSoluce);
    void        LoadOneScript(CObject *pObj, int &nbError);
    void        LoadFileScript(CObject *pObj, char* filename, int objRank, int &nbError);
    void        SaveAllScript();
    void        SaveOneScript(CObject *pObj);
    void        SaveFileScript(CObject *pObj, char* filename, int objRank);
    BOOL        SaveFileStack(CObject *pObj, FILE *file, int objRank);
    BOOL        ReadFileStack(CObject *pObj, FILE *file, int objRank);

    BOOL        FlushNewScriptName();
    BOOL        AddNewScriptName(ObjectType type, char *name);
    char*       RetNewScriptName(ObjectType type, int rank);

    void        WriteFreeParam();
    void        ReadFreeParam();

    BOOL        IsBusy();
    BOOL        IOWriteScene(char *filename, char *filecbot, char *info);
    CObject*    IOReadScene(char *filename, char *filecbot);
    void        IOWriteObject(FILE *file, CObject* pObj, char *cmd);
    CObject*    IOReadObject(char *line, char* filename, int objRank);

    int         CreateSpot(D3DVECTOR pos, D3DCOLORVALUE color);

protected:
    BOOL        EventFrame(const Event &event);
    BOOL        EventObject(const Event &event);
    void        InitEye();

    void        Convert();
    void        CreateScene(BOOL bSoluce, BOOL bFixScene, BOOL bResetObject);

    void        CreateModel();
    D3DVECTOR   LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length );
    CObject*    CreateObject(D3DVECTOR pos, float angle, float zoom, float height, ObjectType type, float power=1.0f, BOOL bTrainer=FALSE, BOOL bToy=FALSE, int option=0);
    int         CreateLight(D3DVECTOR direction, D3DCOLORVALUE color);
    void        HiliteClear();
    void        HiliteObject(FPOINT pos);
    void        HiliteFrame(float rTime);
    void        CreateTooltip(FPOINT pos, char* text);
    void        ClearTooltip();
    CObject*    DetectObject(FPOINT pos);
    void        ChangeCamera();
    void        RemoteCamera(float pan, float zoom, float rTime);
    void        KeyCamera(EventMsg event, long param);
    void        AbortMovie();
    BOOL        IsSelectable(CObject* pObj);
    void        SelectOneObject(CObject* pObj, BOOL bDisplayError=TRUE);
    void        HelpObject();
    BOOL        DeselectObject();
    void        DeleteAllObjects();
    void        UpdateInfoText();
    CObject*    SearchObject(ObjectType type);
    CObject*    RetSelect();
    void        StartDisplayVisit(EventMsg event);
    void        FrameVisit(float rTime);
    void        StopDisplayVisit();
    void        ExecuteCmd(char *cmd);
    BOOL        TestGadgetQuantity(int rank);

protected:
    CInstanceManager* m_iMan;
    CMainMovie*     m_movie;
    CMainDialog*    m_dialog;
    CMainShort*     m_short;
    CMainMap*       m_map;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CParticule*     m_particule;
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
    BOOL            m_bFixScene;        // scene fixed, no interraction
    BOOL            m_bBase;        // OBJECT_BASE exists in mission
    FPOINT          m_lastMousePos;
    CObject*        m_selectObject;

    Phase           m_phase;
    int             m_cameraRank;
    D3DCOLORVALUE   m_color;
    BOOL            m_bFreePhoto;
    BOOL            m_bCmdEdit;
    BOOL            m_bShowPos;
    BOOL            m_bSelectInsect;
    BOOL            m_bShowSoluce;
    BOOL            m_bShowAll;
    BOOL            m_bCheatRadar;
    BOOL            m_bAudioRepeat;
    BOOL            m_bShortCut;
    int             m_audioTrack;
    int             m_delayWriteMessage;
    int             m_movieInfoIndex;

    BOOL            m_bImmediatSatCom;  // SatCom immediately?
    BOOL            m_bBeginSatCom;     // messages SatCom poster?
    BOOL            m_bMovieLock;       // movie in progress?
    BOOL            m_bSatComLock;      // call of SatCom is possible?
    BOOL            m_bEditLock;        // edition in progress?
    BOOL            m_bEditFull;        // edition in full screen?
    BOOL            m_bPause;       // simulation paused
    BOOL            m_bHilite;
    BOOL            m_bTrainerPilot;    // remote trainer?
    BOOL            m_bSuspend;
    BOOL            m_bFriendAim;
    BOOL            m_bResetCreate;
    BOOL            m_bMapShow;
    BOOL            m_bMapImage;
    char            m_mapFilename[100];

    FPOINT          m_tooltipPos;
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
    BOOL            m_bWinTerminate;

    float           m_fontSize;
    FPOINT          m_windowPos;
    FPOINT          m_windowDim;

    BOOL            m_IOPublic;
    FPOINT          m_IOPos;
    FPOINT          m_IODim;

    NewScriptName   m_newScriptName[MAXNEWSCRIPTNAME];

    float           m_cameraPan;
    float           m_cameraZoom;

    EventMsg        m_visitLast;
    CObject*        m_visitObject;
    CObject*        m_visitArrow;
    float           m_visitTime;
    float           m_visitParticule;
    D3DVECTOR       m_visitPos;
    D3DVECTOR       m_visitPosArrow;

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

    D3DCOLORVALUE   m_colorRefBot;
    D3DCOLORVALUE   m_colorNewBot;
    D3DCOLORVALUE   m_colorRefAlien;
    D3DCOLORVALUE   m_colorNewAlien;
    D3DCOLORVALUE   m_colorRefGreen;
    D3DCOLORVALUE   m_colorNewGreen;
    D3DCOLORVALUE   m_colorRefWater;
    D3DCOLORVALUE   m_colorNewWater;
    float           m_colorShiftWater;
};


#endif //_ROBOTMAIN_H_
