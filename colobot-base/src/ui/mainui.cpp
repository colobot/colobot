/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/mainui.h"

#include "app/app.h"

#include "common/event.h"
#include "common/logger.h"
#include "common/settings.h"
#include "common/stringutils.h"
#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include "math/func.h"

#include "sound/sound.h"

#include "ui/maindialog.h"

#include "ui/particles_generator.h"

#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

#include "ui/screen/screen.h"
#include "ui/screen/screen_appearance.h"
#include "ui/screen/screen_io_read.h"
#include "ui/screen/screen_io_write.h"
#include "ui/screen/screen_level_list.h"
#include "ui/screen/screen_loading.h"
#include "ui/screen/screen_main_menu.h"
#include "ui/screen/screen_mod_list.h"
#include "ui/screen/screen_player_select.h"
#include "ui/screen/screen_quit.h"
#include "ui/screen/screen_setup_controls.h"
#include "ui/screen/screen_setup_display.h"
#include "ui/screen/screen_setup_game.h"
#include "ui/screen/screen_setup_graphics.h"
#include "ui/screen/screen_setup_sound.h"
#include "ui/screen/screen_welcome.h"

#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"

namespace Ui
{

// Constructor of robot application.

CMainUserInterface::CMainUserInterface()
{
    m_app        = CApplication::GetInstancePointer();
    m_main       = CRobotMain::GetInstancePointer();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_particleManager   = m_engine->GetParticle();
    m_interface  = m_main->GetInterface();
    m_sound      = m_app->GetSound();
    m_settings   = CSettings::GetInstancePointer();

    m_dialog     = std::make_unique<CMainDialog>();

    m_screenAppearance = std::make_unique<CScreenAppearance>();
    m_screenLevelList = std::make_unique<CScreenLevelList>(m_dialog.get());
    m_screenIORead = std::make_unique<CScreenIORead>(m_screenLevelList.get(), m_dialog.get());
    m_screenIOWrite = std::make_unique<CScreenIOWrite>(m_screenLevelList.get());
    m_screenLoading = std::make_unique<CScreenLoading>();
    m_screenModList = std::make_unique<CScreenModList>(m_dialog.get(), m_app->GetModManager());
    m_screenSetupControls = std::make_unique<CScreenSetupControls>();
    m_screenSetupDisplay = std::make_unique<CScreenSetupDisplay>();
    m_screenSetupGame = std::make_unique<CScreenSetupGame>();
    m_screenSetupGraphics = std::make_unique<CScreenSetupGraphics>();
    m_screenSetupSound = std::make_unique<CScreenSetupSound>();
    m_screenMainMenu = std::make_unique<CScreenMainMenu>();
    m_screenPlayerSelect = std::make_unique<CScreenPlayerSelect>(m_dialog.get());
    m_screenQuit = std::make_unique<CScreenQuit>();
    m_screenWelcome = std::make_unique<CScreenWelcome>();
    m_mouseParticlesGenerator = std::make_unique<UI::CParticlesGenerator>();

    m_currentScreen = nullptr;

    m_phase         = PHASE_PLAYER_SELECT;

    m_glintMouse = { 0.0f, 0.0f };
    m_glintTime  = 1000.0f;
    m_shotDelay = 0;
}

// Destructor of robot application.

CMainUserInterface::~CMainUserInterface()
{
}

CMainDialog* CMainUserInterface::GetDialog()
{
    return m_dialog.get();
}

void CMainUserInterface::ShowLoadingScreen(bool show)
{
    if (show)
    {
        m_app->SetMouseMode(MOUSE_NONE);
        m_currentScreen = m_screenLoading.get();
        m_screenLoading->CreateInterface();
    }
    else
    {
        if (m_currentScreen == m_screenLoading.get())
        {
            m_screenLoading->DestroyInterface();
            m_currentScreen = nullptr;
        }
        m_app->SetMouseMode(MOUSE_ENGINE);
    }
}

CScreenLoading* CMainUserInterface::GetLoadingScreen()
{
    return m_screenLoading.get();
}

// Changes phase.

CScreenSetup* CMainUserInterface::GetSetupScreen(Phase phase)
{
    if(phase == PHASE_SETUPd) return m_screenSetupDisplay.get();
    if(phase == PHASE_SETUPg) return m_screenSetupGraphics.get();
    if(phase == PHASE_SETUPp) return m_screenSetupGame.get();
    if(phase == PHASE_SETUPc) return m_screenSetupControls.get();
    if(phase == PHASE_SETUPs) return m_screenSetupSound.get();
    assert(false);
    return nullptr;
}

void CMainUserInterface::ChangePhase(Phase phase)
{
    m_main->GetCamera()->SetType(Gfx::CAM_TYPE_NULL);
    m_engine->SetOverFront(false);
    m_engine->SetOverColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f), Gfx::TransparencyMode::BLACK); // TODO: color ok?

    m_phase = phase;  // copy the info from CRobotMain

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
    if (m_phase == PHASE_APPEARANCE)
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
    if (m_phase == PHASE_MOD_LIST)
    {
        m_currentScreen = m_screenModList.get();
    }
    if (m_phase >= PHASE_SETUPd && m_phase <= PHASE_SETUPs)
    {
        CScreenSetup* screenSetup = GetSetupScreen(m_phase);
        screenSetup->SetInSimulation(false);
        screenSetup->SetActive();
        m_currentScreen = screenSetup;
    }
    if (m_phase >= PHASE_SETUPds && m_phase <= PHASE_SETUPss)
    {
        CScreenSetup* screenSetup = GetSetupScreen(static_cast<Phase>(m_phase - PHASE_SETUPds + PHASE_SETUPd));
        screenSetup->SetInSimulation(true);
        screenSetup->SetActive();
        m_currentScreen = screenSetup;
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

    if ( IsMainMenuPhase(m_phase) )
    {
        if (!m_sound->IsPlayingMusic())
        {
            m_sound->PlayMusic("music/Intro1.ogg", false);
            m_sound->CacheMusic("music/Intro2.ogg");
        }
    }

    m_engine->LoadAllTextures();
}


