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

// target.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "robotmain.h"
#include "object.h"
#include "restext.h"
#include "target.h"




// Object's constructor.

CTarget::CTarget(CInstanceManager* iMan) : CControl(iMan)
{
}

// Object's destructor.

CTarget::~CTarget()
{
}


// Creates a new button.

BOOL CTarget::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);

    return TRUE;
}


// Management of an event.

BOOL CTarget::EventProcess(const Event &event)
{
#if 0
    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_MOUSEMOVE )
    {
        if ( CControl::Detect(event.pos) )
        {
            m_engine->SetMouseType(D3DMOUSETARGET);
            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);
            return FALSE;
        }
    }

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            Event newEvent = event;
            newEvent.event = EVENT_OBJECT_FIRE;
            m_event->AddEvent(newEvent);
            return FALSE;
        }
    }

    return TRUE;
#else
    CObject*    pObj;

    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_MOUSEMOVE )
    {
        m_main->SetFriendAim(FALSE);

        if ( CControl::Detect(event.pos) )
        {
            pObj = DetectFriendObject(event.pos);
            if ( pObj == 0 )
            {
                m_engine->SetMouseType(D3DMOUSETARGET);

                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
                return FALSE;
            }
            else
            {
                m_main->SetFriendAim(TRUE);
                m_engine->SetMouseType(D3DMOUSENORM);
            }
        }
    }

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            if ( !m_main->RetFriendAim() )
            {
                Event newEvent = event;
                newEvent.event = EVENT_OBJECT_FIRE;
                m_event->AddEvent(newEvent);
                return FALSE;
            }
        }
    }

    return TRUE;
#endif
}


// Draws button.

void CTarget::Draw()
{
    // It is completely invisible!
}


// Returns the tooltip.

BOOL CTarget::GetTooltip(FPOINT pos, char* name)
{
#if 0
    if ( (m_state&STATE_VISIBLE) && Detect(pos) )  // in the window?
    {
        strcpy(name, m_tooltip);
        return TRUE;  // does not detect objects below!
    }

    return FALSE;
#else
//? CObject*    pObj;

    if ( (m_state & STATE_VISIBLE) == 0 )  return FALSE;

    if ( (m_state&STATE_VISIBLE) && Detect(pos) )  // in the window?
    {
//?     pObj = DetectFriendObject(pos);
//?     if ( pObj == 0 )
        if ( !m_main->RetFriendAim() )
        {
            strcpy(name, m_tooltip);
            return TRUE;  // does not detect objects below!
        }
    }

    return FALSE;
#endif
}


// Detects the object aimed by the mouse.

CObject* CTarget::DetectFriendObject(FPOINT pos)
{
    ObjectType  type;
    CObject     *pObj, *pTarget;
    int         objRank, i, j, rank;

    objRank = m_engine->DetectObject(pos);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;
        if ( pObj->RetProxyActivate() )  continue;
        if ( pObj->RetSelect() )  continue;

        pTarget = 0;
        type = pObj->RetType();
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
             pObj->RetTruck() != 0 )  // battery used?
        {
            pTarget = pObj->RetTruck();
            if ( pTarget->RetType() == OBJECT_MOBILEtg )
            {
                pTarget = 0;
            }
        }

        for ( j=0 ; j<OBJECTMAXPART ; j++ )
        {
            rank = pObj->RetObjectRank(j);
            if ( rank == -1 )  continue;
            if ( rank != objRank )  continue;
            return pTarget;
        }
    }
    return 0;
}

