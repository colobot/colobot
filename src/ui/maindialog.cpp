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


#include "ui/maindialog.h"

#include "common/config.h"

#include "app/app.h"
#include "app/input.h"
#include "app/system.h"

#include "common/config_file.h"
#include "common/event.h"
#include "common/global.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/misc.h"
#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "common/resources/inputstream.h"
#include "common/resources/outputstream.h"
#include "common/resources/resourcemanager.h"

#include "object/player_profile.h"
#include "object/robotmain.h"

#include "object/level/parser.h"

#include "sound/sound.h"

#include "ui/screen/screen.h"
#include "ui/screen/screen_apperance.h"
#include "ui/screen/screen_io_read.h"
#include "ui/screen/screen_io_write.h"
#include "ui/screen/screen_level_list.h"
#include "ui/screen/screen_loading.h"
#include "ui/screen/screen_main_menu.h"
#include "ui/screen/screen_player_select.h"
#include "ui/screen/screen_setup.h"
#include "ui/screen/screen_quit.h"
#include "ui/screen/screen_welcome.h"

#include "ui/button.h"
#include "ui/check.h"
#include "ui/color.h"
#include "ui/edit.h"
#include "ui/editvalue.h"
#include "ui/group.h"
#include "ui/image.h"
#include "ui/interface.h"
#include "ui/key.h"
#include "ui/label.h"
#include "ui/list.h"
#include "ui/scroll.h"
#include "ui/slider.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

//TODO Get rid of all sprintf's

namespace Ui
{

// Constructor of robot application.

CMainDialog::CMainDialog()
{
    m_app        = nullptr;
    m_eventQueue = nullptr;
    m_sound      = nullptr;
    m_main       = nullptr;
    m_interface  = nullptr;
    m_camera     = nullptr;
    m_engine     = nullptr;
    m_particle   = nullptr;
    m_pause      = nullptr;

    m_currentScreen = nullptr;

    m_phase        = PHASE_PLAYER_SELECT;

    m_shotDelay      = 0;

    m_glintMouse = Math::Point(0.0f, 0.0f);
    m_glintTime  = 1000.0f;

    for (int i = 0; i < 10; i++)
    {
        m_partiPhase[i] = 0;
        m_partiTime[i]  = 0.0f;
    }

    m_bDialog = false;
}

void CMainDialog::Create()
{
    m_app        = CApplication::GetInstancePointer();
    m_eventQueue = m_app->GetEventQueue();
    m_sound      = m_app->GetSound();
    m_main       = CRobotMain::GetInstancePointer();
    m_interface  = m_main->GetInterface();
    m_camera     = m_main->GetCamera();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_particle   = m_engine->GetParticle();
    m_pause      = CPauseManager::GetInstancePointer();
    m_settings   = CSettings::GetInstancePointer();

    m_screenAppearance = MakeUnique<CScreenApperance>();
    m_screenLevelList = MakeUnique<CScreenLevelList>();
    m_screenIORead = MakeUnique<CScreenIORead>(m_screenLevelList.get());
    m_screenIOWrite = MakeUnique<CScreenIOWrite>(m_screenLevelList.get());
    m_screenLoading = MakeUnique<CScreenLoading>();
    m_screenSetup = MakeUnique<CScreenSetup>();
    m_screenMainMenu = MakeUnique<CScreenMainMenu>(m_screenSetup.get());
    m_screenPlayerSelect = MakeUnique<CScreenPlayerSelect>(this);
    m_screenQuit = MakeUnique<CScreenQuit>();
    m_screenWelcome = MakeUnique<CScreenWelcome>();
}

// Destructor of robot application.

CMainDialog::~CMainDialog()
{
}


// Changes phase.

void CMainDialog::ChangePhase(Phase phase)
{
    m_camera->SetType(Gfx::CAM_TYPE_DIALOG);
    m_engine->SetOverFront(false);
    m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::ENG_RSTATE_TCOLOR_BLACK); // TODO: color ok?

