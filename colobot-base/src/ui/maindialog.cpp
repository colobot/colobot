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


#include "ui/maindialog.h"

#include "common/config.h"

#include "app/app.h"

#include "common/event.h"
#include "common/logger.h"
#include "common/settings.h"

#include "graphics/engine/engine.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

#include "math/func.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
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

    m_dialogOpen = false;
    m_dialogType = {};
    m_dialogFireParticles = false;
    m_dialogTime = 0.0f;
    m_dialogParti = 0.0f;
}

// Destructor of robot application.

CMainDialog::~CMainDialog()
{
}

// Processing an event.
// Returns false if the event has been processed completely.

bool CMainDialog::EventProcess(const Event &event)
{
    if ( m_dialogOpen )  // this dialogue?
    {
        if ( event.type == EVENT_FRAME )
        {
            FrameDialog(event.rTime);
            return true;
        }

        EventType pressedButton = event.type;
        if (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(RETURN) )
        {
            pressedButton = EVENT_DIALOG_OK;
        }
        if (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE) )
        {
            pressedButton = EVENT_DIALOG_CANCEL;
        }

        if ( m_dialogType == DialogType::PauseMenu )
        {
            if ( pressedButton == EVENT_DIALOG_CANCEL )
            {
                StopDialog();
            }

            if ( pressedButton == EVENT_INTERFACE_SETUP )
            {
                StopDialog();
                m_main->StartSuspend();
                if ( CScreenSetup::GetTab() == PHASE_SETUPd )  m_main->ChangePhase(PHASE_SETUPds);
                if ( CScreenSetup::GetTab() == PHASE_SETUPg )  m_main->ChangePhase(PHASE_SETUPgs);
                if ( CScreenSetup::GetTab() == PHASE_SETUPp )  m_main->ChangePhase(PHASE_SETUPps);
                if ( CScreenSetup::GetTab() == PHASE_SETUPc )  m_main->ChangePhase(PHASE_SETUPcs);
                if ( CScreenSetup::GetTab() == PHASE_SETUPs )  m_main->ChangePhase(PHASE_SETUPss);
            }

            if ( pressedButton == EVENT_INTERFACE_WRITE )
            {
                StopDialog();
                m_main->StartSuspend();
                m_main->ChangePhase(PHASE_WRITEs);
            }

            if ( pressedButton == EVENT_INTERFACE_READ )
            {
                StopDialog();
                m_main->StartSuspend();
                m_main->ChangePhase(PHASE_READs);
            }

            if ( pressedButton == EVENT_INTERFACE_AGAIN )
            {
                StopDialog();
                m_main->ChangePhase(PHASE_SIMUL);
            }

            if ( pressedButton == EVENT_DIALOG_OK )
            {
                StopDialog();
                m_main->ChangePhase(PHASE_LEVEL_LIST);
            }
        }

        if ( m_dialogType == DialogType::Question )
        {
            if ( pressedButton == EVENT_DIALOG_OK )
            {
                StopDialog();
                if (m_callbackYes != nullptr)
                {
                    m_callbackYes();
                }
            }

            if ( pressedButton == EVENT_DIALOG_CANCEL )
            {
                StopDialog();
                if (m_callbackNo != nullptr)
                {
                    m_callbackNo();
                }
            }
        }

        return false;
    }

    return true;
}

// Do you want to quit the current mission?

void CMainDialog::StartPauseMenu()
{
    CWindow*    pw;
    CButton*    pb;
    glm::vec2   pos, dim;
    std::string name;

    StartDialog({ 0.3f, 0.8f }, true);
    m_dialogType = DialogType::PauseMenu;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == nullptr )  return;

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
    GetResource(RES_TEXT, RT_DIALOG_CONTINUE, name);
    pb->SetName(name);

    if ( (m_main->GetLevelCategory() == LevelCategory::Missions    ||  // missions ?
         m_main->GetLevelCategory() == LevelCategory::FreeGame     ||  // free games?
         m_main->GetLevelCategory() == LevelCategory::GamePlus     ||  // new game plus?
         m_main->GetLevelCategory() == LevelCategory::CustomLevels ) &&   // user ?
         m_main->GetMissionType() != MISSION_CODE_BATTLE             )
    {
        pos.y = 0.62f;
        pb = pw->CreateButton(pos, dim, -1, EVENT_INTERFACE_WRITE);
        pb->SetState(STATE_SHADOW);
        if ( m_main->IOIsBusy() )  // current task?
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
    GetResource(RES_TEXT, RT_DIALOG_ABORT, name);
    pb->SetName(name);
}

