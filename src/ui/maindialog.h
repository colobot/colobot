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

#include "common/event.h"

#include "object/robotmain.h"

#include <vector>


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

    bool    EventProcess(const Event &event);
    void    ChangePhase(Phase phase);

    void    StartAbort();
    void    StartDeleteObject();
    void    StartDeleteGame(char *gamer);
    void    StartQuit();
    void    StartDialog(Math::Point dim, bool bFire, bool bOK, bool bCancel);
    void    FrameDialog(float rTime);
    void    StopDialog();
    bool    IsDialog();

protected:
    CRobotMain*       m_main;
    Gfx::CEngine*     m_engine;
    Gfx::CParticle*   m_particle;
    CInterface*       m_interface;
    CSoundInterface*  m_sound;
    CSettings*        m_settings;

    Phase                m_phase;            // copy of CRobotMain

    bool                 m_bDialog;          // this dialogue?
    bool                 m_bDialogFire;          // setting on fire?
    bool                 m_bDialogDelete;
    Math::Point          m_dialogPos;
    Math::Point          m_dialogDim;
    float                m_dialogParti;
    float                m_dialogTime;
};

} // namespace Ui