    m_phase = phase;  // copy the info to CRobotMain

    m_currentScreen = nullptr;

    if (m_phase == PHASE_QUIT_SCREEN)
    {
        m_currentScreen = m_screenQuit.get();
    }
    if (m_phase >= PHASE_WELCOME1 && m_phase <= PHASE_WELCOME3)
    {
        m_screenWelcome->SetImageIndex(m_phase - PHASE_WELCOME1);
        m_currentScreen = m_screenWelcome.get();
    }
    if (m_phase == PHASE_PLAYER_SELECT)
    {
        m_currentScreen = m_screenPlayerSelect.get();
    }
    if (m_phase == PHASE_APPERANCE)
    {
        m_currentScreen = m_screenAppearance.get();
    }
    if (m_phase == PHASE_MAIN_MENU)
    {
        m_currentScreen = m_screenMainMenu.get();
    }
    if (m_phase == PHASE_LEVEL_LIST)
    {
        m_screenLevelList->SetLevelCategory(m_main->GetLevelCategory());
        m_currentScreen = m_screenLevelList.get();
    }
    if (m_phase == PHASE_LOADING)
    {
        m_currentScreen = m_screenLoading.get();
    }
    if (m_phase >= PHASE_SETUPd && m_phase <= PHASE_SETUPs)
    {
        m_screenSetup->SetTab(m_phase, false);
        m_currentScreen = m_screenSetup.get();
    }
    if (m_phase >= PHASE_SETUPds && m_phase <= PHASE_SETUPss)
    {
        m_screenSetup->SetTab(static_cast<Phase>(m_phase - PHASE_SETUPds + PHASE_SETUPd), true);
        m_currentScreen = m_screenSetup.get();
    }
    if (m_phase == PHASE_WRITEs)
    {
        m_currentScreen = m_screenIOWrite.get();
    }
    if (m_phase == PHASE_READ || m_phase == PHASE_READs)
    {
        m_currentScreen = m_screenIORead.get();
        m_screenIORead->SetInSimulation(m_phase == PHASE_READs);
    }

    if (m_currentScreen != nullptr)
    {
        m_currentScreen->CreateInterface();
    }

    if ( !IsPhaseWithWorld(m_phase) )
    {
        if (!m_sound->IsPlayingMusic() && m_sound->IsCachedMusic("Intro1.ogg"))
        {
            m_sound->PlayMusic("Intro1.ogg", false);
        }
    }

    if ( m_phase == PHASE_MAIN_MENU    ||
            m_phase == PHASE_PLAYER_SELECT    ||
            m_phase == PHASE_LEVEL_LIST ||
            m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_READ    ||
            m_phase == PHASE_LOADING )
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if (pw != nullptr)
        {
            Math::Point pos, ddim;

            pos.x  = 540.0f/640.0f;
            pos.y  =   9.0f/480.0f;
            ddim.x =  90.0f/640.0f;
            ddim.y =  10.0f/480.0f;
            CLabel* pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, COLOBOT_VERSION_DISPLAY);
            pl->SetFontType(Gfx::FONT_COURIER);
            pl->SetFontSize(9.0f);
        }
    }

    m_engine->LoadAllTextures();
}


// Processing an event.
// Returns false if the event has been processed completely.