void CMainDialog::StartQuestion(const std::string& text, bool warningYes, bool warningNo, bool fireParticles, DialogCallback yes, DialogCallback no)
{
    CWindow*    pw;
    CButton*    pb;
    glm::vec2   pos, dim, ddim;
    std::string name;

    dim.x = 0.7f;
    dim.y = 0.3f;

    StartDialog(dim, fireParticles);
    m_dialogType = DialogType::Question;
    m_callbackYes = yes;
    m_callbackNo = no;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == nullptr )  return;

    pos.x = 0.00f;
    pos.y = 0.50f;
    ddim.x = 1.00f;
    ddim.y = 0.05f;
    pw->CreateLabel(pos, ddim, -1, EVENT_DIALOG_LABEL, text);

    pos.x  = 0.50f-0.15f-0.02f;
    pos.y  = 0.50f-dim.y/2.0f+0.03f;
    ddim.x = 0.15f;
    ddim.y = 0.06f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_OK);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_YES, name);
    pb->SetName(name);
    if (warningYes)
    {
        pb->SetState(STATE_WARNING);
    }

    pos.x  = 0.50f+0.02f;
    pos.y  = 0.50f-dim.y/2.0f+0.03f;
    ddim.x = 0.15f;
    ddim.y = 0.06f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_CANCEL);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_NO, name);
    pb->SetName(name);
    if (warningNo)
    {
        pb->SetState(STATE_WARNING);
    }
}

void CMainDialog::StartQuestion(ResTextType text, bool warningYes, bool warningNo, bool fireParticles, DialogCallback yes, DialogCallback no)
{
    std::string name;
    GetResource(RES_TEXT, text, name);
    StartQuestion(name, warningYes, warningNo, fireParticles, yes, no);
}

void CMainDialog::StartInformation(const std::string& title, const std::string& text, const std::string& details, bool warning, bool fireParticles, DialogCallback ok)
{
    CWindow*    pw;
    CButton*    pb;
    CLabel*     pl;
    glm::vec2   pos, dim, ddim;
    std::string name;

    dim.x = 0.7f;
    dim.y = 0.3f;

    StartDialog(dim, fireParticles);
    m_dialogType = DialogType::Question;
    m_callbackYes = ok;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == nullptr )  return;

    if(!title.empty())
        pw->SetName(title);

    pos.x = 0.00f;
    pos.y = 0.50f;
    ddim.x = 1.00f;
    ddim.y = 0.05f;
    pl = pw->CreateLabel(pos, ddim, -1, EVENT_DIALOG_LABEL, text);
    pl->SetFontType(Gfx::FONT_COMMON_BOLD);
    //TODO: Add \n support in CLabel
    pos.y -= ddim.y;
    pl = pw->CreateLabel(pos, ddim, -1, EVENT_DIALOG_LABEL1, details);
    pl->SetFontSize(10.0f);

    pos.x  = 0.50f-0.075f;
    pos.y  = 0.50f-dim.y/2.0f+0.03f;
    ddim.x = 0.15f;
    ddim.y = 0.06f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_DIALOG_OK);
    pb->SetState(STATE_SHADOW);
    GetResource(RES_TEXT, RT_DIALOG_OK, name);
    pb->SetName(name);
    if (warning)
    {
        pb->SetState(STATE_WARNING);
    }
}

// Beginning of displaying a dialog.

void CMainDialog::StartDialog(const glm::vec2& dim, bool fireParticles)
{
    CWindow*    pw;
    glm::vec2   pos;

    m_main->StartSuspend();

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != nullptr )  pw->ClearState(STATE_VISIBLE | STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != nullptr )  pw->ClearState(STATE_ENABLE);

    m_dialogFireParticles = fireParticles;

    pos.x = (1.0f-dim.x)/2.0f;
    pos.y = (1.0f-dim.y)/2.0f;
    pw = m_interface->CreateWindows(pos, dim, m_dialogFireParticles ? 12 : 8, EVENT_WINDOW9);
    pw->SetState(STATE_SHADOW);
    std::string name;
    GetResource(RES_TEXT, RT_TITLE_APPNAME, name);
    pw->SetName(name);

    m_dialogPos = pos;
    m_dialogDim = dim;
    m_dialogTime = 0.0f;
    m_dialogParti = 999.0f;

    m_sound->Play(SOUND_TZOING);
    m_dialogOpen = true;
}

// Animation of a dialog.

void CMainDialog::FrameDialog(float rTime)
{
    CWindow*    pw;
    glm::vec3    pos, speed;
    glm::vec2   dim, dpos, ddim;
    float       zoom;
    int         i;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    m_dialogTime += rTime;
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw != nullptr )
    {
        if ( m_dialogTime < 1.0f )
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
        }
        pw->SetPos(dpos);
        pw->SetDim(ddim);
    }

    if ( !m_settings->GetInterfaceGlint() )  return;

    m_dialogParti += rTime;
    if ( m_dialogParti < m_engine->ParticleAdapt(0.05f) )  return;
    m_dialogParti = 0.0f;

    if ( !m_dialogFireParticles )  return;

    dpos = m_dialogPos;
    ddim = m_dialogDim;

    pos.z = 0.0f;
    speed = glm::vec3(0.0f, 0.0f, 0.0f);

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
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW1));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW2));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW3));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW4));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW6));
    if ( pw != nullptr )  pw->SetState(STATE_VISIBLE | STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW7));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW8));
    if ( pw != nullptr )  pw->SetState(STATE_ENABLE);

    m_main->StopSuspend();
    m_interface->DeleteControl(EVENT_WINDOW9);
    m_dialogOpen = false;
}


// Specifies whether a dialog is displayed.

bool CMainDialog::IsDialog()
{
    return m_dialogOpen;
}

} // namespace Ui
