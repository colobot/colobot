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

#include "ui/screen/screen_io_read.h"

#include "app/app.h"

#include "common/restext.h"

#include "level/robotmain.h"
#include "level/player_profile.h"

#include "ui/controls/button.h"
#include "ui/controls/image.h"
#include "ui/controls/interface.h"
#include "ui/controls/list.h"
#include "ui/controls/window.h"
#include "ui/maindialog.h"

namespace Ui
{

CScreenIORead::CScreenIORead(CScreenLevelList* screenLevelList, CMainDialog* dialog)
    : CScreenIO(screenLevelList),
      m_inSimulation(false),
      m_dialog(dialog)
{
}

void CScreenIORead::SetInSimulation(bool inSimulation)
{
    m_inSimulation = inSimulation;
}

void CScreenIORead::CreateInterface()
{
    CWindow*        pw;
    CButton*        pb;
    CList*          pli;
    CImage*         pi;
    glm::vec2       pos, ddim;
    std::string     name;

    pos.x = 0.10f;
    pos.y = 0.10f;
    ddim.x = 0.80f;
    ddim.y = 0.80f;
    pw = m_interface->CreateWindows(pos, ddim, 14, EVENT_WINDOW5);
    pw->SetClosable(true);
    GetResource(RES_TEXT, RT_TITLE_READ, name);
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

    pos.y  = 160.0f/480.0f;
    ddim.y = 190.0f/480.0f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_IOLIST);
    pli->SetState(STATE_SHADOW);

    pos.y  = oy+sy*2;
    ddim.y = dim.y*1;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IOREAD);
    pb->SetState(STATE_SHADOW);
    if ( m_inSimulation )
    {
        pb->SetState(STATE_WARNING);
    }

    pos.x  = 105.0f/640.0f;
    pos.y  = 160.0f/480.0f;
    ddim.x = 170.0f/640.0f;
    ddim.y = dim.y*1;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_IODELETE);
    pb->SetState(STATE_SHADOW);

    pos.x  = 105.0f/640.0f;
    pos.y  = 220.0f/480.0f;
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

    IOReadList(false);
    IOUpdateList(false);

    if ( !m_inSimulation )
    {
        SetBackground("textures/interface/interface.png");
        CreateVersionDisplay();
    }
}

bool CScreenIORead::EventProcess(const Event &event)
{
    if (!m_inSimulation)
    {
        CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
        if ( pw == nullptr )  return false;

        if ( event.type == pw->GetEventTypeClose() ||
                event.type == EVENT_INTERFACE_BACK   ||
                (event.type == EVENT_KEY_DOWN && event.GetData<KeyEventData>()->key == KEY(ESCAPE)) )
        {
            m_main->ChangePhase(PHASE_LEVEL_LIST);
            return false;
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
            m_interface->DeleteControl(EVENT_WINDOW5);
            m_main->ChangePhase(PHASE_SIMUL);
            m_main->StopSuspend();
            return false;
        }
    }

    if ( event.type == EVENT_INTERFACE_IOLIST )
    {
        IOUpdateList(false);
        return false;
    }
    if ( event.type == EVENT_INTERFACE_IODELETE )
    {
        IODeleteScene();
        IOReadList(false);
        IOUpdateList(false);
        return false;
    }

    if ( event.type == EVENT_INTERFACE_IOREAD )
    {
        auto doRead = [&]() {
            if(IOReadScene() && m_inSimulation)
            {
                m_main->StopSuspend();
                m_main->ChangePhase(PHASE_SIMUL);
            }
        };
        std::optional<std::filesystem::path> name = GetSceneName();
        if (!name.has_value()) return false;
        if (IsVersionSaveSupported(name.value()))
        {
            doRead();
        }
        else
        {
            m_dialog->StartQuestion("Version unsupported. Open anyway?", false, false, false, doRead);
        }
        return false;
    }

    return true;
}

} // namespace Ui