bool CMainDialog::EventProcess(const Event &event)
{
    Event       newEvent;

    if ( !m_interface->EventProcess(event) )
    {
        return false;
    }

    if (m_currentScreen != nullptr && !m_currentScreen->EventProcess(event)) return false;

    if ( event.type == EVENT_FRAME )
    {
        if ( !IsPhaseWithWorld(m_phase) )
        {
            if (!m_sound->IsPlayingMusic() && m_sound->IsCachedMusic("Intro2.ogg"))
            {
                m_sound->PlayMusic("Intro2.ogg", true);
            }
        }

        if ( m_shotDelay > 0 && !m_bDialog )  // screenshot done?
        {
            m_shotDelay --;
            if ( m_shotDelay == 0 )
            {
                Math::IntPoint windowSize = m_engine->GetWindowSize();

                m_engine->WriteScreenShot(m_shotName, windowSize.x, windowSize.y);
            }
        }

        m_glintTime += event.rTime;
        GlintMove();  // moves reflections

        FrameParticle(event.rTime);

        if ( m_bDialog )  // this dialogue?
        {
            FrameDialog(event.rTime);
        }

        return true;
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        m_glintMouse = event.mousePos;
        NiceParticle(event.mousePos, event.mouseButtonsState & MOUSE_BUTTON_LEFT);
    }

    if ( m_bDialog )  // this dialogue?
    {
        if ( event.type == EVENT_DIALOG_OK ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(RETURN) ) )
        {
            StopDialog();
            if ( m_phase == PHASE_PLAYER_SELECT )
            {
                m_screenPlayerSelect->NameDelete();
            }
            if ( m_phase == PHASE_SIMUL )
            {
                if ( m_bDialogDelete )
                {
                    m_main->DeleteObject();
                }
                else
                {
                    m_main->ChangePhase(PHASE_LEVEL_LIST);
                }
            }
        }
        if ( event.type == EVENT_DIALOG_CANCEL ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(ESCAPE) ) )
        {
            StopDialog();
        }
        if ( event.type == EVENT_INTERFACE_SETUP )
        {
            StopDialog();
            m_main->StartSuspend();
            #if PLATFORM_LINUX
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPds);
            #else
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPgs);
            #endif
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPg )  m_main->ChangePhase(PHASE_SETUPgs);
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPp )  m_main->ChangePhase(PHASE_SETUPps);
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPc )  m_main->ChangePhase(PHASE_SETUPcs);
            if ( m_screenSetup->GetSetupTab() == PHASE_SETUPs )  m_main->ChangePhase(PHASE_SETUPss);
        }
        if ( event.type == EVENT_INTERFACE_AGAIN )
        {
            StopDialog();
            m_main->ChangePhase(PHASE_LOADING);
        }
        if ( event.type == EVENT_INTERFACE_WRITE )
        {
            StopDialog();
            m_main->StartSuspend();
            m_main->ChangePhase(PHASE_WRITEs);
        }
        if ( event.type == EVENT_INTERFACE_READ )
        {
            StopDialog();
            m_main->StartSuspend();
            m_main->ChangePhase(PHASE_READs);
        }

        return false;
    }

    return true;
}


// Moves the reflections.

void CMainDialog::GlintMove()
{
    CWindow*    pw;
    CGroup*     pg;
    Math::Point     pos, dim, zoom;

    if ( m_phase == PHASE_SIMUL )  return;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.23f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.35f;
            pos.y = 0.90f;
            dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
            dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.21f);
            zoom.y = sinf(m_glintTime*0.26f);
            pos.x = 0.65f;
            pos.y = 0.10f;
            dim.x = 0.30f-0.10f*(zoom.x+1.0f)/2.0f;
            dim.y = 0.50f-0.30f*(zoom.y+1.0f)/2.0f;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_PLAYER_SELECT       ||
         m_phase == PHASE_LEVEL_LIST  )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.22f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.10f;
            pos.y = 0.90f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.19f);
            zoom.y = sinf(m_glintTime*0.28f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_SETUPds ||
            m_phase == PHASE_SETUPgs ||
            m_phase == PHASE_SETUPps ||
            m_phase == PHASE_SETUPcs ||
            m_phase == PHASE_SETUPss )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTu));
        if ( pg != 0 )
        {
            zoom.y = sinf(m_glintTime*0.27f);
            pos.x = 0.10f;
            pos.y = 0.76f;
            dim.x = 0.80f;
            dim.y = 0.32f+0.20f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.29f);
            zoom.y = sinf(m_glintTime*0.14f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.40f+0.20f*zoom.x;
            dim.y = 0.40f+0.20f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }

    if ( m_phase == PHASE_WRITEs ||
         m_phase == PHASE_READ   ||
         m_phase == PHASE_READs  )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.22f);
            zoom.y = sinf(m_glintTime*0.37f);
            pos.x = 0.10f;
            pos.y = 0.90f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.y -= dim.y;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }

        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTr));
        if ( pg != 0 )
        {
            zoom.x = sinf(m_glintTime*0.19f);
            zoom.y = sinf(m_glintTime*0.28f);
            pos.x = 0.90f;
            pos.y = 0.10f;
            dim.x = 0.60f+0.30f*zoom.x;
            dim.y = 0.60f+0.30f*zoom.y;
            pos.x -= dim.x;
            pg->SetPos(pos);
            pg->SetDim(dim);
        }
    }
}


