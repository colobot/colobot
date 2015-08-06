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

#include "ui/screen/screen_setup_controls.h"

#include "common/config.h"

#include "app/app.h"

#include "common/settings.h"
#include "common/stringutils.h"

#include "graphics/engine/camera.h"

#include "ui/controls/button.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/key.h"
#include "ui/controls/label.h"
#include "ui/controls/scroll.h"
#include "ui/controls/window.h"

namespace Ui
{

const int KEY_VISIBLE = 6;      // number of visible keys redefinable

CScreenSetupControls::CScreenSetupControls()
{
}

void CScreenSetupControls::SetActive()
{
    m_tab = PHASE_SETUPc;
}

void CScreenSetupControls::CreateInterface()
{
    CWindow*        pw;
    CLabel*         pl;
    CCheck*         pc;
    CScroll*        ps;
    CButton*        pb;
    CGroup*         pg;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pos.x = ox+sx*3;
    pos.y = 320.0f/480.0f;
    ddim.x = dim.x*15.0f;
    ddim.y = 18.0f/480.0f;
    GetResource(RES_TEXT, RT_SETUP_KEY1, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO1, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    pos.x = ox+sx*3;
    pos.y = 302.0f/480.0f;
    ddim.x = dim.x*15.0f;
    ddim.y = 18.0f/480.0f;
    GetResource(RES_TEXT, RT_SETUP_KEY2, name);
    pl = pw->CreateLabel(pos, ddim, 0, EVENT_INTERFACE_KINFO2, name);
    pl->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);

    ddim.x = 428.0f/640.0f;
    ddim.y = 128.0f/480.0f;
    pos.x = 105.0f/640.0f;
    pos.y = 164.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 0, EVENT_INTERFACE_KGROUP);
    pg->ClearState(STATE_ENABLE);
    pg->SetState(STATE_DEAD);
    pg->SetState(STATE_SHADOW);

    ddim.x =  18.0f/640.0f;
    ddim.y = (20.0f/480.0f)*KEY_VISIBLE;
    pos.x = 510.0f/640.0f;
    pos.y = 168.0f/480.0f;
    ps = pw->CreateScroll(pos, ddim, -1, EVENT_INTERFACE_KSCROLL);
    ps->SetVisibleRatio(static_cast<float>(KEY_VISIBLE/INPUT_SLOT_MAX));
    ps->SetArrowStep(1.0f/(static_cast<float>(INPUT_SLOT_MAX-KEY_VISIBLE)));
    UpdateKey();

    ddim.x = dim.x*6;
    ddim.y = dim.y*0.5f;
    pos.x = ox+sx*3;
    pos.y = 130.0f/480.0f;
    pc = pw->CreateCheck(pos, ddim, -1, EVENT_INTERFACE_JOYSTICK);
    pc->SetState(STATE_SHADOW);

    ddim.x = dim.x*6;
    ddim.y = dim.y*1;
    pos.x = ox+sx*10;
    pos.y = oy+sy*2;
    pb = pw->CreateButton(pos, ddim, -1, EVENT_INTERFACE_KDEF);
    pb->SetState(STATE_SHADOW);

    UpdateSetupButtons();
}

bool CScreenSetupControls::EventProcess(const Event &event)
{
    if (!CScreenSetup::EventProcess(event)) return false;

    switch( event.type )
    {
        case EVENT_INTERFACE_KSCROLL:
            UpdateKey();
            break;

        case EVENT_INTERFACE_KDEF:
            CInput::GetInstancePointer()->SetDefaultInputBindings();
            UpdateKey();
            break;

        case EVENT_INTERFACE_JOYSTICK:
            m_app->SetJoystickEnabled(!m_app->GetJoystickEnabled());
            UpdateSetupButtons();
            break;

        default:
            if (event.type >= EVENT_INTERFACE_KEY && event.type <= EVENT_INTERFACE_KEY_END)
            {
                ChangeKey(event.type);
                UpdateKey();
                break;
            }
            return true;
    }
    return false;
}

// Updates the buttons during the setup phase.

void CScreenSetupControls::UpdateSetupButtons()
{
    CWindow*    pw;
    CCheck*     pc;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == 0 )  return;

    pc = static_cast<CCheck*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK));
    if ( pc != 0 )
    {
        pc->SetState(STATE_ENABLE, m_app->GetJoystick().index >= 0);
        pc->SetState(STATE_CHECK, m_app->GetJoystickEnabled());
    }
}

// Updates the list of keys.

void CScreenSetupControls::UpdateKey()
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CScroll* ps = static_cast<CScroll*>(pw->SearchControl(EVENT_INTERFACE_KSCROLL));
    if (ps == nullptr) return;

    int first = static_cast<int>(ps->GetVisibleValue()*(INPUT_SLOT_MAX-KEY_VISIBLE));

    for (int i = 0; i < INPUT_SLOT_MAX; i++)
        pw->DeleteControl(static_cast<EventType>(EVENT_INTERFACE_KEY+i));

    Math::Point dim;
    dim.x = 400.0f/640.0f;
    dim.y =  20.0f/480.0f;
    Math::Point pos;
    pos.x = 110.0f/640.0f;
    pos.y = 168.0f/480.0f + dim.y*(KEY_VISIBLE-1);
    for (int i = 0; i < KEY_VISIBLE; i++)
    {
        pw->CreateKey(pos, dim, -1, static_cast<EventType>(EVENT_INTERFACE_KEY+first+i));
        CKey* pk = static_cast<CKey*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_KEY+first+i)));
        if (pk == nullptr) break;

        pk->SetBinding(CInput::GetInstancePointer()->GetInputBinding(static_cast<InputSlot>(first+i)));
        pos.y -= dim.y;
    }
}

// Change a key.

void CScreenSetupControls::ChangeKey(EventType event)
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CScroll* ps = static_cast<CScroll*>(pw->SearchControl(EVENT_INTERFACE_KSCROLL));
    if (ps == nullptr) return;

    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        if ( EVENT_INTERFACE_KEY+i == event )
        {
            CKey* pk = static_cast<CKey*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_KEY+i)));
            if (pk == nullptr) break;

            CInput::GetInstancePointer()->SetInputBinding(static_cast<InputSlot>(i), pk->GetBinding());
        }
    }
}

} // namespace Ui
