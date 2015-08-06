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

#include "app/pausemanager.h"

#include "graphics/core/color.h"

#include "object/level_category.h"
#include "object/robotmain.h"

#include <vector>


class CEventQueue;
class CSettings;
class CSoundInterface;

namespace Gfx
{
class CEngine;
class CParticle;
}

namespace Ui
{

class CInterface;
class CWindow;
class CControl;

class CScreen;
class CScreenApperance;
class CScreenIORead;
class CScreenIOWrite;
class CScreenLevelList;
class CScreenLoading;
class CScreenMainMenu;
class CScreenPlayerSelect;
class CScreenQuit;
class CScreenSetup;
class CScreenSetupControls;
class CScreenSetupDisplay;
class CScreenSetupGame;
class CScreenSetupGraphics;
class CScreenSetupSound;
class CScreenWelcome;



class CMainDialog
{
public:
    CMainDialog();
    ~CMainDialog();

    void    Create();

    bool    EventProcess(const Event &event);
    void    ChangePhase(Phase phase);

    bool    GetSceneSoluce();

    void    StartAbort();
    void    StartDeleteObject();
    void    StartDeleteGame(char *gamer);
    void    StartQuit();
    void    StartDialog(Math::Point dim, bool bFire, bool bOK, bool bCancel);
    void    FrameDialog(float rTime);
    void    StopDialog();
    bool    IsDialog();

    void    UpdateChapterPassed();
    void    NextMission();

    bool    GetGamerOnlyHead();
    float   GetPersoAngle();

    void    AllMissionUpdate();
    void    ShowSoluceUpdate();

    void    UpdateCustomLevelList();
    std::string GetCustomLevelName(int id);
    const std::vector<std::string>& GetCustomLevelList();

    void    MakeSaveScreenshot(const std::string& name);

protected:
    void    GlintMove();
    void    FrameParticle(float rTime);
    void    NiceParticle(Math::Point mouse, bool bPress);

    CScreenSetup* GetSetupScreen(Phase phase);

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
    CSettings*        m_settings;

    CScreen* m_currentScreen;
    std::unique_ptr<CScreenApperance> m_screenAppearance;
    std::unique_ptr<CScreenIORead> m_screenIORead;
    std::unique_ptr<CScreenIOWrite> m_screenIOWrite;
    std::unique_ptr<CScreenLevelList> m_screenLevelList;
    std::unique_ptr<CScreenLoading> m_screenLoading;
    std::unique_ptr<CScreenMainMenu> m_screenMainMenu;
    std::unique_ptr<CScreenPlayerSelect> m_screenPlayerSelect;
    std::unique_ptr<CScreenQuit> m_screenQuit;
    std::unique_ptr<CScreenSetupControls> m_screenSetupControls;
    std::unique_ptr<CScreenSetupDisplay> m_screenSetupDisplay;
    std::unique_ptr<CScreenSetupGame> m_screenSetupGame;
    std::unique_ptr<CScreenSetupGraphics> m_screenSetupGraphics;
    std::unique_ptr<CScreenSetupSound> m_screenSetupSound;
    std::unique_ptr<CScreenWelcome> m_screenWelcome;

    Phase           m_phase;            // copy of CRobotMain

    int             m_shotDelay;        // number of frames before copy
    std::string     m_shotName;        // generate a file name

    Math::Point          m_glintMouse;
    float                m_glintTime;

    bool                 m_bDialog;          // this dialogue?
    bool                 m_bDialogFire;          // setting on fire?
    bool                 m_bDialogDelete;
    Math::Point          m_dialogPos;
    Math::Point          m_dialogDim;
    float                m_dialogParti;
    float                m_dialogTime;

    int                  m_partiPhase[10];
    float                m_partiTime[10];
    Math::Point          m_partiPos[10];
};

} // namespace Ui
