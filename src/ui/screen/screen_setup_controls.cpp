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

#include "ui/screen/screen_setup_controls.h"

#include "app/app.h"

#include "common/settings.h"

#include "ui/controls/button.h"
#include "ui/controls/check.h"
#include "ui/controls/editvalue.h"
#include "ui/controls/group.h"
#include "ui/controls/interface.h"
#include "ui/controls/key.h"
#include "ui/controls/label.h"
#include "ui/controls/list.h"
#include "ui/controls/scroll.h"
#include "ui/controls/window.h"

namespace Ui
{

const int KEY_VISIBLE = 8;      // number of visible keys redefinable

CScreenSetupControls::CScreenSetupControls()
     : CScreenSetup({
        // EVENT_INTERFACE_SETUPd,
        // EVENT_INTERFACE_SETUPg,
        // EVENT_INTERFACE_SETUPp,
        //EVENT_INTERFACE_SETUPc,
        EVENT_INTERFACE_SETUPs,

        EVENT_INTERFACE_KSCROLL,           //Scroll
        EVENT_INTERFACE_JOYSTICK,          //List
        EVENT_INTERFACE_JOYSTICK_X,
        EVENT_INTERFACE_JOYSTICK_X_INVERT,
        EVENT_INTERFACE_JOYSTICK_Y,
        EVENT_INTERFACE_JOYSTICK_Y_INVERT,
        EVENT_INTERFACE_JOYSTICK_Z,
        EVENT_INTERFACE_JOYSTICK_Z_INVERT,
        EVENT_INTERFACE_JOYSTICK_CAM_X,
        EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT,
        EVENT_INTERFACE_JOYSTICK_CAM_Y,
        EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT,
        EVENT_INTERFACE_JOYSTICK_CAM_Z,
        EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT,
        EVENT_INTERFACE_JOYSTICK_DEADZONE, //EditValue
        EVENT_INTERFACE_KDEF,              //Button

        EVENT_INTERFACE_BACK,
        EVENT_INTERFACE_SETUPd,
        EVENT_INTERFACE_SETUPg,
        EVENT_INTERFACE_SETUPp,
    })
{
    m_input = CInput::GetInstancePointer();
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
    CList*          pli;
    CEditValue*     pev;
    Math::Point     pos, ddim;
    std::string     name;

    CScreenSetup::CreateInterface();
    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

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

    ddim.x = 273.0f/640.0f;
    ddim.y = 168.0f/480.0f;
    pos.x = 105.0f/640.0f;
    pos.y = 124.0f/480.0f;
    pg = pw->CreateGroup(pos, ddim, 0, EVENT_INTERFACE_KGROUP);
    pg->ClearState(STATE_ENABLE);
    pg->SetState(STATE_DEAD);
    pg->SetState(STATE_SHADOW);

    ddim.x =  18.0f/640.0f;
    ddim.y = (20.0f/480.0f)*KEY_VISIBLE;
    pos.x = 355.0f/640.0f;
    pos.y = 128.0f/480.0f;
    ps = pw->CreateScroll(pos, ddim, -1, EVENT_INTERFACE_KSCROLL);
    ps->SetVisibleRatio(static_cast<float>(KEY_VISIBLE/INPUT_SLOT_MAX));
    ps->SetArrowStep(1.0f/(static_cast<float>(INPUT_SLOT_MAX-KEY_VISIBLE)));
    UpdateKey();

    ddim.x = 160.0f/640.0f;
    ddim.y = 80.0f/480.0f;
    pos.x = 400.0f/640.0f;
    pos.y = 273.0f/480.0f;
    pli = pw->CreateList(pos, ddim, 0, EVENT_INTERFACE_JOYSTICK);
    pli->SetState(STATE_SHADOW);

    ddim.x = dim.x*1.5f;
    ddim.y = 18.0f/480.0f;
    pos.y = 240.0f/480.0f;

    auto CreateJoystickControls = [&](const std::string& label, EventType bindingControl, EventType invertControl)
    {
        pos.y -= 20.0f/480.0f;
        pos.x = 390.0f/640.0f;
        pos.y -= 5.0f/480.0f;
        pw->CreateLabel(pos, ddim, 0, EVENT_LABEL0, label);
        pos.y += 5.0f/480.0f;
        pos.x = 442.0f/640.0f;
        pev = pw->CreateEditValue(pos, ddim, 0, bindingControl);
        pev->SetState(STATE_SHADOW);
        pev->SetType(EVT_INT);
        pev->SetMinValue(-1);
        pev->SetMaxValue(2);
        pev->SetStepValue(1);
        pev->SetValue(1);
        pos.x = 500.0f/640.0f;
        pc = pw->CreateCheck(pos, ddim, 0, invertControl);
        pc->SetState(STATE_SHADOW);
    };
    pos.y += 15.0f/480.0f;
    CreateJoystickControls("X:", EVENT_INTERFACE_JOYSTICK_X, EVENT_INTERFACE_JOYSTICK_X_INVERT);
    CreateJoystickControls("Y:", EVENT_INTERFACE_JOYSTICK_Y, EVENT_INTERFACE_JOYSTICK_Y_INVERT);
    CreateJoystickControls("Z:", EVENT_INTERFACE_JOYSTICK_Z, EVENT_INTERFACE_JOYSTICK_Z_INVERT);
    CreateJoystickControls("CamX:", EVENT_INTERFACE_JOYSTICK_CAM_X, EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT);
    CreateJoystickControls("CamY:", EVENT_INTERFACE_JOYSTICK_CAM_Y, EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT);
    CreateJoystickControls("CamZ:", EVENT_INTERFACE_JOYSTICK_CAM_Z, EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT);

    pos.y -= 25.0f/480.0f;
    pos.x = 420.0f/640.0f;
    pos.y -= 5.0f/480.0f;
    pw->CreateLabel(pos, ddim, 0, EVENT_LABEL3, "Deadzone:");
    pos.y += 5.0f/480.0f;
    pos.x = 480.0f/640.0f;
    ddim.x = dim.x*2.2f;
    pev = pw->CreateEditValue(pos, ddim, 0, EVENT_INTERFACE_JOYSTICK_DEADZONE);
    pev->SetState(STATE_SHADOW);
    pev->SetType(EVT_100);
    pev->SetMinValue(0);
    pev->SetMaxValue(1);
    pev->SetStepValue(0.01);

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
    if (!CScreenSetup::EventProcess(event))
        return false;
    // TODO : check if keyboard navigation there don't break anything

    switch( event.type )
    {
        case EVENT_INTERFACE_KSCROLL:
            UpdateKey();
            break;

        case EVENT_INTERFACE_KDEF:
            m_input->SetDefaultInputBindings();
            UpdateKey();
            break;

        case EVENT_INTERFACE_JOYSTICK_X_INVERT:
        case EVENT_INTERFACE_JOYSTICK_Y_INVERT:
        case EVENT_INTERFACE_JOYSTICK_Z_INVERT:
        case EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT:
        case EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT:
        case EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT:
            ToggleJoystickInvert(event.type);
            ChangeSetupButtons();
            UpdateSetupButtons();
            break;

        case EVENT_INTERFACE_JOYSTICK:
        case EVENT_INTERFACE_JOYSTICK_X:
        case EVENT_INTERFACE_JOYSTICK_Y:
        case EVENT_INTERFACE_JOYSTICK_Z:
        case EVENT_INTERFACE_JOYSTICK_CAM_X:
        case EVENT_INTERFACE_JOYSTICK_CAM_Y:
        case EVENT_INTERFACE_JOYSTICK_CAM_Z:
        case EVENT_INTERFACE_JOYSTICK_DEADZONE:
            ChangeSetupButtons();
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

void CScreenSetupControls::ChangeSetupButtons()
{
    CWindow*    pw;
    CList*      pli;
    CEditValue* pev;
    CCheck* pc;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if ( pw == nullptr )  return;

    pli = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK));
    if ( pli != nullptr )
    {
        if (pli->GetSelect() > 0)
        {
            m_app->SetJoystickEnabled(false);
            m_app->ChangeJoystick(m_app->GetJoystickList().at(pli->GetSelect()-1));
            m_app->SetJoystickEnabled(true);
        }
        else
        {
            m_app->SetJoystickEnabled(false);
        }
    }

    auto HandleJoystickControls = [&](JoyAxisSlot joyAxis, EventType bindingControl, EventType invertControl)
    {
        if (nullptr != (pev = static_cast<CEditValue*>(pw->SearchControl(bindingControl))))
        {
            JoyAxisBinding binding = m_input->GetJoyAxisBinding(joyAxis);
            binding.axis = static_cast<int>(round(pev->GetValue()));
            m_input->SetJoyAxisBinding(joyAxis, binding);
        }
        if (nullptr != (pc = static_cast<CCheck*>(pw->SearchControl(invertControl))))
        {
            JoyAxisBinding binding = m_input->GetJoyAxisBinding(joyAxis);
            binding.invert = pc->TestState(STATE_CHECK);
            m_input->SetJoyAxisBinding(joyAxis, binding);
        }
    };
    HandleJoystickControls(JOY_AXIS_SLOT_X, EVENT_INTERFACE_JOYSTICK_X, EVENT_INTERFACE_JOYSTICK_X_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_Y, EVENT_INTERFACE_JOYSTICK_Y, EVENT_INTERFACE_JOYSTICK_Y_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_Z, EVENT_INTERFACE_JOYSTICK_Z, EVENT_INTERFACE_JOYSTICK_Z_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_X, EVENT_INTERFACE_JOYSTICK_CAM_X, EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_Y, EVENT_INTERFACE_JOYSTICK_CAM_Y, EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_Z, EVENT_INTERFACE_JOYSTICK_CAM_Z, EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT);

    if (nullptr != (pev = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK_DEADZONE))))
    {
        m_input->SetJoystickDeadzone(pev->GetValue());
    }
}