// Returns the position for a sound.

Math::Vector SoundPos(Math::Point pos)
{
    Math::Vector    s;

    s.x = (pos.x-0.5f)*2.0f;
    s.y = (pos.y-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Returns a random position for a sound.

Math::Vector SoundRand()
{
    Math::Vector    s;

    s.x = (Math::Rand()-0.5f)*2.0f;
    s.y = (Math::Rand()-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Makes pretty qq particles evolve.

void CMainDialog::FrameParticle(float rTime)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       *pParti, *pGlint;
    int          nParti,  nGlint;
    int         i, r, ii;

    static float partiPosInit[1+5*12] =
    { //  x       x      t2    t2   type
        12.0f,
        607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // upper cable
        604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // middle cable
        603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // lower cable
        119.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // left pipe
        366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // upper pipe
        560.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // button lower/right
        20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // button lower/left
        39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // left pot
        39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // left pot
        170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // left smoke
        170.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // left smoke
        474.0f, 229.0f, 0.5f, 0.5f, 3.0f,  // right smoke
    };

    static float glintPosInit[1+2*14] =
    {
        14.0f,
        15.0f, 407.0f,
        68.0f, 417.0f,
        548.0f,  36.0f,
        611.0f,  37.0f,
        611.0f, 100.0f,
        611.0f, 395.0f,
        36.0f,  35.0f,
        166.0f,  55.0f,
        166.0f,  94.0f,
        477.0f,  56.0f,
        31.0f, 190.0f,
        32.0f, 220.0f,
        65.0f, 221.0f,
        65.0f, 250.0f,
    };

    static float partiPosBig[1+5*12] =
    { //  x       x      t2    t2   type
        12.0f,
        607.0f, 164.0f, 0.2f, 0.8f, 1.0f,  // upper cable
        604.0f, 205.0f, 0.1f, 0.3f, 1.0f,  // middle cable
        603.0f, 247.0f, 0.1f, 0.3f, 1.0f,  // lower cable
        64.0f, 444.0f, 0.2f, 0.8f, 1.0f,  // down the left cable
        113.0f, 449.0f, 0.1f, 0.3f, 1.0f,  // down the left cable
        340.0f, 463.0f, 0.2f, 0.8f, 1.0f,  // down the middle cable
        36.0f, 155.0f, 0.2f, 0.4f, 2.0f,  // left pipe
        366.0f,  23.0f, 0.5f, 1.5f, 4.0f,  // upper pipe
        612.0f, 414.0f, 0.1f, 0.1f, 1.0f,  // button lower/right
        20.0f, 413.0f, 0.1f, 0.1f, 2.0f,  // button lower/left
        39.0f,  78.0f, 0.1f, 0.2f, 1.0f,  // left pot
        39.0f,  78.0f, 0.5f, 0.9f, 1.0f,  // left pot
    };

    static float glintPosBig[1+2*12] =
    {
        12.0f,
        15.0f, 407.0f,
        48.0f, 399.0f,
        611.0f,  37.0f,
        611.0f, 100.0f,
        611.0f, 395.0f,
        36.0f,  35.0f,
        31.0f, 190.0f,
        32.0f, 220.0f,
        31.0f, 221.0f,
        31.0f, 189.0f,
        255.0f,  18.0f,
        279.0f,  18.0f,
    };

    if ( m_bDialog || !m_settings->GetInterfaceRain() )  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pParti = partiPosInit;
        pGlint = glintPosInit;
    }
    else if ( m_phase == PHASE_PLAYER_SELECT    ||
            m_phase == PHASE_LEVEL_LIST ||
            m_phase == PHASE_SETUPd  ||
            m_phase == PHASE_SETUPg  ||
            m_phase == PHASE_SETUPp  ||
            m_phase == PHASE_SETUPc  ||
            m_phase == PHASE_SETUPs  ||
            m_phase == PHASE_READ    )
    {
        pParti = partiPosBig;
        pGlint = glintPosBig;
    }
    else
    {
        return;
    }

    nParti = static_cast<int>(*pParti++);
    nGlint = static_cast<int>(*pGlint++);

    for ( i=0 ; i<10 ; i++ )
    {
        if ( m_partiPhase[i] == 0 )  // waiting?
        {
            m_partiTime[i] -= rTime;
            if ( m_partiTime[i] <= 0.0f )
            {
                r = rand()%3;

                if ( r == 0 )
                {
                    ii = rand()%nParti;
                    m_partiPos[i].x = pParti[ii*5+0]/640.0f;
                    m_partiPos[i].y = (480.0f-pParti[ii*5+1])/480.0f;
                    m_partiTime[i] = pParti[ii*5+2]+Math::Rand()*pParti[ii*5+3];
                    m_partiPhase[i] = static_cast<int>(pParti[ii*5+4]);
                    if ( m_partiPhase[i] == 3 )
                    {
                        m_sound->Play(SOUND_PSHHH, SoundPos(m_partiPos[i]), 0.3f+Math::Rand()*0.3f);
                    }
                    else
                    {
                        m_sound->Play(SOUND_GGG, SoundPos(m_partiPos[i]), 0.1f+Math::Rand()*0.4f);
                    }
                }

                if ( r == 1 )
                {
                    ii = rand()%nGlint;
                    pos.x = pGlint[ii*2+0]/640.0f;
                    pos.y = (480.0f-pGlint[ii*2+1])/480.0f;
                    pos.z = 0.0f;
                    speed.x = 0.0f;
                    speed.y = 0.0f;
                    speed.z = 0.0f;
                    dim.x = 0.04f+Math::Rand()*0.04f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim,
                            rand()%2?Gfx::PARTIGLINT:Gfx::PARTICONTROL,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    m_partiTime[i] = 0.5f+Math::Rand()*0.5f;
                }

                if ( r == 2 )
                {
                    ii = rand()%7;
                    if ( ii == 0 )
                    {
                        m_sound->Play(SOUND_ENERGY, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Math::Rand()*1.0f;
                    }
                    if ( ii == 1 )
                    {
                        m_sound->Play(SOUND_STATION, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 1.0f+Math::Rand()*2.0f;
                    }
                    if ( ii == 2 )
                    {
                        m_sound->Play(SOUND_ALARM, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 3 )
                    {
                        m_sound->Play(SOUND_INFO, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 4 )
                    {
                        m_sound->Play(SOUND_RADAR, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_partiTime[i] = 0.5f+Math::Rand()*1.0f;
                    }
                    if ( ii == 5 )
                    {
                        m_sound->Play(SOUND_GFLAT, SoundRand(), 0.3f+Math::Rand()*0.3f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 6 )
                    {
                        m_sound->Play(SOUND_ALARMt, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
                    }
                }
            }
        }

        if ( m_partiPhase[i] != 0 )  // generates?
        {
            m_partiTime[i] -= rTime;
            if ( m_partiTime[i] > 0.0f )
            {
                if ( m_partiPhase[i] == 1 )  // sparks?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.01f+Math::Rand()*0.01f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim,
                            static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 2 )  // sparks?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS,
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_partiPhase[i] == 3 )  // smoke?
                {
                    pos.x = m_partiPos[i].x;
                    pos.y = m_partiPos[i].y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.03f;
                    pos.y += (Math::Rand()-0.5f)*0.03f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = Math::Rand()*0.5f;
                    speed.z = 0.0f;
                    dim.x = 0.03f+Math::Rand()*0.07f;
                    dim.y = dim.x/0.75f;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
            }
            else
            {
                m_partiPhase[i] = 0;
                m_partiTime[i] = 2.0f+Math::Rand()*4.0f;
            }
        }
    }
    // #endif
}

// Some nice particles following the mouse.

void CMainDialog::NiceParticle(Math::Point mouse, bool bPress)
{
    Math::Vector    pos, speed;
    Math::Point     dim;

    if ( !m_settings->GetInterfaceRain() )  return;
    if ( (m_phase == PHASE_SIMUL ||
                m_phase == PHASE_WIN   ||
                m_phase == PHASE_LOST  ) &&
            !m_bDialog             )  return;

    if ( bPress )
    {
        pos.x = mouse.x;
        pos.y = mouse.y;
        pos.z = 0.0f;
        speed.x = (Math::Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Math::Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.005f+Math::Rand()*0.005f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS,
                Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                Gfx::SH_INTERFACE);
    }
    else
    {
        pos.x = mouse.x;
        pos.y = mouse.y;
        pos.z = 0.0f;
        speed.x = (Math::Rand()-0.5f)*0.5f;
        speed.y = (0.3f+Math::Rand()*0.3f);
        speed.z = 0.0f;
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                Gfx::SH_INTERFACE);
    }
}

// Updates the lists according to the cheat code.

void CMainDialog::AllMissionUpdate()
{
    if ( m_phase == PHASE_LEVEL_LIST )
    {
        m_screenLevelList->AllMissionUpdate();
    }
}

void CMainDialog::MakeSaveScreenshot(const std::string& name)
{
    m_shotDelay = 3;
    m_shotName = CResourceManager::GetSaveLocation() + "/" + name; //TODO: Use PHYSFS?
}

// Updates the button "solution" according to cheat code.

void CMainDialog::ShowSoluceUpdate()
{
    if ( m_phase == PHASE_LEVEL_LIST )
    {
        m_screenLevelList->ShowSoluceUpdate();
    }
}

// Do you want to quit the current mission?

void CMainDialog::StartAbort()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;
    std::string name;

    StartDialog(Math::Point(0.3f, 0.8f), true, false, false);
    m_bDialogDelete = false;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

    pos.x = 0.35f;
    pos.y = 0.60f;
    dim.x = 0.30f;
    dim.y = 0.30f;
    pw->CreateGroup(pos, dim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x = 0.35f;
    pos.y = 0.10f;
    dim.x = 0.30f;
    dim.y = 0.30f;
    pw->CreateGroup(pos, dim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x = 0.40f;
    dim.x = 0.20f;
    dim.y = 32.0f/480.0f;

    pos.y = 0.74f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_CANCEL);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_NO, name);
    pb->SetName(name);

    if ( m_main->GetLevelCategory() == LevelCategory::Missions     ||  // missions ?
         m_main->GetLevelCategory() == LevelCategory::FreeGame     ||  // free games?
         m_main->GetLevelCategory() == LevelCategory::CustomLevels )   // user ?
    {
        pos.y = 0.62f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_WRITE);
        pb->SetState(STATE_SHADOW);
        if ( m_main->IsBusy() )  // current task?
        {
            pb->ClearState(STATE_ENABLE);
        }

        pos.y = 0.53f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_READ);
        pb->SetState(STATE_SHADOW);
        if ( !m_main->GetPlayerProfile()->HasAnySavedScene() )  // no file to read?
        {
            pb->ClearState(STATE_ENABLE);
        }
        pb->SetState(STATE_WARNING);
    }

    pos.y = 0.39f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_SETUP);
    pb->SetState(STATE_SHADOW);

    pos.y = 0.25f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_AGAIN);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_WARNING);

    pos.y = 0.16f;
    pb = pw->CreateButton(pos, dim, -1, EVENT_DIALOG_OK);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_WARNING);
    GetResource(RES_TEXT, RT_DIALOG_YES, name);
    pb->SetName(name);
}

