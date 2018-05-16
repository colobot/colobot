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


#include "ui/mainshort.h"

#include "app/app.h"
#include "app/pausemanager.h"

#include "common/logger.h"

#include "level/robotmain.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/controllable_object.h"
#include "object/interface/programmable_object.h"

#include "ui/controls/interface.h"
#include "ui/controls/shortcut.h"

#include <algorithm>


namespace Ui
{

// Constructor of the application card.

CMainShort::CMainShort()
{
    m_event     = CApplication::GetInstancePointer()->GetEventQueue();
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_main      = CRobotMain::GetInstancePointer();
    m_interface = m_main->GetInterface();

    m_shortcuts.clear();

    m_bBuilding = false;
}

// Destructor of the application card.

CMainShort::~CMainShort()
{
}



void CMainShort::SetMode(bool bBuilding)
{
    m_bBuilding = bBuilding;
}

// Interface creates shortcuts to the units.

bool CMainShort::CreateShortcuts()
{
    Math::Point     pos, dim;

    if ( m_main->GetFixScene() )  return false;

    // Delete all old controls
    m_interface->DeleteControl(EVENT_OBJECT_MOVIELOCK);
    m_interface->DeleteControl(EVENT_OBJECT_EDITLOCK);
    m_interface->DeleteControl(EVENT_OBJECT_SAVING);
    m_interface->DeleteControl(EVENT_OBJECT_SHORTCUT_MODE);
    for (int i = EVENT_OBJECT_SHORTCUT; i <= EVENT_OBJECT_SHORTCUT_MAX; i++)
    {
        m_interface->DeleteControl(static_cast<EventType>(i));
    }
    m_shortcuts.clear();


    Math::IntPoint size = m_engine->GetWindowSize();
    float ratio = static_cast<float>(size.y) / static_cast<float>(size.x);

    // Display pause / movie indicator
    dim.y = 28.0f/480.0f;
    dim.x = dim.y*ratio;
    pos.x =  4.0f/640.0f;
    pos.y = (480.0f-32.0f)/480.0f;

    m_interface->CreateShortcut(pos, dim, 58, EVENT_OBJECT_SAVING);

    if ( m_main->GetMovieLock() &&
        !m_main->GetEditLock()  )  // hangs during film?
    {
        m_interface->CreateShortcut(pos, dim, 128+7, EVENT_OBJECT_MOVIELOCK);
        return true;
    }
    if ( !m_main->GetPauseManager()->IsPauseType(PAUSE_PHOTO) &&
         (m_main->GetEditLock() ||
          m_engine->GetPause()) )  // hangs during edition?
    {
        m_interface->CreateShortcut(pos, dim, 128+6, EVENT_OBJECT_EDITLOCK);
    }
    if (m_main->GetPauseManager()->IsPauseType(PAUSE_PHOTO) && m_main->GetSelect() == nullptr)
    {
        return true;
    }

    if (m_main->GetPauseManager()->IsPauseType(PAUSE_HIDE_SHORTCUTS))
        return true;

    // Create new shortcuts

    m_interface->CreateShortcut(pos, dim, 128+2, EVENT_OBJECT_SHORTCUT_MODE);
    pos.x += dim.x*1.2f;

    std::vector<int> teams;
    for (CObject* object : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (!object->GetDetectable())
            continue;

        if(GetShortcutIcon(object->GetType()) == -1)
            continue;

        if(std::find(teams.begin(), teams.end(), object->GetTeam()) == teams.end())
            teams.push_back(object->GetTeam());
    }
    std::sort(teams.begin(), teams.end());

    std::vector<Math::Point> positions;
    for(unsigned int i = 0; i < teams.size(); i++)
    {
        positions.push_back(pos);
        pos.y -= dim.y;
    }

    int rank = 0;
    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !pObj->GetDetectable() )  continue;
        if ( pObj->Implements(ObjectInterfaceType::Controllable) && !dynamic_cast<CControllableObject*>(pObj)->GetSelectable() )  continue;
        if ( pObj->GetProxyActivate() )  continue;