void CScreenSetupControls::ToggleJoystickInvert(EventType type)
{
    CWindow* pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    CCheck* pc = static_cast<CCheck*>(pw->SearchControl(type));
    if (pc == nullptr) return;

    pc->SetState(STATE_CHECK, !pc->TestState(STATE_CHECK));
}

// Updates the buttons during the setup phase.

void CScreenSetupControls::UpdateSetupButtons()
{
    CWindow* pw;
    CList* pli;
    CEditValue* pev;
    CCheck* pc;

    pw = static_cast<CWindow*>(m_interface->SearchControl(EVENT_WINDOW5));
    if (pw == nullptr) return;

    pli = static_cast<CList*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK));
    if (pli != nullptr)
    {
        pli->Flush();
        pli->SetItemName(0, "[No joystick]");
        auto joysticks = m_app->GetJoystickList();
        for (unsigned int i = 0; i < joysticks.size(); i++)
        {
            pli->SetItemName(1 + i, joysticks[i].name);
        }
        pli->SetSelect(m_app->GetJoystickEnabled() ? m_app->GetJoystick().index + 1 : 0);
    }

    auto HandleJoystickControls = [&](JoyAxisSlot joyAxis, EventType bindingControl, EventType invertControl)
    {
        if (nullptr != (pev = static_cast<CEditValue*>(pw->SearchControl(bindingControl))))
        {
            pev->SetState(STATE_ENABLE, m_app->GetJoystickEnabled());
            pev->SetMaxValue(m_app->GetJoystick().axisCount-1);
            pev->SetValue(m_input->GetJoyAxisBinding(joyAxis).axis);
        }
        if (nullptr != (pc = static_cast<CCheck*>(pw->SearchControl(invertControl))))
        {
            pc->SetState(STATE_ENABLE, m_app->GetJoystickEnabled());
            pc->SetState(STATE_CHECK, m_input->GetJoyAxisBinding(joyAxis).invert);
        }
    };
    HandleJoystickControls(JOY_AXIS_SLOT_X, EVENT_INTERFACE_JOYSTICK_X, EVENT_INTERFACE_JOYSTICK_X_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_Y, EVENT_INTERFACE_JOYSTICK_Y, EVENT_INTERFACE_JOYSTICK_Y_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_Z, EVENT_INTERFACE_JOYSTICK_Z, EVENT_INTERFACE_JOYSTICK_Z_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_X, EVENT_INTERFACE_JOYSTICK_CAM_X, EVENT_INTERFACE_JOYSTICK_CAM_X_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_Y, EVENT_INTERFACE_JOYSTICK_CAM_Y, EVENT_INTERFACE_JOYSTICK_CAM_Y_INVERT);
    HandleJoystickControls(JOY_AXIS_SLOT_CAM_Z, EVENT_INTERFACE_JOYSTICK_CAM_Z, EVENT_INTERFACE_JOYSTICK_CAM_Z_INVERT);

    if (nullptr != (pev = static_cast<CEditValue*>(pw->SearchControl(EVENT_INTERFACE_JOYSTICK_DEADZONE))))
    {
        pev->SetState(STATE_ENABLE, m_app->GetJoystickEnabled());
        pev->SetValue(m_input->GetJoystickDeadzone());
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
    dim.x = 250.0f/640.0f;
    dim.y =  20.0f/480.0f;
    Math::Point pos;
    pos.x = 110.0f/640.0f;
    pos.y = 128.0f/480.0f + dim.y*(KEY_VISIBLE-1);
    for (int i = 0; i < KEY_VISIBLE; i++)
    {
        pw->CreateKey(pos, dim, -1, static_cast<EventType>(EVENT_INTERFACE_KEY+first+i));
        CKey* pk = static_cast<CKey*>(pw->SearchControl(static_cast<EventType>(EVENT_INTERFACE_KEY+first+i)));
        if (pk == nullptr) break;

        pk->SetBinding(m_input->GetInputBinding(static_cast<InputSlot>(first+i)));
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

            m_input->SetInputBinding(static_cast<InputSlot>(i), pk->GetBinding());
        }
    }
}

} // namespace Ui