// Do you want to destroy the building?

void CMainDialog::StartDeleteObject()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;
    std::string name;

    StartDialog(Math::Point(0.7f, 0.3f), false, true, true);
    m_bDialogDelete = true;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == 0 )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELOBJ, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Do you want to delete the player?

void CMainDialog::StartDeleteGame(char *gamer)
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;

    StartDialog(Math::Point(0.7f, 0.3f), false, true, true);
    m_bDialogDelete = true;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if (pw == nullptr)
        return;

    std::string name;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_DELGAME, name);
    std::string text = StrUtils::Format(name.c_str(), gamer);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, text);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESDEL, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NODEL, name);
    pb->SetName(name);
}

// Would you quit the game?

void CMainDialog::StartQuit()
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, dim;

    StartDialog(Math::Point(0.6f, 0.3f), false, true, true);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if (pw == nullptr)
        return;

    std::string name;

    pos.x = 0.00f;
    pos.y = 0.50f;
    dim.x = 1.00f;
    dim.y = 0.05f;
    GetResource(RES_TEXT, RT_DIALOG_QUIT, name);
    pw->CreateLabel(pos, dim, -1, EVENT_DIALOG_LABEL, name);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_YESQUIT, name);
    pb->SetName(name);
    pb->SetState(STATE_WARNING);

    pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
    if ( pb == 0 )  return;
    GetResource(RES_TEXT, RT_DIALOG_NOQUIT, name);
    pb->SetName(name);
}

