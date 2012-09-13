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

// maindialog.h

#pragma once


#include "common/struct.h"
#include "object/robotmain.h"


class CInstanceManager;
class CEvent;
class CD3DEngine;
class CInterface;
class CWindow;
class CControl;
class CParticule;
class CSound;


const int USERLISTMAX = 100;
const int MAXSCENE = 1000;

struct SceneInfo
{
    char    numTry;
    char    bPassed;
};

struct GamerPerso
{
    int     face;           // face
    int     glasses;        // glasses
    D3DCOLORVALUE   colorHair;      // hair color
    D3DCOLORVALUE   colorCombi;     // spacesuit volor
    D3DCOLORVALUE   colorBand;      // strips color
};



class CMainDialog
{
public:
    CMainDialog(CInstanceManager* iMan);
    ~CMainDialog();

    bool    EventProcess(const Event &event);
    void    ChangePhase(Phase phase);

    void    SetSceneRead(char* name);
    void    SetStackRead(char* name);
    void    SetSceneName(char* name);
    void    SetSceneRank(int rank);
    char*   RetSceneRead();
    char*   RetStackRead();
    char*   RetSceneName();
    int     RetSceneRank();
    char*   RetSceneDir();
    bool    RetSceneSoluce();
    char*   RetSavegameDir();
    char*   RetPublicDir();

    bool    RetTooltip();
    bool    RetGlint();
    bool    RetSoluce4();
    bool    RetMovies();
    bool    RetNiceReset();
    bool    RetHimselfDamage();

    void    SetUserDir(char *base, int rank);
    void    BuildSceneName(char *filename, char *base, int rank);
    void    BuildResumeName(char *filename, char *base, int rank);
    char*   RetFilesDir();

    void    StartAbort();
    void    StartDeleteObject();
    void    StartDeleteGame(char *gamer);
    void    StartQuit();
    void    StartDialog(Math::Point dim, bool bFire, bool bOK, bool bCancel);
    void    FrameDialog(float rTime);
    void    StopDialog();
    bool    IsDialog();

    void    StartSuspend();
    void    StopSuspend();

    void    SetupMemorize();
    void    SetupRecall();

    bool    ReadGamerInfo();
    bool    WriteGamerInfo();
    void    SetGamerInfoTry(int rank, int numTry);
    int     RetGamerInfoTry(int rank);
    void    SetGamerInfoPassed(int rank, bool bPassed);
    bool    RetGamerInfoPassed(int rank);
    bool    NextMission();

    void    WriteGamerPerso(char *gamer);
    void    ReadGamerPerso(char *gamer);
    void    SetGamerFace(char *gamer, int face);
    int     RetGamerFace(char *gamer);
    int     RetGamerFace();
    int     RetGamerGlasses();
    bool    RetGamerOnlyHead();
    float   RetPersoAngle();
    D3DCOLORVALUE RetGamerColorHair();
    D3DCOLORVALUE RetGamerColorCombi();
    D3DCOLORVALUE RetGamerColorBand();

    void    AllMissionUpdate();
    void    ShowSoluceUpdate();

protected:
    void    GlintMove();
    void    FrameParticule(float rTime);
    void    NiceParticule(Math::Point mouse, bool bPress);
    void    ReadNameList();
    void    UpdateNameList();
    void    UpdateNameEdit();
    void    UpdateNameControl();
    void    UpdateNameFace();
    void    NameSelect();
    void    NameCreate();
    void    NameDelete();
    void    UpdatePerso();
    void    CameraPerso();
    void    FixPerso(int rank, int index);
    void    ColorPerso();
    void    DefPerso();
    bool    IsIOReadScene();
    void    IOReadName();
    void    IOReadList();
    void    IOUpdateList();
    void    IODeleteScene();
    bool    IOWriteScene();
    bool    IOReadScene();
    int     RetChapPassed();
    void    UpdateSceneChap(int &chap);
    void    UpdateSceneList(int chap, int &sel);
    void    UpdateSceneResume(int rank);
    void    UpdateDisplayDevice();
    void    UpdateDisplayMode();
    void    ChangeDisplay();
    void    UpdateApply();
    void    UpdateSetupButtons();
    void    ChangeSetupButtons();
    void    ChangeSetupQuality(int quality);
    void    UpdateKey();
    void    ChangeKey(EventMsg event);

protected:
    CInstanceManager* m_iMan;
    CRobotMain*     m_main;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CInterface*     m_interface;
    CParticule*     m_particule;
    CCamera*        m_camera;
    CSound*         m_sound;

    Phase           m_phase;            // copy of CRobotMain
    Phase           m_phaseSetup;           // tab selected
    Phase           m_phaseTerm;            // phase trainer/scene/proto
    float           m_phaseTime;

    GamerPerso      m_perso;            // perso: description
    GamerPerso      m_persoCopy;            // perso: copy for cancellation
    int         m_persoTab;         // perso: tab selected
    float           m_persoAngle;           // perso: angle of presentation

    char            m_sceneDir[_MAX_FNAME];     // scene folder
    char            m_savegameDir[_MAX_FNAME];  // savegame folder
    char            m_publicDir[_MAX_FNAME];    // program folder
    char            m_userDir[_MAX_FNAME];      // user folder
    char            m_filesDir[_MAX_FNAME];     // case files

    int             m_index;        // 0..4
    int             m_chap[10];     // selected chapter (0..8)
    int             m_sel[10];      // chosen mission (0..98)
    int             m_maxList;
    int             m_accessChap;
    char            m_sceneRead[100];       // name of the scene to read
    char            m_stackRead[100];       // name of the scene to read
    char            m_sceneName[20];        // name of the scene to play
    int             m_sceneRank;        // rank of the scene to play
    bool            m_bSceneSoluce;         // shows the solution
    bool            m_bSimulSetup;          // adjustment during the game
    bool            m_accessEnable;
    bool            m_accessMission;
    bool            m_accessUser;
    bool            m_bDeleteGamer;

    int             m_userTotal;
    char            m_userList[USERLISTMAX][100];

    int             m_shotDelay;        // number of frames before copy
    char            m_shotName[100];        // generate a file name

    int             m_setupSelDevice;
    int             m_setupSelMode;
    bool            m_setupFull;

    bool            m_bTooltip;         // tooltips to be displayed?
    bool            m_bGlint;           // reflections on buttons?
    bool            m_bRain;            // rain in the interface?
    bool            m_bSoluce4;         // solutions in program 4?
    bool            m_bMovies;          // cinematics?
    bool            m_bNiceReset;           // for CTaskReset
    bool            m_bHimselfDamage;       // for shots
    bool            m_bCameraScroll;        // for CCamera
    bool            m_bCameraInvertX;       // for CCamera
    bool            m_bCameraInvertY;       // for CCamera
    bool            m_bEffect;          // for CCamera

    Math::Point         m_glintMouse;
    float           m_glintTime;

    int             m_loadingCounter;

    bool            m_bDialog;          // this dialogue?
    bool            m_bDialogFire;          // setting on fire?
    bool            m_bDialogDelete;
    Math::Point         m_dialogPos;
    Math::Point         m_dialogDim;
    float           m_dialogParti;
    float           m_dialogTime;
    bool            m_bInitPause;
    CameraType      m_initCamera;

    int             m_partiPhase[10];
    float           m_partiTime[10];
    Math::Point         m_partiPos[10];

    SceneInfo       m_sceneInfo[MAXSCENE];
};


