/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_setup.h"

#include "common/config.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/settings.h"
#include "core/stringutils.h"

#include "graphics/engine/camera.h"

#include "ui/controls/button.h"
#include "ui/controls/interface.h"
#include "ui/controls/window.h"

namespace Ui
{

Phase CScreenSetup::m_tab = PHASE_SETUPg;

CScreenSetup::CScreenSetup()
    : m_simulationSetup{false}
{
    m_settings = CSettings::GetInstancePointer();
    m_camera = m_main->GetCamera();
}

void CScreenSetup::SetInSimulation(bool simulationSetup)
{
    m_simulationSetup = simulationSetup;
}

Phase CScreenSetup::GetTab()
{
    return m_tab;
}

void CScreenSetup::CreateInterface()
{
    CWindow*        pw;
    CButton*        pb;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 12, EVENT_WINDOW5);
    pw->SetClosable(true);
    GetResource(RES_TEXT, RT_TITLE_SETUP, name);
    pw->SetName(name);

    pos.x  = 0.70f;
    pos.y  = 0.10f;
    ddim.x = 0.20f;
    ddim.y = 0.20f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x  = 0.10f;
    ddim.x = 0.80f;
    pos.y  = 0.76f;
    ddim.y = 0.05f;
    pw->CreateGroup(pos, ddim, 3, EVENT_NULL);  // transparent -> gray

    ddim.x = 0.78f/5-0.01f;
    ddim.y = 0.06f;
    pos.x = 0.115f;
    pos.y = 0.76f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPd);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPd));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPg);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPg));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPp);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPp));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPc);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPc));

    pos.x += ddim.x+0.01f;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SETUPs);
    pb->SetState(STATE_SHADOW);
    pb->SetState(STATE_CARD);
    pb->SetState(STATE_CHECK, (m_tab == PHASE_SETUPs));

    pos.x  = 0.10f;
    ddim.x = 0.80f;
    pos.y  = 0.34f;
    ddim.y = 0.42f;
    pw->CreateGroup(pos, ddim, 2, EVENT_INTERFACE_GLINTu);  // orange -> transparent
    pos.x  = 0.10f+(6.0f/640.0f);
    ddim.x = 0.80f-(11.0f/640.0f);
    pos.y  = 0.74f;
    ddim.y = 0.02f;
    pw->CreateGroup(pos, ddim, 1, EVENT_INTERFACE_GLINTb);  // orange bar

    ddim.x = dim.x*4;
    ddim.y = dim.y*1;
    pos.x = ox+sx*3;
    pos.y = oy+sy*2;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
    pb->SetState(STATE_SHADOW);

    if ( !m_simulationSetup )
    {
        SetBackground("textures/interface/interface.png");
        CreateVersionDisplay();
    }
}

bool CScreenSetup::EventProcess(const Event &event)
{
    if ( !m_simulationSetup )
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == nullptr )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE)) )
        {
            m_settings->SaveSettings();
            m_main->ChangePhase(PHASE_MAIN_MENU);
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                m_main->ChangePhase(PHASE_SETUPd);
                return false;

            case EVENT_INTERFACE_SETUPg:
                m_main->ChangePhase(PHASE_SETUPg);
                return false;

            case EVENT_INTERFACE_SETUPp:
                m_main->ChangePhase(PHASE_SETUPp);
                return false;

            case EVENT_INTERFACE_SETUPc:
                m_main->ChangePhase(PHASE_SETUPc);
                return false;

            case EVENT_INTERFACE_SETUPs:
                m_main->ChangePhase(PHASE_SETUPs);
                return false;

            default:
                break;
        }
    }
    else
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == nullptr )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE)) )
        {
            m_settings->SaveSettings();
            m_interface->DeleteControl(EVENT_WINDOW5);
            m_main->ChangePhase(PHASE_SIMUL);
            m_main->StopSuspend();
            return false;
        }

        switch( event.type )
        {
            case EVENT_INTERFACE_SETUPd:
                m_main->ChangePhase(PHASE_SETUPds);
                return false;

            case EVENT_INTERFACE_SETUPg:
                m_main->ChangePhase(PHASE_SETUPgs);
                return false;

            case EVENT_INTERFACE_SETUPp:
                m_main->ChangePhase(PHASE_SETUPps);
                return false;

            case EVENT_INTERFACE_SETUPc:
                m_main->ChangePhase(PHASE_SETUPcs);
                return false;

            case EVENT_INTERFACE_SETUPs:
                m_main->ChangePhase(PHASE_SETUPss);
                return false;

            default:
                break;
        }
    }

    return true;
}

} // namespace Ui
