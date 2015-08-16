/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_setup_game.h"

#include "app/app.h"

#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenSetupGame::CScreenSetupGame()
{
}

void CScreenSetupGame::SetActive()
{
    m_tab = PHASE_SETUPp;
}

void CScreenSetupGame::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CCheck*         pc;
    CSlider*        psl;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    ddim.x = dim.x*6;
    ddim.y = dim.y*0.5f;
    pos.x = ox+sx*3;
    pos.y = 0.65f;

    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOVIES);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_SCROLL);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVERTX);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_INVERTY);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EFFECT);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_BLOOD);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_ENABLE);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;

    pos.y -= ddim.y;
    ddim.x = dim.x*2.5f;
    psl = pw->CreateSlider(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_INTERVAL);
    psl->SetState(STATE_SHADOW);
    psl->SetLimit(1.0f, 30.0f);
    psl->SetArrowStep(1.0f);
    pos.y += ddim.y/2;
    GetResource(RES_EVENT, EVENT_INTERFACE_AUTOSAVE_INTERVAL, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
    pos.y -= ddim.y/2;
    pos.x = ox+sx*3+dim.x*3.5f;
    psl = pw->CreateSlider(pos, ddim, -1, EVENT_INTERFACE_AUTOSAVE_SLOTS);
    psl->SetState(STATE_SHADOW);
    psl->SetLimit(1.0f, 10.0f);
    psl->SetArrowStep(1.0f);
    pos.y += ddim.y/2;
    GetResource(RES_EVENT, EVENT_INTERFACE_AUTOSAVE_SLOTS, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
    pos.y -= ddim.y/2;

    ddim.x = dim.x*6;
    ddim.y = dim.y*0.5f;
    pos.x = ox+sx*10;
    pos.y = 0.65f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_TOOLTIP);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_GLINT);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_RAIN);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_MOUSE);
    pc->SetState(STATE_SHADOW);
    pos.y -= 0.048f;
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITMODE);
    pc->SetState(STATE_SHADOW);
    if ( m_simulationSetup )
    {
        pc->SetState(STATE_DEAD);
    }
    pos.y -= 0.048f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_EDITVALUE);
    pc->SetState(STATE_SHADOW);

    UpdateSetupButtons();
}

bool CScreenSetupGame::EventProcess(const Event &event)
{
    if (!CScreenSetup::EventProcess(event)) return false;

    switch( event.type )
    {
        case EVENT_INTERFACE_TOOLTIP:
            m_settings->SetTooltips(!m_settings->GetTooltips());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_GLINT:
            m_settings->SetInterfaceGlint(!m_settings->GetInterfaceGlint());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_RAIN:
            m_settings->SetInterfaceRain(!m_settings->GetInterfaceRain());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_MOUSE:
            m_settings->SetSystemMouse(!m_settings->GetSystemMouse());
            m_app->SetMouseMode(m_settings->GetSystemMouse() ? MOUSE_SYSTEM : MOUSE_ENGINE);

            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_EDITMODE:
            m_engine->SetEditIndentMode(!m_engine->GetEditIndentMode());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_EDITVALUE:
            if ( m_engine->GetEditIndentValue() == 2 )
            {
                m_engine->SetEditIndentValue(4);
            }
            else
            {
                m_engine->SetEditIndentValue(2);
            }
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SOLUCE4:
            m_settings->SetSoluce4(!m_settings->GetSoluce4());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_MOVIES:
            m_settings->SetMovies(!m_settings->GetMovies());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_SCROLL:
            m_camera->SetCameraScroll(!m_camera->GetCameraScroll());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_INVERTX:
            m_camera->SetCameraInvertX(!m_camera->GetCameraInvertX());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_INVERTY:
            m_camera->SetCameraInvertY(!m_camera->GetCameraInvertY());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_EFFECT:
            m_camera->SetEffect(!m_camera->GetEffect());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_BLOOD:
            m_camera->SetBlood(!m_camera->GetBlood());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_AUTOSAVE_ENABLE:
            m_main->SetAutosave(!m_main->GetAutosave());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_AUTOSAVE_INTERVAL:
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_AUTOSAVE_SLOTS:
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        default:
            return true;
    }
    return false;
}

// Updates the buttons during the setup phase.

void CScreenSetupGame::UpdateSetupButtons()
{
    CWindow*    pw;
    CCheck*     pc;
    CSlider*    ps;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_TOOLTIP));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetTooltips());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_GLINT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetInterfaceGlint());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_RAIN));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetInterfaceRain());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOUSE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetSystemMouse());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITMODE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITVALUE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentValue()>2);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE4));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetSoluce4());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOVIES));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_settings->GetMovies());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SCROLL));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraScroll());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTX));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertX());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTY));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertY());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EFFECT));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetEffect());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_BLOOD));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_camera->GetBlood());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_ENABLE));
    if ( pc != 0 )
    {
        pc->SetState(STATE_CHECK, m_main->GetAutosave());
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
        ps->SetVisibleValue(m_main->GetAutosaveInterval());

    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != 0 )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
        ps->SetVisibleValue(m_main->GetAutosaveSlots());
    }
}

// Updates the engine function of the buttons after the setup phase.

void CScreenSetupGame::ChangeSetupButtons()
{
    CWindow*    pw;
    CSlider*    ps;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveInterval(static_cast<int>(round(value)));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != 0 )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveSlots(static_cast<int>(round(value)));
    }
}

} // namespace Ui
