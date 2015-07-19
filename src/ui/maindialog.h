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
#pragma once


#include "graphics/core/color.h"
#include "graphics/engine/camera.h"

#include "object/level_category.h"
#include "object/robotmain.h"

#include "app/pausemanager.h"

#include <vector>


class CEventQueue;
class CSoundInterface;

namespace Gfx {
class CEngine;
class CParticle;
}

namespace Ui {

class CInterface;
class CWindow;
class CControl;



class CMainDialog
{
public:
    CMainDialog();
    ~CMainDialog();

    void    Create();

    bool    EventProcess(const Event &event);
    void    ChangePhase(Phase phase);

    void          SetSceneRead(const char* name);
    std::string & GetSceneRead();
    void          SetStackRead(const char* name);
    std::string & GetStackRead();

    void          SetLevel(LevelCategory cat, int chap, int rank);
    LevelCategory GetLevelCategory();
    int           GetLevelChap();
    int           GetLevelRank();
    std::string   GetCustomLevelDir();

    bool          GetSceneSoluce();

    bool    GetTooltip();
    bool    GetGlint();
    bool    GetSoluce4();
    bool    GetMovies();
    bool    GetNiceReset();
    bool    GetHimselfDamage();

    void    BuildResumeName(char *filename, std::string base, int chap, int rank);

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

    void    UpdateChapterPassed();
    bool    NextMission();

    bool    GetGamerOnlyHead();
    float   GetPersoAngle();

    void    AllMissionUpdate();
    void    ShowSoluceUpdate();

    void    UpdateCustomLevelList();
    std::string GetCustomLevelName(int id);

    void    MakeSaveScreenshot(const std::string& name);

protected:
    void    GlintMove();
    void    FrameParticle(float rTime);
    void    NiceParticle(Math::Point mouse, bool bPress);
    void    ReadNameList();
    void    UpdateNameList();
    void    UpdateNameEdit();
    void    UpdateNameControl();
    void    NameSelect();
    bool    NameCreate();
    void    NameDelete();
    void    UpdatePerso();
    void    CameraPerso();
    void    FixPerso(int rank, int index);
    void    ColorPerso();
    bool    IsIOReadScene();
    void    IOReadName();
    void    IOReadList();
    void    IOUpdateList();
    void    IODeleteScene();
    bool    IOWriteScene();
    bool    IOReadScene();
    int     GetChapPassed();
    void    UpdateSceneChap(int &chap);
    void    UpdateSceneList(int chap, int &sel);
    void    UpdateSceneResume(int chap, int rank);
    void    UpdateDisplayMode();
    void    ChangeDisplay();
    void    UpdateApply();
    void    UpdateSetupButtons();
    void    ChangeSetupButtons();
    void    ChangeSetupQuality(int quality);
    void    UpdateKey();
    void    ChangeKey(EventType event);

protected:
    CApplication*     m_app;
    CRobotMain*       m_main;
    CEventQueue*      m_eventQueue;
    Gfx::CEngine*     m_engine;
    CInterface*       m_interface;
    Gfx::CParticle*   m_particle;
    Gfx::CCamera*     m_camera;
    CSoundInterface*  m_sound;
    CPauseManager*    m_pause;

    Phase           m_phase;            // copy of CRobotMain
    Phase           m_phaseSetup;           // tab selected
    float           m_phaseTime;

    int             m_apperanceTab;         // perso: tab selected
    float           m_apperanceAngle;           // perso: angle of presentation

    LevelCategory   m_category;     // 0..4
    std::map<LevelCategory, int> m_chap;     // selected chapter (0..8)
    std::map<LevelCategory, int> m_sel;      // chosen mission (0..98)
    int             m_maxList;
    int             m_accessChap;
    std::string     m_sceneRead;       // name of the scene to read
    std::string     m_stackRead;       // name of the scene to read
    int             m_levelChap;       // chapter of the level to play
    int             m_levelRank;       // rank of the level to play
    bool            m_bSceneSoluce;         // shows the solution
    bool            m_bSimulSetup;          // adjustment during the game
    bool            m_accessEnable;
    bool            m_accessMission;
    bool            m_accessUser;
    bool            m_bDeleteGamer;

    std::vector<std::string> m_customLevelList;

    int             m_shotDelay;        // number of frames before copy
    std::string     m_shotName;        // generate a file name

    int             m_setupSelMode;
    bool            m_setupFull;

    bool            m_bTooltip;         // tooltips to be displayed?
    bool            m_bGlint;           // reflections on buttons?
    bool            m_bRain;            // rain in the interface?
    bool            m_bSoluce4;         // solutions in program 4?
    bool            m_bMovies;          // cinematics?
    bool            m_bNiceReset;           // for reset animation
    bool            m_bHimselfDamage;       // for shots
    bool            m_bCameraScroll;        // for CCamera
    bool            m_bCameraInvertX;       // for CCamera
    bool            m_bCameraInvertY;       // for CCamera
    bool            m_bEffect;          // for CCamera
    bool            m_bBlood;          // for CCamera
    bool            m_bAutosave;

    Math::Point          m_glintMouse;
    float                m_glintTime;

    int                  m_loadingCounter;

    bool                 m_bDialog;          // this dialogue?
    bool                 m_bDialogFire;          // setting on fire?
    bool                 m_bDialogDelete;
    Math::Point          m_dialogPos;
    Math::Point          m_dialogDim;
    float                m_dialogParti;
    float                m_dialogTime;
    PauseType            m_bInitPause;
    Gfx::CameraType      m_initCamera;

    int                  m_partiPhase[10];
    float                m_partiTime[10];
    Math::Point          m_partiPos[10];

    std::vector<std::string> m_saveList;
};

} // namespace Ui
