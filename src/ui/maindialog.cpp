/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include "common/make_unique.h"
#include "common/settings.h"

#include "level/player_profile.h"
#include "level/robotmain.h"

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
    : m_iCurrentSelectedItem(0)
    , m_tabOrder    //specific for PauseMenu
        {
            EVENT_DIALOG_CANCEL,    //0
            EVENT_INTERFACE_WRITE,  //opt 1
            EVENT_INTERFACE_READ,   //opt 2
            EVENT_INTERFACE_SETUP,  //3
            EVENT_INTERFACE_AGAIN,  //4
            EVENT_DIALOG_OK         //5
        }
{
    m_main       = CRobotMain::GetInstancePointer();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_particle   = m_engine->GetParticle();
    m_interface  = m_main->GetInterface();
    m_sound      = CApplication::GetInstancePointer()->GetSound();
    m_settings   = CSettings::GetInstancePointer();

    m_dialogOpen = false;
    m_dialogType = {};  // ?? enum: Question / PauseMenu
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
        if (EVENT_KEY_DOWN == event.type)
        {
            if( (event.kmodState & KEY_MOD(ALT) ) != 0 )
                DisplayActive(0);
            switch (event.GetData<KeyEventData>()->key)
            {
            case KEY(RETURN):
                pressedButton = EVENT_DIALOG_OK;
                break;
            case KEY(ESCAPE):
                pressedButton = EVENT_DIALOG_CANCEL;
                //nota: even on info/warning dialog
                break;
            case KEY(LEFT):
            case KEY(UP):
                DisplayActive(-1);
                return false;
            case KEY(RIGHT):
            case KEY(DOWN):
                DisplayActive(1);
                return false;
            case KEY(TAB):
                DisplayActive(event.kmodState & KEY_MOD(SHIFT) ? -1 : 1);
                return false;
            }
            if (INPUT_SLOT_ACTION==event.GetData<KeyEventData>()->slot)
                pressedButton = EVENT_DIALOG_OK;
        }

        if ( m_dialogType == DialogType::PauseMenu )
        {
            // 0     : EVENT_DIALOG_CANCEL
            // opt 1 : EVENT_INTERFACE_WRITE
            // opt 2 : EVENT_INTERFACE_READ
            // 3     : EVENT_INTERFACE_SETUP
            // 4     : EVENT_INTERFACE_AGAIN
            // 5     : EVENT_DIALOG_OK
            if (EVENT_KEY_DOWN == event.type)
            {
                switch (event.GetData<KeyEventData>()->key)
                {
                case KEY(SPACE):
                    switch(m_iCurrentSelectedItem)
                    {
                    case 1: //opt
                        pressedButton = EVENT_INTERFACE_WRITE;
                        break;
                    case 2: //opt
                        pressedButton = EVENT_INTERFACE_READ;
                        break;
                    case 3:
                        pressedButton = EVENT_INTERFACE_SETUP;
                        break;
                    case 4:
                        pressedButton = EVENT_INTERFACE_AGAIN;
                        break;
                    case 5:
                        pressedButton = EVENT_DIALOG_OK;
                        break;
                    default:    //case 0:
                        pressedButton = EVENT_DIALOG_CANCEL;
                    }
                    break;
                case KEY(s):
                    pressedButton = EVENT_INTERFACE_SETUP;
                    break;
                case KEY(q):
                    pressedButton = EVENT_DIALOG_OK;
                    break;
                //case KEY(RETURN): //filter finally wantted
                case KEY(ESCAPE):
                    break;  //don't filter
                //space let for shortcut-keys
                default:
                    return false;   //try to capture KeyBoard !
                }
            }
            if ( pressedButton == EVENT_DIALOG_CANCEL )
                StopDialog();

            if ( pressedButton == EVENT_INTERFACE_SETUP )
            {
                StopDialog();
                m_main->StartSuspend();
                m_main->ChangePhase(static_cast<const Phase>
                    (CScreenSetup::GetTab() + PHASE_SETUPds - PHASE_SETUPd));
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
        else    //  DialogType::Question
        {
            assert ( m_dialogType == DialogType::Question );
            //  0    : EVENT_DIALOG_OK
            // opt 1 : EVENT_DIALOG_CANCEL

            if (EVENT_KEY_DOWN == event.type)
            {
                if( (event.kmodState & KEY_MOD(ALT) ) != 0 )
                    DisplayActive(0);
                switch (event.GetData<KeyEventData>()->key)
                {
                case KEY(SPACE):
                    if(0==m_iCurrentSelectedItem)
                        pressedButton = EVENT_DIALOG_OK;
                    else
                        pressedButton = EVENT_DIALOG_CANCEL;
                    break;
                case KEY(RETURN):
                case KEY(ESCAPE):
                    break;  //don't filter
                default:
                    return false;   //try to capture KeyBoard !
                }
            }
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
    Math::Point     pos, dim;
    std::string name;

    StartDialog(Math::Point(0.3f, 0.8f), true);
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
    Math::Point pos, dim, ddim;
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
        m_iCurrentSelectedItem=1;
    }
    else
        m_iCurrentSelectedItem=0;

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
        m_iCurrentSelectedItem=0;
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
    Math::Point pos, dim, ddim;
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
    pl->SetFontType(Gfx::FONT_COLOBOT_BOLD);
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
        pb->SetState(STATE_WARNING);
}

// Beginning of displaying a dialog.

void CMainDialog::StartDialog(Math::Point dim, bool fireParticles)
{
    CWindow*    pw;
    Math::Point pos, ddim;

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
    Math::Vector    pos, speed;
    Math::Point     dim, dpos, ddim;
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

/**
 * display current active button highlighted (and reset other highlight)
 *  + manage current button modif (tab-stops)
 * @param slide : optionnal slide : 0:none, 1:next, -1: previous
 */
void CMainDialog::DisplayActive(const short slide)
{
    if(!m_dialogOpen)
        return ;
    assert(DialogType::PauseMenu == m_dialogType
        || DialogType::Question == m_dialogType);

    const short nbCtrl =
        DialogType::PauseMenu == m_dialogType
        ? m_tabOrder.size()
        : 2;    //  m_dialogType == DialogType::Question

    CButton* pb;
    CWindow* pw = static_cast<CWindow*>
        (m_interface->SearchControl(EVENT_WINDOW9));
    if ( pw == nullptr )
        return;
    m_iCurrentSelectedItem+=slide;
    if(nbCtrl-1<m_iCurrentSelectedItem)
        m_iCurrentSelectedItem=0;
    else if(0>m_iCurrentSelectedItem)
        m_iCurrentSelectedItem=nbCtrl-1;

    if(DialogType::Question == m_dialogType)
    {
        // 0    : EVENT_DIALOG_OK
        //opt 1 : EVENT_DIALOG_CANCEL
        pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_CANCEL));
        if ( pb == nullptr )
            //in fact, info/warning box !
            m_iCurrentSelectedItem=0;
        else
            // a real yes/no dialog !
            pb->SetState(STATE_HILIGHT,1==m_iCurrentSelectedItem);
        pb = static_cast<CButton*>(pw->SearchControl(EVENT_DIALOG_OK));
        if (pb != nullptr && pb->TestState(STATE_ENABLE))
            pb->SetState(STATE_HILIGHT,0==m_iCurrentSelectedItem);
    }
    else    //DialogType::PauseMenu
    {
        //check ctrl activable (or try next...)
        CControl* pc = pw->SearchControl(m_tabOrder[m_iCurrentSelectedItem]);
        if (nullptr==pc || !pc->TestState(STATE_ENABLE))
        {
            if (slide)
            {
                DisplayActive(slide);
                return;
            }
            else
                m_iCurrentSelectedItem=0;
        }
        //update highLights
        short iPlace=0;
        for (EventType id:m_tabOrder)
        {
            pc = pw->SearchControl(id);
            if (nullptr!=pc)
            {
                pc->SetState(STATE_HILIGHT,iPlace==m_iCurrentSelectedItem);
                if (iPlace==m_iCurrentSelectedItem)
                    pc->SetFocus(pc);
                else
                    pc->SetFocus(nullptr);
            }
            ++iPlace;
        }
    }
}

} // namespace Ui
