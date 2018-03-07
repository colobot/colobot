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

#include "ui/screen/screen.h"

#include "app/app.h"

#include "common/version.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "ui/controls/edit.h"
#include "ui/controls/editvalue.h"
#include "ui/controls/interface.h"
#include "ui/controls/label.h"
#include "ui/controls/window.h"

namespace Ui
{

CScreen::CScreen()
    : m_windowOwnerEvt(EVENT_WINDOW5)
{
    assert(false);
}

CScreen::CScreen(const EventType windowOwnerEvt,
        const std::vector<EventType> tabOrder)
    : m_windowOwnerEvt(windowOwnerEvt)
    , m_iCurrentSelectedItem(0)
    , m_tabOrder(tabOrder)
{
    m_main       = CRobotMain::GetInstancePointer();
    m_interface  = m_main->GetInterface();
    m_app        = CApplication::GetInstancePointer();
    m_eventQueue = m_app->GetEventQueue();
    m_engine     = Gfx::CEngine::GetInstancePointer();
    m_sound      = m_app->GetSound();
}

CScreen::~CScreen()
{
}

void CScreen::SetBackground(const std::string& filename, bool scaled)
{
    m_engine->SetBackground(filename,
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f),
            true, scaled);
    m_engine->SetBackForce(true);
}

void CScreen::CreateVersionDisplay()
{
    CWindow* pw = static_cast<CWindow*>(
        m_interface->SearchControl(m_windowOwnerEvt));
    if (pw != nullptr)
    {
        Math::Point pos, ddim;

        pos.x  = 540.0f/640.0f;
        pos.y  =   9.0f/480.0f;
        ddim.x =  90.0f/640.0f;
        ddim.y =  10.0f/480.0f;
        CLabel* pl = pw->CreateLabel(pos, ddim, 0, EVENT_LABEL1, COLOBOT_VERSION_DISPLAY);
        pl->SetFontType(Gfx::FONT_COURIER);
        pl->SetFontSize(9.0f);
    }
}

/**
 * Is given item activable (into current window)
 * @param  item : window control
 * @return      : item exist and is activable (existing, enabled & visible)
 */
bool CScreen::IsItemEnabled(const EventType item)const
{
    CWindow* pw = static_cast<CWindow*>(
        m_interface->SearchControl(m_windowOwnerEvt));
    if ( pw == nullptr )
        return false;
    CControl* pc = pw->SearchControl(item);
    if ( pc != nullptr)
        return (STATE_VISIBLE | STATE_ENABLE)
            == (pc->GetState() & (STATE_VISIBLE | STATE_ENABLE));
    return false;
}

/**
 * Display current active ctrl highlighted (and reset other highlight)
 *  + manage selection modif (tab-stops - taborder)
 * @param slide         optionnal slide : 0:none, 1:next, -1: previous
 * @param bUnselectEdit undisplay carret on eventual other edit areas
 *  or let carret to opt CEdit in case of need
 */
void CScreen::DisplayActive(const short slide, const bool bUnselectEdit)
{
    // chg current control
    m_iCurrentSelectedItem+=slide;
    if(static_cast<short>(m_tabOrder.size())<=m_iCurrentSelectedItem)
        m_iCurrentSelectedItem=0;
    else if(0>m_iCurrentSelectedItem)
        m_iCurrentSelectedItem=m_tabOrder.size()-1;
    //check ctrl activable (or try next...)
    if (!IsItemEnabled(m_tabOrder[m_iCurrentSelectedItem]))
    {
        if(slide)
        {
            DisplayActive(slide, bUnselectEdit);
            return;
        }
        else
            m_iCurrentSelectedItem=0;
    }
    //update highLights
    CWindow* pw = static_cast<CWindow*>(
        m_interface->SearchControl(m_windowOwnerEvt));
    if (nullptr==pw)
        return;
    CControl* pc;
    CEdit *pe;
    CEditValue *pev;
    CControl* pcCur = pw->SearchControl(m_tabOrder[m_iCurrentSelectedItem]);
    short iPlace=0;
    for(EventType id:m_tabOrder)
    {
        pc = pw->SearchControl(id);
        if(nullptr!=pc)
        {
            pc->SetState(STATE_HILIGHT,iPlace==m_iCurrentSelectedItem);
            pe=dynamic_cast<CEdit*>(pc);
            pev=dynamic_cast<CEditValue*>(pc);
            if(((nullptr==pe && nullptr==pev) || bUnselectEdit)
                || iPlace==m_iCurrentSelectedItem)
                pc->SetFocus(pcCur);
        }
        ++iPlace;
    }
}

/**
 * manage key navigation & activation into current window
 *     tab, shift+tab  : navigation :(front & rear depending on taborder)
 *     space           : activate current control
 *     alt             : highlight current activale control
 * @param  event         key event
 * @param  bUnselectEdit undisplay carret on eventual other edit areas
 * @return               false if event is managed there
 */
bool CScreen::EventProcessTabStop(const Event &event, const bool bUnselectEdit)
{
    if (EVENT_KEY_DOWN !=  event.type)
        return true;    //non managed there
    if (event.kmodState & KEY_MOD(ALT))
        DisplayActive(0);
    switch (event.GetData<KeyEventData>()->key)
    {
    case KEY(TAB):
        DisplayActive(event.kmodState & KEY_MOD(SHIFT) ? -1 : 1, bUnselectEdit);
        return false;   //managed
    case KEY(SPACE):
        if(IsItemEnabled(m_tabOrder[m_iCurrentSelectedItem]))
            return EventProcess(Event(m_tabOrder[m_iCurrentSelectedItem]));
    }
    return true;    //non managed
}

} // namespace Ui