        int icon = GetShortcutIcon(pObj->GetType());
        if ( icon == -1 )  continue;

        unsigned int teamIndex = std::find(teams.begin(), teams.end(), pObj->GetTeam()) - teams.begin();

        CShortcut* shortcut = m_interface->CreateShortcut(positions[teamIndex], dim, icon, static_cast<EventType>(EVENT_OBJECT_SHORTCUT+rank));
        positions[teamIndex].x += dim.x;
        m_shortcuts.push_back(pObj);

        shortcut->SetTooltip(pObj->GetTooltipText());

        rank ++;

        if (rank > EVENT_OBJECT_SHORTCUT_MAX-EVENT_OBJECT_SHORTCUT)
        {
            GetLogger()->Warn("Not enough shortcut slots!\n");
            break;
        }
    }

    UpdateShortcuts();
    return true;
}

int CMainShort::GetShortcutIcon(ObjectType type)
{
    int icon = -1;
    if ( m_bBuilding )
    {
        if ( type == OBJECT_FACTORY  )  icon = 32;
        if ( type == OBJECT_DERRICK  )  icon = 33;
        if ( type == OBJECT_CONVERT  )  icon = 34;
        if ( type == OBJECT_RESEARCH )  icon = 35;
        if ( type == OBJECT_STATION  )  icon = 36;
        if ( type == OBJECT_TOWER    )  icon = 37;
        if ( type == OBJECT_LABO     )  icon = 38;
        if ( type == OBJECT_ENERGY   )  icon = 39;
        if ( type == OBJECT_RADAR    )  icon = 40;
        if ( type == OBJECT_INFO     )  icon = 44;
        if ( type == OBJECT_REPAIR   )  icon = 41;
        if ( type == OBJECT_DESTROYER)  icon = 41;
        if ( type == OBJECT_NUCLEAR  )  icon = 42;
        if ( type == OBJECT_PARA     )  icon = 46;
        if ( type == OBJECT_SAFE     )  icon = 47;
        if ( type == OBJECT_HUSTON   )  icon = 48;
        if ( type == OBJECT_BASE     )  icon = 43;
    }
    else
    {
        if ( type == OBJECT_HUMAN    )  icon =  8;
        if ( type == OBJECT_MOBILEfa )  icon = 11;
        if ( type == OBJECT_MOBILEta )  icon = 10;
        if ( type == OBJECT_MOBILEwa )  icon =  9;
        if ( type == OBJECT_MOBILEia )  icon = 22;
        if ( type == OBJECT_MOBILEfc )  icon = 17;
        if ( type == OBJECT_MOBILEtc )  icon = 16;
        if ( type == OBJECT_MOBILEwc )  icon = 15;
        if ( type == OBJECT_MOBILEic )  icon = 23;
        if ( type == OBJECT_MOBILEfi )  icon = 27;
        if ( type == OBJECT_MOBILEti )  icon = 26;
        if ( type == OBJECT_MOBILEwi )  icon = 25;
        if ( type == OBJECT_MOBILEii )  icon = 28;
        if ( type == OBJECT_MOBILEfs )  icon = 14;
        if ( type == OBJECT_MOBILEts )  icon = 13;
        if ( type == OBJECT_MOBILEws )  icon = 12;
        if ( type == OBJECT_MOBILEis )  icon = 24;
        if ( type == OBJECT_MOBILErt )  icon = 18;
        if ( type == OBJECT_MOBILErc )  icon = 19;
        if ( type == OBJECT_MOBILErr )  icon = 20;
        if ( type == OBJECT_MOBILErs )  icon = 29;
        if ( type == OBJECT_MOBILEsa )  icon = 21;
        if ( type == OBJECT_MOBILEft )  icon = 30;
        if ( type == OBJECT_MOBILEtt )  icon = 30;
        if ( type == OBJECT_MOBILEwt )  icon = 30;
        if ( type == OBJECT_MOBILEit )  icon = 30;
        if ( type == OBJECT_MOBILEdr )  icon = 48;
        if ( type == OBJECT_APOLLO2  )  icon = 49;
    }
    if ( icon == -1 ) return -1;
    return 128+icon;
}

