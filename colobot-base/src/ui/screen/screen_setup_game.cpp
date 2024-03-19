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

#include "ui/screen/screen_setup_game.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
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
    CList*          pli;
    glm::vec2       pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

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
    pos.x = ox+sx*3+dim.x*4.0f;
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
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_BGPAUSE);
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

    ddim.y = dim.y*3.0f;
    pos.y -= ddim.y;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_LANGUAGE);
    pli->SetState(STATE_SHADOW);
    // TODO: Add something like GetSupportedLanguages() and GetLanguageFriendlyName() for this
    pli->SetItemName(1+LANGUAGE_ENV, "[System default]");
    pli->SetItemName(1+LANGUAGE_CZECH, "Czech");
    pli->SetItemName(1+LANGUAGE_ENGLISH, "English");
    pli->SetItemName(1+LANGUAGE_FRENCH, "French");
    pli->SetItemName(1+LANGUAGE_GERMAN, "German");
    pli->SetItemName(1+LANGUAGE_POLISH, "Polish");
    pli->SetItemName(1+LANGUAGE_RUSSIAN, "Russian");
    pli->SetItemName(1+LANGUAGE_PORTUGUESE_BRAZILIAN, "Brazilian Portuguese");
    pli->SetItemName(1+LANGUAGE_SIMPLIFIED_CHINESE, "Simplified Chinese");

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
            m_settings->SetMouseParticlesEnabled(!m_settings->GetMouseParticlesEnabled());
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_BGPAUSE:
            m_settings->SetFocusLostPause(!m_settings->GetFocusLostPause());
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
            m_camera->SetOldCameraScroll(!m_camera->GetOldCameraScroll());
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
        case EVENT_INTERFACE_AUTOSAVE_SLOTS:
        case EVENT_INTERFACE_LANGUAGE:
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
    CList*      pli;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_TOOLTIP));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetTooltips());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_GLINT));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetInterfaceGlint());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_RAIN));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetMouseParticlesEnabled());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_BGPAUSE));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetFocusLostPause());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITMODE));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentMode());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EDITVALUE));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_engine->GetEditIndentValue()>2);
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SOLUCE4));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetSoluce4());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_MOVIES));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_settings->GetMovies());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_SCROLL));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_camera->GetOldCameraScroll());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTX));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertX());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_INVERTY));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_camera->GetCameraInvertY());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_EFFECT));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_camera->GetEffect());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_BLOOD));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_camera->GetBlood());
    }

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_ENABLE));
    if ( pc != nullptr )
    {
        pc->SetState(STATE_CHECK, m_main->GetAutosave());
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != nullptr )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
        ps->SetVisibleValue(m_main->GetAutosaveInterval());

    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != nullptr )
    {
        ps->SetState(STATE_ENABLE, m_main->GetAutosave());
        ps->SetVisibleValue(m_main->GetAutosaveSlots());
    }

    pli = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LANGUAGE));
    if ( pli != nullptr )
    {
        pli->SetSelect(1+m_settings->GetLanguage());
    }
}

// Updates the engine function of the buttons after the setup phase.

void CScreenSetupGame::ChangeSetupButtons()
{
    CWindow*    pw;
    CSlider*    ps;
    CList*      pli;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_INTERVAL));
    if ( ps != nullptr )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveInterval(static_cast<int>(round(value)));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_AUTOSAVE_SLOTS));
    if ( ps != nullptr )
    {
        value = ps->GetVisibleValue();
        m_main->SetAutosaveSlots(static_cast<int>(round(value)));
    }

    pli = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_LANGUAGE));
    if ( pli != nullptr )
    {
        m_settings->SetLanguage(static_cast<Language>(pli->GetSelect()-1));
        // TODO: A really ugly way to apply the change immediately
        m_main->ChangePhase(m_main->GetPhase());
    }
}

} // namespace Ui