// Processing an event.
// Returns false if the event has been processed completely.

bool CMainUserInterface::EventProcess(const Event &event)
{
    if ( !m_interface->EventProcess(event) )
    {
        return false;
    }

    if (m_currentScreen != nullptr && !m_currentScreen->EventProcess(event)) return false;

    if ( event.type == EVENT_FRAME )
    {
        if ( IsMainMenuPhase(m_phase) )
        {
            if (!m_sound->IsPlayingMusic() && m_sound->IsCachedMusic("music/Intro2.ogg"))
            {
                m_sound->PlayMusic("music/Intro2.ogg", true);
            }
        }

        m_glintTime += event.rTime;
        GlintMove();  // moves reflections

        FrameParticle(event.rTime);
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        m_glintMouse = event.mousePos;
        CreateMouseParticles(event.mousePos, event.mouseButtonsState & MOUSE_BUTTON_LEFT);
    }

    if (!m_dialog->EventProcess(event)) return false;

    return true;
}


// Moves the reflections.

void CMainUserInterface::GlintMove()
{
    CWindow*    pw;
    CGroup*     pg;
    glm::vec2     pos, dim, zoom;

    if ( m_phase == PHASE_SIMUL )  return;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pg = static_cast<CGroup*>(pw->SearchControl(EVENT_INTERFACE_GLINTl));
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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
        if ( pg != nullptr )
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

static glm::vec3 SoundPos(const glm::vec2& pos)
{
    glm::vec3    s;

    s.x = (pos.x-0.5f)*2.0f;
    s.y = (pos.y-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Returns a random position for a sound.

static glm::vec3 SoundRand()
{
    glm::vec3    s;

    s.x = (Math::Rand()-0.5f)*2.0f;
    s.y = (Math::Rand()-0.5f)*2.0f;
    s.z = 0.0f;

    return s;
}

// Makes pretty qq particles evolve.

void CMainUserInterface::FrameParticle(float rTime)
{
    glm::vec3    pos, speed;
    glm::vec2     dim;
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

    if (m_dialog->IsDialog() || !m_settings->GetMouseParticlesEnabled())  return;

    if ( m_phase == PHASE_MAIN_MENU )
    {
        pParti = partiPosInit;
        pGlint = glintPosInit;
    }
    else if ( m_phase == PHASE_PLAYER_SELECT    ||
            m_phase == PHASE_LEVEL_LIST ||
            m_phase == PHASE_MOD_LIST ||
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
        if ( m_particles[i].phase == 0 )  // waiting?
        {
            m_particles[i].time -= rTime;
            if ( m_particles[i].time <= 0.0f )
            {
                r = rand()%3;

                if ( r == 0 )
                {
                    ii = rand()%nParti;
                    m_particles[i].pos.x = pParti[ii*5+0]/640.0f;
                    m_particles[i].pos.y = (480.0f-pParti[ii*5+1])/480.0f;
                    m_particles[i].time = pParti[ii*5+2]+Math::Rand()*pParti[ii*5+3];
                    m_particles[i].phase = static_cast<int>(pParti[ii*5+4]);
                    if ( m_particles[i].phase == 3 )
                    {
                        m_sound->Play(SOUND_PSHHH, SoundPos(m_particles[i].pos), 0.3f+Math::Rand()*0.3f);
                    }
                    else
                    {
                        m_sound->Play(SOUND_GGG, SoundPos(m_particles[i].pos), 0.1f+Math::Rand()*0.4f);
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
                    m_particleManager->CreateParticle(pos, speed, dim,
                            rand()%2?Gfx::PARTIGLINT:Gfx::PARTICONTROL,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    m_particles[i].time = 0.5f+Math::Rand()*0.5f;
                }

                if ( r == 2 )
                {
                    ii = rand()%7;
                    if ( ii == 0 )
                    {
                        m_sound->Play(SOUND_ENERGY, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_particles[i].time = 1.0f+Math::Rand()*1.0f;
                    }
                    if ( ii == 1 )
                    {
                        m_sound->Play(SOUND_STATION, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_particles[i].time = 1.0f+Math::Rand()*2.0f;
                    }
                    if ( ii == 2 )
                    {
                        m_sound->Play(SOUND_ALARM, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_particles[i].time = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 3 )
                    {
                        m_sound->Play(SOUND_INFO, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_particles[i].time = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 4 )
                    {
                        m_sound->Play(SOUND_RADAR, SoundRand(), 0.2f+Math::Rand()*0.2f);
                        m_particles[i].time = 0.5f+Math::Rand()*1.0f;
                    }
                    if ( ii == 5 )
                    {
                        m_sound->Play(SOUND_GFLAT, SoundRand(), 0.3f+Math::Rand()*0.3f);
                        m_particles[i].time = 2.0f+Math::Rand()*4.0f;
                    }
                    if ( ii == 6 )
                    {
                        m_sound->Play(SOUND_ALARMt, SoundRand(), 0.1f+Math::Rand()*0.1f);
                        m_particles[i].time = 2.0f+Math::Rand()*4.0f;
                    }
                }
            }
        }

        if ( m_particles[i].phase != 0 )  // generates?
        {
            m_particles[i].time -= rTime;
            if ( m_particles[i].time > 0.0f )
            {
                if ( m_particles[i].phase == 1 )  // sparks?
                {
                    pos.x = m_particles[i].pos.x;
                    pos.y = m_particles[i].pos.y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particleManager->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_particles[i].pos.x;
                    pos.y = m_particles[i].pos.y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.01f+Math::Rand()*0.01f;
                    dim.y = dim.x/0.75f;
                    m_particleManager->CreateParticle(pos, speed, dim,
                            static_cast<Gfx::ParticleType>(Gfx::PARTILENS1+rand()%3),
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_particles[i].phase == 2 )  // sparks?
                {
                    pos.x = m_particles[i].pos.x;
                    pos.y = m_particles[i].pos.y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.01f;
                    pos.y += (Math::Rand()-0.5f)*0.01f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = (Math::Rand()-0.5f)*0.2f;
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particleManager->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ,
                            Math::Rand()*0.2f+0.2f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                    pos.x = m_particles[i].pos.x;
                    pos.y = m_particles[i].pos.y;
                    pos.z = 0.0f;
                    speed.x = (Math::Rand()-0.5f)*0.5f;
                    speed.y = (0.3f+Math::Rand()*0.3f);
                    speed.z = 0.0f;
                    dim.x = 0.005f+Math::Rand()*0.005f;
                    dim.y = dim.x/0.75f;
                    m_particleManager->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS,
                            Math::Rand()*0.5f+0.5f, 2.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
                if ( m_particles[i].phase == 3 )  // smoke?
                {
                    pos.x = m_particles[i].pos.x;
                    pos.y = m_particles[i].pos.y;
                    pos.z = 0.0f;
                    pos.x += (Math::Rand()-0.5f)*0.03f;
                    pos.y += (Math::Rand()-0.5f)*0.03f;
                    speed.x = (Math::Rand()-0.5f)*0.2f;
                    speed.y = Math::Rand()*0.5f;
                    speed.z = 0.0f;
                    dim.x = 0.03f+Math::Rand()*0.07f;
                    dim.y = dim.x/0.75f;
                    m_particleManager->CreateParticle(pos, speed, dim, Gfx::PARTICRASH,
                            Math::Rand()*0.4f+0.4f, 0.0f, 0.0f,
                            Gfx::SH_INTERFACE);
                }
            }
            else
            {
                m_particles[i].phase = 0;
                m_particles[i].time = 2.0f+Math::Rand()*4.0f;
            }
        }
    }
}

void CMainUserInterface::CreateMouseParticles(const glm::vec2& mousePosition, bool buttonPressed)
{
    if (isAllowedToCreateMouseParticles())
    {
        m_mouseParticlesGenerator->GenerateMouseParticles({ mousePosition.x, mousePosition.y }, buttonPressed);
    }
}

bool CMainUserInterface::isAllowedToCreateMouseParticles()
{
    return m_settings->GetMouseParticlesEnabled() &&
        !((m_phase == PHASE_SIMUL || m_phase == PHASE_WIN || m_phase == PHASE_LOST) && !m_dialog->IsDialog());
}

// Updates the lists according to the cheat code.

void CMainUserInterface::AllMissionUpdate()
{
    if ( m_phase == PHASE_LEVEL_LIST )
    {
        m_screenLevelList->AllMissionUpdate();
    }
}

// Updates the button "solution" according to cheat code.

void CMainUserInterface::ShowSoluceUpdate()
{
    if ( m_phase == PHASE_LEVEL_LIST )
    {
        m_screenLevelList->ShowSoluceUpdate();
    }
}

// Whether to show the solution.

bool CMainUserInterface::GetSceneSoluce()
{
    return m_screenLevelList->GetSceneSoluce();
}

bool CMainUserInterface::GetPlusTrainer()
{
    return m_screenLevelList->GetPlusTrainer();
}

bool CMainUserInterface::GetPlusResearch()
{
    return m_screenLevelList->GetPlusResearch();
}

bool CMainUserInterface::GetPlusExplorer()
{
    return m_screenLevelList->GetPlusExplorer();
}

bool CMainUserInterface::GetGamerOnlyHead()
{
    if (m_phase == PHASE_APPEARANCE)
        return m_screenAppearance->GetGamerOnlyHead();

    return false;
}

float CMainUserInterface::GetPersoAngle()
{
    if (m_phase == PHASE_APPEARANCE)
        return m_screenAppearance->GetPersoAngle();

    return 0.0f;
}

void CMainUserInterface::UpdateChapterPassed()
{
    m_screenLevelList->UpdateChapterPassed();
}

void CMainUserInterface::NextMission()
{
    m_screenLevelList->NextMission();
}

void CMainUserInterface::UpdateCustomLevelList()
{
    m_screenLevelList->UpdateCustomLevelList();
}

std::filesystem::path CMainUserInterface::GetCustomLevelName(int id)
{
    return m_screenLevelList->GetCustomLevelName(id);
}

const std::vector<std::filesystem::path>& CMainUserInterface::GetCustomLevelList()
{
    return m_screenLevelList->GetCustomLevelList();
}


} // namespace Ui