// Updates the interface shortcuts to the units.

bool CMainShort::UpdateShortcuts()
{
    for(unsigned int i = 0; i < m_shortcuts.size(); i++)
    {
        CControl* pc = m_interface->SearchControl(static_cast<EventType>(EVENT_OBJECT_SHORTCUT+i));
        if ( pc != nullptr )
        {
            assert(m_shortcuts[i]->Implements(ObjectInterfaceType::Controllable));
            pc->SetState(STATE_CHECK, dynamic_cast<CControllableObject*>(m_shortcuts[i])->GetSelect());
            pc->SetState(STATE_RUN, m_shortcuts[i]->Implements(ObjectInterfaceType::Programmable) && dynamic_cast<CProgrammableObject*>(m_shortcuts[i])->IsProgram());
            pc->SetState(STATE_DAMAGE, dynamic_cast<CDamageableObject*>(m_shortcuts[i])->IsDamaging());
        }
    }
    return true;
}

// Selects an object through a shortcut.

void CMainShort::SelectShortcut(EventType event)
{
    if (event == EVENT_OBJECT_SHORTCUT_MODE)
    {
        m_bBuilding = !m_bBuilding;
        CreateShortcuts();
    }

    if(event >= EVENT_OBJECT_SHORTCUT && event <= EVENT_OBJECT_SHORTCUT_MAX)
    {
        unsigned int i = event-EVENT_OBJECT_SHORTCUT;
        m_main->SelectObject(m_shortcuts[i]);
    }
}


// Selects the next object.

void CMainShort::SelectNext()
{
    if ( m_main->GetMovieLock() ||
         m_main->GetEditLock()  ||
         m_engine->GetPause()   )  return;

    CObject* pPrev = m_main->DeselectAll();

    if (m_shortcuts.size() == 0)
    {
        m_main->SelectObject(m_main->SearchHuman());
        return;
    }

    // Find the current object in the list
    auto it = std::find(m_shortcuts.begin(), m_shortcuts.end(), pPrev);

    // Get the next one
    if (it != m_shortcuts.end())
    {
        ++it;
    }
    // If there is no more left, return to the first one
    if (it == m_shortcuts.end())
    {
        it = m_shortcuts.begin();
    }

    m_main->SelectObject(*it);
}


// The object detected by the mouse hovers over.

CObject* CMainShort::DetectShort(Math::Point pos)
{
    for (unsigned int i = 0; i < m_shortcuts.size(); i++)
    {
        CControl* pc = m_interface->SearchControl(static_cast<EventType>(EVENT_OBJECT_SHORTCUT+i));
        if ( pc != nullptr )
        {
            Math::Point cpos = pc->GetPos();
            Math::Point cdim = pc->GetDim();

            if ( pos.x >= cpos.x        &&
                 pos.x <= cpos.x+cdim.x &&
                 pos.y >= cpos.y        &&
                 pos.y <= cpos.y+cdim.y )
            {
                return m_shortcuts[i];
            }
        }
    }
    return nullptr;
}

// Reports the object with the mouse hovers over.

void CMainShort::SetHighlight(CObject* pObj)
{
    for (unsigned int i = 0; i < m_shortcuts.size(); i++)
    {
        CControl* pc = m_interface->SearchControl(static_cast<EventType>(EVENT_OBJECT_SHORTCUT+i));
        if ( pc == nullptr )  continue;

        if ( m_shortcuts[i] == pObj )
        {
            pc->SetState(STATE_HILIGHT);
            pc->SetState(STATE_FRAME);
        }
        else
        {
            pc->ClearState(STATE_HILIGHT);
            pc->ClearState(STATE_FRAME);
        }
    }
}

}