// Beginning of displaying a dialog.

void CMainDialog::StartDialog(Math::Point dim, bool bFire, bool bOK, bool bCancel)
{
    CWindow*    pw;
    CButton*    pb;
    Math::Point     pos, ddim;

    m_main->StartSuspend();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != 0 )  pw->ClearState(STATE_ENABLE);

    m_bDialogFire = bFire;

    std::string name;

    pos.x = (1.0f-dim.x)/2.0f;
    pos.y = (1.0f-dim.y)/2.0f;
    pw = m_interface->CreateWindows(pos, dim, bFire?12:8, EVENT_WINDOW9);
    pw->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_TITLE_BASE, name);
    pw->SetName(name);

    m_dialogPos = pos;
    m_dialogDim = dim;
    m_dialogTime = 0.0f;
    m_dialogParti = 999.0f;

    if ( bOK )
    {
        pos.x  = 0.50f-0.15f-0.02f;
        pos.y  = 0.50f-dim.y/2.0f+0.03f;
        ddim.x = 0.15f;
        ddim.y = 0.06f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_OK);
        pb->SetState(STATE_SHADOW);
        GetResource(RES_EVENT, EVENT_DIALOG_OK, name);
        pb->SetName(name);
    }

    if ( bCancel )
    {
        pos.x  = 0.50f+0.02f;
        pos.y  = 0.50f-dim.y/2.0f+0.03f;
        ddim.x = 0.15f;
        ddim.y = 0.06f;
        pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_CANCEL);
        pb->SetState(STATE_SHADOW);
        GetResource(RES_EVENT, EVENT_DIALOG_CANCEL, name);
        pb->SetName(name);
    }

    m_sound->Play(SOUND_TZOING);
    m_bDialog = true;
}

