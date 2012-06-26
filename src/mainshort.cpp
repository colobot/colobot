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

// mainshort.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "global.h"
#include "event.h"
#include "iman.h"
#include "object.h"
#include "interface.h"
#include "map.h"
#include "button.h"
#include "robotmain.h"
#include "mainshort.h"




// Constructor of the application card.

CMainShort::CMainShort(CInstanceManager* iMan)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_SHORT, this);

    m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);

    FlushShortcuts();
}

// Destructor of the application card.

CMainShort::~CMainShort()
{
}



void CMainShort::SetMode(BOOL bBuilding)
{
    m_bBuilding = bBuilding;
}



// Reset all shortcuts.

void CMainShort::FlushShortcuts()
{
    int     i;

    for ( i=0 ; i<20 ; i++ )
    {
        m_shortcuts[i] = 0;
    }
}

static EventMsg table_sc_em[20] =
{
    EVENT_OBJECT_SHORTCUT00,
    EVENT_OBJECT_SHORTCUT01,
    EVENT_OBJECT_SHORTCUT02,
    EVENT_OBJECT_SHORTCUT03,
    EVENT_OBJECT_SHORTCUT04,
    EVENT_OBJECT_SHORTCUT05,
    EVENT_OBJECT_SHORTCUT06,
    EVENT_OBJECT_SHORTCUT07,
    EVENT_OBJECT_SHORTCUT08,
    EVENT_OBJECT_SHORTCUT09,
    EVENT_OBJECT_SHORTCUT10,
    EVENT_OBJECT_SHORTCUT11,
    EVENT_OBJECT_SHORTCUT12,
    EVENT_OBJECT_SHORTCUT13,
    EVENT_OBJECT_SHORTCUT14,
    EVENT_OBJECT_SHORTCUT15,
    EVENT_OBJECT_SHORTCUT16,
    EVENT_OBJECT_SHORTCUT17,
    EVENT_OBJECT_SHORTCUT18,
    EVENT_OBJECT_SHORTCUT19,
};

// Interface creates shortcuts to the units.

BOOL CMainShort::CreateShortcuts()
{
    CObject*    pObj;
    CControl*   pc;
    ObjectType  type;
    FPOINT      pos, dim;
    int         i, rank, icon;
    char        name[100];

    if ( m_main->RetFixScene() )  return FALSE;

    m_interface->DeleteControl(EVENT_OBJECT_MOVIELOCK);
    m_interface->DeleteControl(EVENT_OBJECT_EDITLOCK);
    for ( i=0 ; i<20 ; i++ )
    {
        if ( i != 0 && m_shortcuts[i] == 0 )  continue;

        m_interface->DeleteControl(table_sc_em[i]);
        m_shortcuts[i] = 0;
    }

    dim.x = 28.0f/640.0f;
    dim.y = 28.0f/480.0f;
    pos.x =  4.0f/640.0f;
    pos.y = (480.0f-32.0f)/480.0f;

    if ( m_main->RetMovieLock() &&
        !m_main->RetEditLock()  )  // hangs during film?
    {
        m_interface->CreateShortcut(pos, dim, 7, EVENT_OBJECT_MOVIELOCK);
        return TRUE;
    }
    if ( !m_main->RetFreePhoto() &&
         (m_main->RetEditLock() ||
          m_engine->RetPause()) )  // hangs during edition?
    {
        m_interface->CreateShortcut(pos, dim, 6, EVENT_OBJECT_EDITLOCK);
        return TRUE;
    }

    rank = 0;

    m_interface->CreateShortcut(pos, dim, 2, table_sc_em[rank]);
    pos.x += dim.x*1.2f;
    m_shortcuts[rank] = 0;
    rank ++;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;
        if ( !pObj->RetSelectable() )  continue;
        if ( pObj->RetProxyActivate() )  continue;

        type = pObj->RetType();
        icon = -1;
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
        if ( icon == -1 )  continue;

        m_interface->CreateShortcut(pos, dim, icon, table_sc_em[rank]);
        pos.x += dim.x;
        m_shortcuts[rank] = pObj;

        pc = m_interface->SearchControl(table_sc_em[rank]);
        if ( pc != 0 )
        {
            pObj->GetTooltipName(name);
            pc->SetTooltip(name);
        }
        rank ++;

        if ( rank >= 20 )  break;
    }

    UpdateShortcuts();
    return TRUE;
}

// Updates the interface shortcuts to the units.

BOOL CMainShort::UpdateShortcuts()
{
    CControl*   pc;
    int         i;

    for ( i=0 ; i<20 ; i++ )
    {
        if ( m_shortcuts[i] == 0 )  continue;

        pc = m_interface->SearchControl(table_sc_em[i]);
        if ( pc != 0 )
        {
            pc->SetState(STATE_CHECK, m_shortcuts[i]->RetSelect());
            pc->SetState(STATE_RUN, m_shortcuts[i]->IsProgram());
        }
    }
    return TRUE;
}

// Selects an object through a shortcut.

void CMainShort::SelectShortcut(EventMsg event)
{
    int     i;

    for ( i=0 ; i<20 ; i++ )
    {
        if ( event == table_sc_em[i] )
        {
            if ( i != 0 && m_shortcuts[i] == 0 )  continue;

            if ( i == 0 )  // buildings <-> vehicles?
            {
                m_bBuilding = !m_bBuilding;
                CreateShortcuts();
            }
            else
            {
                m_main->SelectObject(m_shortcuts[i]);
            }
            return;
        }
    }
}


// Selects the next object.

void CMainShort::SelectNext()
{
    CObject*    pPrev;
    int         i;

    if ( m_main->RetMovieLock() ||
         m_main->RetEditLock()  ||
         m_engine->RetPause()   )  return;

    pPrev = m_main->DeselectAll();

    for ( i=1 ; i<20 ; i++ )
    {
        if ( m_shortcuts[i] == pPrev )
        {
            if ( m_shortcuts[++i] == 0 )  i = 1;
            break;
        }
    }

    if ( i == 20 || m_shortcuts[i] == 0 )
    {
        m_main->SelectHuman();
    }
    else
    {
        m_main->SelectObject(m_shortcuts[i]);
    }
}


// The object detected by the mouse hovers over.

CObject* CMainShort::DetectShort(FPOINT pos)
{
    CControl*   pc;
    FPOINT      cpos, cdim;
    int         i;

    for ( i=0 ; i<20 ; i++ )
    {
        if ( m_shortcuts[i] == 0 )  continue;

        pc = m_interface->SearchControl(table_sc_em[i]);
        if ( pc != 0 )
        {
            cpos = pc->RetPos();
            cdim = pc->RetDim();

            if ( pos.x >= cpos.x        &&
                 pos.x <= cpos.x+cdim.x &&
                 pos.y >= cpos.y        &&
                 pos.y <= cpos.y+cdim.y )
            {
                return m_shortcuts[i];
            }
        }
    }
    return 0;
}

// Reports the object with the mouse hovers over.

void CMainShort::SetHilite(CObject* pObj)
{
    CControl*   pc;
    int         i;

    for ( i=0 ; i<20 ; i++ )
    {
        if ( m_shortcuts[i] == 0 )  continue;

        pc = m_interface->SearchControl(table_sc_em[i]);
        if ( pc == 0 )  continue;

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

