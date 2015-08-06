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

#include "common/event.h"
#include "common/logger.h"
#include "common/make_unique.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "object/player_profile.h"
#include "object/robotmain.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/interface.h"
#include "ui/mainui.h"
#include "ui/controls/window.h"

#include "ui/screen/screen_setup.h"

namespace Ui
{

// Constructor of robot application.

CMainDialog::CMainDialog()
{
    m_main       = CRobotMain::GetInstancePointer();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_particle   = m_engine->GetParticle();
    m_interface  = m_main->GetInterface();
    m_sound      = CApplication::GetInstancePointer()->GetSound();
    m_settings   = CSettings::GetInstancePointer();

    m_phase = PHASE_PLAYER_SELECT;

    m_bDialog = false;
}

// Destructor of robot application.

CMainDialog::~CMainDialog()
{
}

void CMainDialog::ChangePhase(Phase phase)
{
    m_phase = phase;  // copy the info to CRobotMain
}

// Processing an event.
// Returns false if the event has been processed completely.

bool CMainDialog::EventProcess(const Event &event)
{
    if ( m_bDialog )  // this dialogue?
    {
        if ( event.type == EVENT_FRAME )
        {
            FrameDialog(event.rTime);
            return true;
        }

        if ( event.type == EVENT_DIALOG_OK ||
                (event.type == EVENT_KEY_DOWN && event.key.key == KEY(RETURN) ) )
        {
            StopDialog();
            if ( m_phase == PHASE_PLAYER_SELECT )
            {
                CMainUserInterface::GetInstancePointer()->NameDelete(); // TODO: Clean this up
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
            if ( CScreenSetup::GetTab() == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPds);
            #else
            if ( CScreenSetup::GetTab() == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPgs);
            #endif
            if ( CScreenSetup::GetTab() == PHASE_SETUPg )  m_main->ChangePhase(PHASE_SETUPgs);
            if ( CScreenSetup::GetTab() == PHASE_SETUPp )  m_main->ChangePhase(PHASE_SETUPps);
            if ( CScreenSetup::GetTab() == PHASE_SETUPc )  m_main->ChangePhase(PHASE_SETUPcs);
            if ( CScreenSetup::GetTab() == PHASE_SETUPs )  m_main->ChangePhase(PHASE_SETUPss);
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

} // namespace Ui