// Animation of a dialog.

void CMainDialog::FrameDialog(float rTime)
{
    CWindow*    pw;
    Math::Vector    pos, speed;
    Math::Point     dim, dpos, ddim;
    float       zoom;
    int         i;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    m_dialogTime += rTime;
    if ( m_dialogTime < 1.0f )
    {
        pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
        if ( pw != 0 )
        {
            if ( m_dialogTime < 0.50f )
            {
                zoom = Math::Bounce(m_dialogTime/0.50f);
            }
            else
            {
                zoom = 1.0f;
            }

            dpos.x += ddim.x/2.0f;
            dpos.y += ddim.y/2.0f;

            ddim.x *= zoom;
//?         ddim.y *= zoom;

            dpos.x -= ddim.x/2.0f;
            dpos.y -= ddim.y/2.0f;

            pw->SetPos(dpos);
            pw->SetDim(ddim);
        }
    }

    if ( !m_settings->GetInterfaceGlint() )  return;

    m_dialogParti += rTime;
    if ( m_dialogParti < m_engine->ParticleAdapt(0.05f) )  return;
    m_dialogParti = 0.0f;

    if ( !m_bDialogFire )  return;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    pos.z = 0.0f;
    speed = Math::Vector(0.0f, 0.0f, 0.0f);

    for ( i=0 ; i<2 ; i++ )
    {
        // Bottom.
        pos.x = dpos.x + ddim.x*Math::Rand();
        pos.y = dpos.y;
        pos.x += (Math::Rand()-0.5f)*(6.0f/640.0f);
        pos.y += Math::Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Top.
        pos.x = dpos.x + ddim.x*Math::Rand();
        pos.y = dpos.y + ddim.y;
        pos.x += (Math::Rand()-0.5f)*(6.0f/640.0f);
        pos.y -= Math::Rand()*(16.0f/480.0f)-(10.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Left.
        pos.y = dpos.y + ddim.y*Math::Rand();
        pos.x = dpos.x;
        pos.x += Math::Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Math::Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);

        // Right.
        pos.y = dpos.y + ddim.y*Math::Rand();
        pos.x = dpos.x + ddim.x;
        pos.x -= Math::Rand()*(16.0f/640.0f)-(10.0f/640.0f);
        pos.y += (Math::Rand()-0.5f)*(6.0f/480.0f);
        dim.x = 0.01f+Math::Rand()*0.01f;
        dim.y = dim.x/0.75f;
        m_particle->CreateParticle(pos, speed, dim,
                                     static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                                     1.0f, 0.0f, 0.0f, Gfx::SH_INTERFACE);
    }
}

