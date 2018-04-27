/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "ui/screen/screen_io_write.h"

#include "app/app.h"

#include "common/restext.h"

#include "level/robotmain.h"

#include "ui/controls/button.h"
#include "ui/controls/edit.h"
#include "ui/controls/image.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreenIOWrite::CScreenIOWrite(CScreenLevelList* screenLevelList)
    : CScreenIO(screenLevelList)
{
}

void CScreenIOWrite::CreateInterface()
{
    CWindow*        pw;
    CButton*        pb;
    CList*          pli;
    CLabel*         pl;
    CImage*         pi;
    CEdit*          pe;
    Math::Point     pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 13, EVENT_WINDOW5);
    pw->SetClosable(true);
    GetResource(RES_TEXT, RT_TITLE_WRITE, name);
    pw->SetName(name);

    pos.x  = 0.10f;
    pos.y  = 0.40f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 5, EVENT_INTERFACE_GLINTl);  // orange corner
    pos.x  = 0.40f;
    pos.y  = 0.10f;
    ddim.x = 0.50f;
    ddim.y = 0.50f;
    pw->CreateGroup(pos, ddim, 4, EVENT_INTERFACE_GLINTr);  // blue corner

    pos.x  = 290.0f/640.0f;
    ddim.x = 245.0f/640.0f;

    pos.y  = 146.0f/480.0f;
    ddim.y =  18.0f/480.0f;
    GetResource(RES_EVENT, EVENT_INTERFACE_IOLABEL, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_IOLABEL, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.y  = 130.0f/480.0f;
    ddim.y =  18.0f/480.0f;
    pe = pw->CreateEdit(pos, ddim, 0, EVENT_INTERFACE_IONAME);
    pe->SetState(STATE_SHADOW);
    pe->SetFontType(Gfx::FONT_COLOBOT);
    pe->SetMaxChar(35);
    IOReadName();

    pos.y  = 190.0f/480.0f;
    ddim.y = 190.0f/480.0f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
    pli->SetState(STATE_SHADOW);

    pos.y  = oy+sy*2;
    ddim.y = dim.y*1;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IOWRITE);
    pb->SetState(STATE_SHADOW);

    pos.x  = 105.0f/640.0f;
    pos.y  = 190.0f/480.0f;
    ddim.x = 170.0f/640.0f;
    ddim.y = dim.y*1;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IODELETE);
    pb->SetState(STATE_SHADOW);

    pos.x  = 105.0f/640.0f;
    pos.y  = 250.0f/480.0f;
    ddim.x = 170.0f/640.0f;
    ddim.y = 128.0f/480.0f;
    pi = pw->CreateImage(pos, ddim, 0, EVENT_INTERFACE_IOIMAGE);
    pi->SetState(STATE_SHADOW);

    ddim.x = dim.x*4;
    ddim.y = dim.y*1;
    pos.x  = ox+sx*3;
    pos.y  = oy+sy*2;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_BACK);
    pb->SetState(STATE_SHADOW);

    IOReadList(true);
    IOUpdateList(true);
}

bool CScreenIOWrite::EventProcess(const Event &event)
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return false;

    if ( event.type == pw->GetEventTypeClose() ||
            event.type == EVENT_INTERFACE_BACK   ||
            (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE)) )
    {
        m_interface->DeleteControl(EVENT_WINDOW5);
        m_main->ChangePhase(PHASE_SIMUL);
        m_main->StopSuspend();
        return false;
    }

    if ( event.type == EVENT_INTERFACE_IOLIST )
    {
        IOUpdateList(true);
        return false;
    }
    if ( event.type == EVENT_INTERFACE_IODELETE )
    {
        IODeleteScene();
        IOReadList(true);
        IOUpdateList(true);
        return false;
    }

    if ( event.type == EVENT_INTERFACE_IOWRITE )
    {
        m_main->ChangePhase(PHASE_SIMUL);
        m_main->StopSuspend();
        IOWriteScene();
        return false;
    }

    return true;
}

} // namespace Ui
