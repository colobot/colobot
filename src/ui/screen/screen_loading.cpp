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

#include "ui/screen/screen_loading.h"

#include "app/app.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenLoading::CScreenLoading()
    : m_loadingCounter(0)
{
}

void CScreenLoading::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CGroup*         pg;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x  = 0.35f;
    pos.y  = 0.10f;
    ddim.x = 0.30f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 10, EVENT_WINDOW5);

    pw->SetName(" ");

    pos.x  = 0.35f;
    pos.y  = 0.60f;
    ddim.x = 0.30f;
    ddim.y = 0.30f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x  = 0.35f;
    pos.y  = 0.10f;
    ddim.x = 0.30f;
    ddim.y = 0.30f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x  = 254.0f/640.0f;
    pos.y  = 208.0f/480.0f;
    ddim.x = 132.0f/640.0f;
    ddim.y =  42.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 22, EVENT_NULL);
    pg->SetState(STATE_SHADOW);

    pos.x  = 220.0f/640.0f;
    pos.y  = 210.0f/480.0f;
    ddim.x = 200.0f/640.0f;
    ddim.y =  20.0f/480.0f;
    GetResource(RES_TEXT, RT_DIALOG_LOADING, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetFontSize(12.0f);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_CENTER);

    SetBackground("textures/interface/interface.png");

    m_loadingCounter = 1;  // enough time to display!
}

bool CScreenLoading::EventProcess(const Event &event)
{
    if ( m_loadingCounter == 0 )
    {
        m_main->ChangePhase(PHASE_SIMUL);
    }
    m_loadingCounter --;
    return true;
}

} // namespace Ui
