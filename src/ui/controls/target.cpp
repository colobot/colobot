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


#include "ui/controls/target.h"

#include "level/robotmain.h"

#include "object/object_manager.h"
#include "object/old_object.h"


namespace Ui
{
// Object's constructor.

CTarget::CTarget() : CControl()
{
}

// Object's destructor.

CTarget::~CTarget()
{
}


// Creates a new button.

bool CTarget::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);

    return true;
}


// Management of an event.

bool CTarget::EventProcess(const Event &event)
{
    CObject*    pObj;

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;
    if ( m_state & STATE_DEAD )  return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_DOWN || event.type == EVENT_MOUSE_BUTTON_UP )
    {
        m_main->SetFriendAim(false);

        if ( CControl::Detect(event.mousePos) )
        {
            pObj = DetectFriendObject(event.mousePos);
            if ( pObj == nullptr )
            {
                m_engine->SetMouseType(Gfx::ENG_MOUSE_TARGET);

                m_event->AddEvent(Event(m_eventType));
            }
            else
            {
                m_main->SetFriendAim(true);
                m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
            }
        }
    }

    if (event.type == EVENT_MOUSE_BUTTON_DOWN &&
        event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT &&
        (m_state & STATE_VISIBLE) &&
        (m_state & STATE_ENABLE))
    {
        if ( CControl::Detect(event.mousePos) )
        {
            if ( !m_main->GetFriendAim() )
            {
                m_event->AddEvent(Event(EVENT_OBJECT_FIRE));
                return false;
            }
        }
    }

    return true;
}


// Draws button.

void CTarget::Draw()
{
    // It is completely invisible!
}


// Returns the tooltip.

bool CTarget::GetTooltip(Math::Point pos, std::string &name)const
{
    if ( (m_state & STATE_VISIBLE) == 0 )  return false;

    if ( (m_state&STATE_VISIBLE) && Detect(pos) )  // in the window?
    {
        if ( !m_main->GetFriendAim() )
        {
            //BUG?? - TOCHECK - old:    m_tooltip = name;
            name=m_tooltip;
            return true;  // does not detect objects below!
        }
    }

    return false;
}


// Detects the object aimed by the mouse.

CObject* CTarget::DetectFriendObject(Math::Point pos)
{
    Math::Vector p;
    int objRank = m_engine->DetectObject(pos, p);

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        CObject* target = obj;
        if ( obj->Implements(ObjectInterfaceType::PowerContainer) && IsObjectBeingTransported(obj) )
        {
            target = dynamic_cast<CTransportableObject*>(obj)->GetTransporter();
        }

        if ( !target->GetDetectable() )  continue;
        if ( target->GetProxyActivate() )  continue;
        if ( target->Implements(ObjectInterfaceType::Controllable) && dynamic_cast<CControllableObject*>(target)->GetSelect() )  continue;
        if ( !target->Implements(ObjectInterfaceType::Controllable) || !dynamic_cast<CControllableObject*>(target)->GetSelectable() )  continue;

        if (!target->Implements(ObjectInterfaceType::Old)) continue; // TODO: To be removed after COldObjectInterface is gone

        for (int j=0 ; j<OBJECTMAXPART ; j++ )
        {
            int rank = obj->GetObjectRank(j);
            if ( rank == -1 )  continue;
            if ( rank != objRank )  continue;
            return target;
        }
    }

    return nullptr;
}

}
