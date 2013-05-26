// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "ui/target.h"

#include "common/iman.h"


namespace Ui {
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
#if 0
    if ( (m_state & STATE_VISIBLE) == 0 )  return true;
    if ( m_state & STATE_DEAD )  return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            m_engine->SetMouseType(Gfx::ENG_MOUSE_TARGET);
            Event newEvent = event;
            newEvent.type = m_eventType;
            m_event->AddEvent(newEvent);
            return false;
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            Event newEvent = event;
            newEvent.type = EVENT_OBJECT_FIRE;
            m_event->AddEvent(newEvent);
            return false;
        }
    }

    return true;
#else
    CObject*    pObj;

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;
    if ( m_state & STATE_DEAD )  return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        m_main->SetFriendAim(false);

        if ( CControl::Detect(event.mousePos) )
        {
            pObj = DetectFriendObject(event.mousePos);
            if ( pObj == 0 )
            {
                m_engine->SetMouseType(Gfx::ENG_MOUSE_TARGET);

                Event newEvent = event;
                newEvent.type = m_eventType;
                m_event->AddEvent(newEvent);
                return false;
            }
            else
            {
                m_main->SetFriendAim(true);
                m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
            }
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
          event.mouseButton.button == MOUSE_BUTTON_LEFT   &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            if ( !m_main->GetFriendAim() )
            {
                Event newEvent = event;
                newEvent.type = EVENT_OBJECT_FIRE;
                m_event->AddEvent(newEvent);
                return false;
            }
        }
    }

    return true;
#endif
}


// Draws button.

void CTarget::Draw()
{
    // It is completely invisible!
}


// Returns the tooltip.

bool CTarget::GetTooltip(Math::Point pos, std::string &name)
{
#if 0
    if ( (m_state&STATE_VISIBLE) && Detect(pos) )  // in the window?
    {
        strcpy(name, m_tooltip);
        return true;  // does not detect objects below!
    }

    return false;
#else
//? CObject*    pObj;

    if ( (m_state & STATE_VISIBLE) == 0 )  return false;

    if ( (m_state&STATE_VISIBLE) && Detect(pos) )  // in the window?
    {
//?     pObj = DetectFriendObject(pos);
//?     if ( pObj == 0 )
        if ( !m_main->GetFriendAim() )
        {
             m_tooltip = name;
            return true;  // does not detect objects below!
        }
    }

    return false;
#endif
}


// Detects the object aimed by the mouse.

CObject* CTarget::DetectFriendObject(Math::Point pos)
{
    ObjectType  type;
    CObject     *pObj, *pTarget;
    int         objRank, i, j, rank;

    objRank = m_engine->DetectObject(pos);

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( !pObj->GetActif() )  continue;
        if ( pObj->GetProxyActivate() )  continue;
        if ( pObj->GetSelect() )  continue;

        pTarget = 0;
        type = pObj->GetType();
        if ( type == OBJECT_DERRICK      ||
             type == OBJECT_FACTORY      ||
             type == OBJECT_REPAIR       ||
             type == OBJECT_DESTROYER    ||
             type == OBJECT_STATION      ||
             type == OBJECT_CONVERT      ||
             type == OBJECT_TOWER        ||
             type == OBJECT_RESEARCH     ||
             type == OBJECT_RADAR        ||
             type == OBJECT_INFO         ||
             type == OBJECT_ENERGY       ||
             type == OBJECT_LABO         ||
             type == OBJECT_NUCLEAR      ||
             type == OBJECT_PARA         ||
             type == OBJECT_SAFE         ||
             type == OBJECT_HUSTON       ||
             type == OBJECT_HUMAN        ||
             type == OBJECT_TECH         ||
             type == OBJECT_TOTO         ||
             type == OBJECT_MOBILEfa     ||
             type == OBJECT_MOBILEta     ||
             type == OBJECT_MOBILEwa     ||
             type == OBJECT_MOBILEia     ||
             type == OBJECT_MOBILEfc     ||
             type == OBJECT_MOBILEtc     ||
             type == OBJECT_MOBILEwc     ||
             type == OBJECT_MOBILEic     ||
             type == OBJECT_MOBILEfi     ||
             type == OBJECT_MOBILEti     ||
             type == OBJECT_MOBILEwi     ||
             type == OBJECT_MOBILEii     ||
             type == OBJECT_MOBILEfs     ||
             type == OBJECT_MOBILEts     ||
             type == OBJECT_MOBILEws     ||
             type == OBJECT_MOBILEis     ||
             type == OBJECT_MOBILErt     ||
             type == OBJECT_MOBILErc     ||
             type == OBJECT_MOBILErr     ||
             type == OBJECT_MOBILErs     ||
             type == OBJECT_MOBILEsa     ||
             type == OBJECT_MOBILEft     ||
             type == OBJECT_MOBILEtt     ||
             type == OBJECT_MOBILEwt     ||
             type == OBJECT_MOBILEit     ||
             type == OBJECT_MOBILEdr     )
        {
            pTarget = pObj;
        }
        else if ( (type == OBJECT_POWER  ||
                  type == OBJECT_ATOMIC ) &&
             pObj->GetTruck() != 0 )  // battery used?
        {
            pTarget = pObj->GetTruck();
            if ( pTarget->GetType() == OBJECT_MOBILEtg )
            {
                pTarget = 0;
            }
        }

        for ( j=0 ; j<OBJECTMAXPART ; j++ )
        {
            rank = pObj->GetObjectRank(j);
            if ( rank == -1 )  continue;
            if ( rank != objRank )  continue;
            return pTarget;
        }
    }
    return 0;
}

}

