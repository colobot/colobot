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

#include "ui/screen/screen_setup_sound.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"

#include "sound/sound.h"

#include "ui/controls/button.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/slider.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenSetupSound::CScreenSetupSound()
    : CScreenSetup({
        // EVENT_INTERFACE_SETUPd,
        // EVENT_INTERFACE_SETUPg,
        // EVENT_INTERFACE_SETUPp,
        // EVENT_INTERFACE_SETUPc,
        // EVENT_INTERFACE_SETUPs,  //0

        EVENT_INTERFACE_VOLSOUND,
        EVENT_INTERFACE_VOLMUSIC,
        EVENT_INTERFACE_SILENT,
        EVENT_INTERFACE_NOISY,

        EVENT_INTERFACE_BACK,
        EVENT_INTERFACE_SETUPd,
        EVENT_INTERFACE_SETUPg,
        EVENT_INTERFACE_SETUPp,
        EVENT_INTERFACE_SETUPc,
        })
{
}

void CScreenSetupSound::SetActive()
{
    m_tab = PHASE_SETUPs;
}

void CScreenSetupSound::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CSlider*        psl;
    CButton*        pb;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pos.x = ox+sx*3;
    pos.y = 0.55f;
    ddim.x = dim.x*4.0f;
    ddim.y = 18.0f/480.0f;
    psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLSOUND);
    psl->SetState(STATE_SHADOW);
    psl->SetLimit(0.0f, MAXVOLUME);
    psl->SetArrowStep(1.0f);
    pos.y += ddim.y;
    GetResource(RES_EVENT, EVENT_INTERFACE_VOLSOUND, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*3;
    pos.y = 0.40f;
    ddim.x = dim.x*4.0f;
    ddim.y = 18.0f/480.0f;
    psl = pw->CreateSlider(pos, ddim, 0, EVENT_INTERFACE_VOLMUSIC);
    psl->SetState(STATE_SHADOW);
    psl->SetLimit(0.0f, MAXVOLUME);
    psl->SetArrowStep(1.0f);
    pos.y += ddim.y;
    GetResource(RES_EVENT, EVENT_INTERFACE_VOLMUSIC, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    ddim.x = dim.x*3;
    ddim.y = dim.y*1;
    pos.x = ox+sx*10;
    pos.y = oy+sy*2;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_SILENT);
    pb->SetState(STATE_SHADOW);
    pos.x += ddim.x;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_NOISY);
    pb->SetState(STATE_SHADOW);

    UpdateSetupButtons();
}

bool CScreenSetupSound::EventProcess(const Event &event)
{
    if (!CScreenSetup::EventProcess(event))
        return false;
    switch( event.type )
    {
        case EVENT_INTERFACE_VOLSOUND:
        case EVENT_INTERFACE_VOLMUSIC:
            ChangeSetupButtons();
            break;

        case EVENT_INTERFACE_SILENT:
            m_sound->SetAudioVolume(0);
            m_sound->SetMusicVolume(0);
            UpdateSetupButtons();
            break;
        case EVENT_INTERFACE_NOISY:
            m_sound->SetAudioVolume(MAXVOLUME);
            m_sound->SetMusicVolume(MAXVOLUME*3/4);
            UpdateSetupButtons();
            break;

        default:
            return true;
    }
    return false;
}

// Updates the buttons during the setup phase.

void CScreenSetupSound::UpdateSetupButtons()
{
    CWindow*    pw;
    CSlider*    ps;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLSOUND));
    if ( ps != nullptr )
    {
        value = static_cast<float>(m_sound->GetAudioVolume());
        ps->SetVisibleValue(value);
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLMUSIC));
    if ( ps != nullptr )
    {
        value = static_cast<float>(m_sound->GetMusicVolume());
        ps->SetVisibleValue(value);
    }
}

// Updates the engine function of the buttons after the setup phase.

void CScreenSetupSound::ChangeSetupButtons()
{
    CWindow*    pw;
    CSlider*    ps;
    float       value;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLSOUND));
    if ( ps != nullptr )
    {
        value = ps->GetVisibleValue();
        m_sound->SetAudioVolume(static_cast<int>(value));
    }

    ps = static_cast<CSlider*>(pw->SearchControl(EVENT_INTERFACE_VOLMUSIC));
    if ( ps != nullptr )
    {
        value = ps->GetVisibleValue();
        m_sound->SetMusicVolume(static_cast<int>(value));
    }
}

} // namespace Ui
