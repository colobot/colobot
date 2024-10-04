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

#include "ui/screen/screen_quit.h"

#include "app/app.h"

#include "common/restext.h"
#include "common/stringutils.h"

#include "level/robotmain.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenQuit::CScreenQuit()
{
}

void CScreenQuit::CreateInterface()
{
    CWindow*        pw;
    CEdit*          pe;
    CLabel*         pl;
    CButton*        pb;
    glm::vec2       pos, ddim;
    std::string     name;

    pos.x  = 0.0f;
    pos.y  = 0.0f;
    ddim.x = 0.0f;
    ddim.y = 0.0f;
    pw = m_interface->CreateWindows(pos, ddim, -1, EVENT_WINDOW5);

    pos.x  =  80.0f/640.0f;
    pos.y  = 190.0f/480.0f;
    ddim.x = 490.0f/640.0f;
    ddim.y = 160.0f/480.0f;
    pe = pw->CreateEdit(pos, ddim, 0, EVENT_EDIT1);
    pe->SetGenericMode(true);
    pe->SetEditCap(false);
    pe->SetHighlightCap(false);
    pe->SetFontType(Gfx::FONT_STUDIO);
    pe->SetFontSize(Gfx::FONT_SIZE_SMALL);
    pe->ReadText(TempToPath(std::string("help/") + m_app->GetLanguageChar() + std::string("/authors.txt")));

    pos.x  =  40.0f/640.0f;
    pos.y  =  83.0f/480.0f;
    ddim.x = 246.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    GetResource(RES_TEXT, RT_GENERIC_DEV1, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, name);
    pl->SetFontType(Gfx::FONT_STUDIO);
    pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

    pos.y  =  13.0f/480.0f;
    GetResource(RES_TEXT, RT_GENERIC_DEV2, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL2, name);
    pl->SetFontType(Gfx::FONT_STUDIO);
    pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

    pos.x  = 355.0f/640.0f;
    pos.y  =  83.0f/480.0f;
    ddim.x = 246.0f/640.0f;
    ddim.y =  16.0f/480.0f;
    GetResource(RES_TEXT, RT_GENERIC_EDIT1, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, name);
    pl->SetFontType(Gfx::FONT_STUDIO);
    pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

    pos.y  =  13.0f/480.0f;
    GetResource(RES_TEXT, RT_GENERIC_EDIT2, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL4, name);
    pl->SetFontType(Gfx::FONT_STUDIO);
    pl->SetFontSize(Gfx::FONT_SIZE_SMALL);

    pos.x  = 306.0f/640.0f;
    pos.y  =  17.0f/480.0f;
    ddim.x =  30.0f/640.0f;
    ddim.y =  30.0f/480.0f;
    pb = pw->CreateButton(pos, ddim, 49, EVENT_INTERFACE_ABORT);
    pb->SetState(STATE_SHADOW);

    SetBackground("textures/interface/generico.png");
}

bool CScreenQuit::EventProcess(const Event &event)
{
    if ( event.type == EVENT_INTERFACE_ABORT )
    {
        m_main->ChangePhase(PHASE_MAIN_MENU);
        return false;
    }

    if ( event.type == EVENT_KEY_DOWN )
    {
        if ( event.GetData<KeyEventData>()->key == KEY(ESCAPE) )
        {
            m_main->ChangePhase(PHASE_MAIN_MENU);
            return false;
        }
        else
        {
            m_eventQueue->AddEvent(Event(EVENT_QUIT));
            return false;
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN )
    {
        m_eventQueue->AddEvent(Event(EVENT_QUIT));
        return false;
    }

    return true;
}

} // namespace Ui