// End of the display of a dialogue.

void CMainDialog::StopDialog()
{
    CWindow*    pw;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != 0 )  pw->SetState(STATE_ENABLE);

    m_main->StopSuspend();
    m_interface->DeleteControl(EVENT_WINDOW9);
    m_bDialog = false;
}


// Specifies whether a dialog is displayed.

bool CMainDialog::IsDialog()
{
    return m_bDialog;
}


// Whether to show the solution.

bool CMainDialog::GetSceneSoluce()
{
    return m_screenLevelList->GetSceneSoluce();
}

bool CMainDialog::GetGamerOnlyHead()
{
    if (m_phase == PHASE_APPERANCE)
        return m_screenAppearance->GetGamerOnlyHead();

    return false;
}

float CMainDialog::GetPersoAngle()
{
    if (m_phase == PHASE_APPERANCE)
        return m_screenAppearance->GetPersoAngle();

    return 0.0f;
}

void CMainDialog::UpdateChapterPassed()
{
    m_screenLevelList->UpdateChapterPassed();
}

void CMainDialog::NextMission()
{
    m_screenLevelList->NextMission();
}

void CMainDialog::UpdateCustomLevelList()
{
    m_screenLevelList->UpdateCustomLevelList();
}

std::string CMainDialog::GetCustomLevelName(int id)
{
    return m_screenLevelList->GetCustomLevelName(id);
}

const std::vector<std::string>& CMainDialog::GetCustomLevelList()
{
    return m_screenLevelList->GetCustomLevelList();
}


} // namespace Ui
