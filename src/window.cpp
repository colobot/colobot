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

// window.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "restext.h"
#include "iman.h"
#include "button.h"
#include "color.h"
#include "check.h"
#include "key.h"
#include "group.h"
#include "image.h"
#include "label.h"
#include "edit.h"
#include "editvalue.h"
#include "scroll.h"
#include "slider.h"
#include "list.h"
#include "shortcut.h"
#include "map.h"
#include "gauge.h"
#include "compass.h"
#include "target.h"
#include "text.h"
#include "window.h"




// Object's constructor.

CWindow::CWindow(CInstanceManager* iMan) : CControl(iMan)
{
    int     i;

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        m_table[i] = 0;
    }

    m_bTrashEvent = TRUE;
    m_bMaximized  = FALSE;
    m_bMinimized  = FALSE;
    m_bFixed      = FALSE;

    m_minDim = FPOINT(0.0f, 0.0f);
    m_maxDim = FPOINT(1.0f, 1.0f);

    m_buttonReduce = 0;
    m_buttonFull   = 0;
    m_buttonClose  = 0;

    m_bMovable  = FALSE;
    m_bRedim    = FALSE;
    m_bClosable = FALSE;
    m_bCapture  = FALSE;

    m_fontStretch = NORMSTRETCH*1.2f;
}

// Object's destructor.

CWindow::~CWindow()
{
    Flush();
}


// Purge all the controls.

void CWindow::Flush()
{
    int     i;

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] != 0 )
        {
            delete m_table[i];
            m_table[i] = 0;
        }
    }

    if ( m_buttonReduce != 0 )
    {
        delete m_buttonReduce;
        m_buttonReduce = 0;
    }

    if ( m_buttonFull != 0 )
    {
        delete m_buttonFull;
        m_buttonFull = 0;
    }

    if ( m_buttonClose != 0 )
    {
        delete m_buttonClose;
        m_buttonClose = 0;
    }
}


// Creates a new window.

BOOL CWindow::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);
    return TRUE;
}


// Creates a new button.

CButton* CWindow::CreateButton(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CButton*    pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CButton(m_iMan);
            pc = (CButton*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new button.

CColor* CWindow::CreateColor(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CColor*     pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CColor(m_iMan);
            pc = (CColor*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new button.

CCheck* CWindow::CreateCheck(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CCheck*     pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CCheck(m_iMan);
            pc = (CCheck*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new button.

CKey* CWindow::CreateKey(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CKey*       pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CKey(m_iMan);
            pc = (CKey*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new button.

CGroup* CWindow::CreateGroup(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CGroup*     pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CGroup(m_iMan);
            pc = (CGroup*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new button.

CImage* CWindow::CreateImage(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CImage*     pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CImage(m_iMan);
            pc = (CImage*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new label.

CLabel* CWindow::CreateLabel(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg,
                          char *name)
{
    CLabel*     pc;
    char*       p;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CLabel(m_iMan);
            pc = (CLabel*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);

            p = strchr(name, '\\');
            if ( p == 0 )
            {
                pc->SetName(name);
            }
            else
            {
                char    text[100];
                strncpy(text, name, 100);
                text[100-1] = 0;
                if ( p-name < 100 )
                {
                    text[p-name] = 0;  // deletes text after "\\" (tooltip)
                }
                pc->SetName(text);
            }
            return pc;
        }
    }
    return 0;
}

// Creates a new editable pave.

CEdit* CWindow::CreateEdit(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CEdit*      pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CEdit(m_iMan);
            pc = (CEdit*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new editable pave.

CEditValue* CWindow::CreateEditValue(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CEditValue* pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CEditValue(m_iMan);
            pc = (CEditValue*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new elevator.

CScroll* CWindow::CreateScroll(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CScroll*    pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CScroll(m_iMan);
            pc = (CScroll*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new cursor.

CSlider* CWindow::CreateSlider(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CSlider*    pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CSlider(m_iMan);
            pc = (CSlider*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new list.

CList* CWindow::CreateList(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg,
                         float expand)
{
    CList*      pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CList(m_iMan);
            pc = (CList*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg, expand);
            return pc;
        }
    }
    return 0;
}

// Creates a new shortcut.

CShortcut* CWindow::CreateShortcut(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CShortcut*  ps;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CShortcut(m_iMan);
            ps = (CShortcut*)m_table[i];
            ps->Create(pos, dim, icon, eventMsg);
            return ps;
        }
    }
    return 0;
}

// Creates a new card.

CMap* CWindow::CreateMap(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CMap*       pm;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CMap(m_iMan);
            pm = (CMap*)m_table[i];
            pm->Create(pos, dim, icon, eventMsg);
            return pm;
        }
    }
    return 0;
}

// Creates a new gauge.

CGauge* CWindow::CreateGauge(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CGauge*     pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CGauge(m_iMan);
            pc = (CGauge*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new compass.

CCompass* CWindow::CreateCompass(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CCompass*   pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CCompass(m_iMan);
            pc = (CCompass*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Creates a new target.

CTarget* CWindow::CreateTarget(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CTarget*    pc;
    int         i;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] == 0 )
        {
            m_table[i] = new CTarget(m_iMan);
            pc = (CTarget*)m_table[i];
            pc->Create(pos, dim, icon, eventMsg);
            return pc;
        }
    }
    return 0;
}

// Removes a control.

BOOL CWindow::DeleteControl(EventMsg eventMsg)
{
    int     i;

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] != 0 )
        {
            if ( eventMsg == m_table[i]->RetEventMsg() )
            {
                delete m_table[i];
                m_table[i] = 0;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Gives a control.

CControl* CWindow::SearchControl(EventMsg eventMsg)
{
    int     i;

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] != 0 )
        {
            if ( eventMsg == m_table[i]->RetEventMsg() )
            {
                return m_table[i];
            }
        }
    }
    return 0;
}


// Makes the tooltip binds to the window.

BOOL CWindow::GetTooltip(FPOINT pos, char* name)
{
    int     i;

    for ( i=MAXWINDOW-1 ; i>=0 ; i-- )
    {
        if ( m_table[i] != 0 )
        {
            if ( m_table[i]->GetTooltip(pos, name) )
            {
                return TRUE;
            }
        }
    }

    if ( m_buttonClose != 0 &&
         m_buttonClose->GetTooltip(pos, name) )
    {
        return TRUE;
    }
    if ( m_buttonFull != 0 &&
         m_buttonFull->GetTooltip(pos, name) )
    {
        return TRUE;
    }
    if ( m_buttonReduce != 0 &&
         m_buttonReduce->GetTooltip(pos, name) )
    {
        return TRUE;
    }

    if ( Detect(pos) )  // in the window?
    {
        strcpy(name, m_tooltip);
        return TRUE;
    }

    return FALSE;
}


// Specifies the name for the title bar.

void CWindow::SetName(char* name)
{
    CButton*    pc;
    BOOL        bAdjust;

    CControl::SetName(name);

    if ( m_buttonReduce != 0 )
    {
        delete m_buttonReduce;
        m_buttonReduce = 0;
    }

    if ( m_buttonFull != 0 )
    {
        delete m_buttonFull;
        m_buttonFull = 0;
    }

    if ( m_buttonClose != 0 )
    {
        delete m_buttonClose;
        m_buttonClose = 0;
    }

    bAdjust = FALSE;

    if ( m_name[0] != 0 && m_bRedim )  // title bar exists?
    {
        m_buttonReduce = new CButton(m_iMan);
        pc = (CButton*)m_buttonReduce;
        pc->Create(m_pos, m_dim, 0, EVENT_NULL);

        m_buttonFull = new CButton(m_iMan);
        pc = (CButton*)m_buttonFull;
        pc->Create(m_pos, m_dim, 0, EVENT_NULL);

        bAdjust = TRUE;
    }

    if ( m_name[0] != 0 && m_bClosable )  // title bar exists?
    {
        m_buttonClose = new CButton(m_iMan);
        pc = (CButton*)m_buttonClose;
        pc->Create(m_pos, m_dim, 0, EVENT_NULL);

        bAdjust = TRUE;
    }

    if ( bAdjust )
    {
        AdjustButtons();
    }

    MoveAdjust();
}


void CWindow::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CWindow::SetDim(FPOINT dim)
{
    if ( dim.x < m_minDim.x )  dim.x = m_minDim.x;
    if ( dim.x > m_maxDim.x )  dim.x = m_maxDim.x;
    if ( dim.y < m_minDim.y )  dim.y = m_minDim.y;
    if ( dim.y > m_maxDim.y )  dim.y = m_maxDim.y;

    CControl::SetDim(dim);
    MoveAdjust();
}

void CWindow::MoveAdjust()
{
    FPOINT      pos, dim;
    float       h, offset;

    h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);
    dim.y = h*1.2f;
    dim.x = dim.y*0.75f;

    if ( m_buttonClose != 0 )
    {
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonClose->SetPos(pos);
        m_buttonClose->SetDim(dim);
        offset = dim.x*1.0f;
    }
    else
    {
        offset = 0.0f;
    }

    if ( m_buttonFull != 0 )
    {
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x-offset;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonFull->SetPos(pos);
        m_buttonFull->SetDim(dim);
    }

    if ( m_buttonReduce != 0 )
    {
        pos.x = m_pos.x+m_dim.x-0.01f-dim.x*2.0f-offset;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        m_buttonReduce->SetPos(pos);
        m_buttonReduce->SetDim(dim);
    }
}


void CWindow::SetMinDim(FPOINT dim)
{
    m_minDim = dim;
}

void CWindow::SetMaxDim(FPOINT dim)
{
    m_maxDim = dim;
}

FPOINT CWindow::RetMinDim()
{
    return m_minDim;
}

FPOINT CWindow::RetMaxDim()
{
    return m_maxDim;
}


// Indicates whether the window is moved.

void CWindow::SetMovable(BOOL bMode)
{
    m_bMovable = bMode;
}

BOOL CWindow::RetMovable()
{
    return m_bMovable;
}


// Management of the presence of minimize/maximize buttons.

void CWindow::SetRedim(BOOL bMode)
{
    m_bRedim = bMode;
}

BOOL CWindow::RetRedim()
{
    return m_bRedim;
}


// Management of the presence of the close button.

void CWindow::SetClosable(BOOL bMode)
{
    m_bClosable = bMode;
}

BOOL CWindow::RetClosable()
{
    return m_bClosable;
}


void CWindow::SetMaximized(BOOL bMaxi)
{
    m_bMaximized = bMaxi;
    AdjustButtons();
}

BOOL CWindow::RetMaximized()
{
    return m_bMaximized;
}

void CWindow::SetMinimized(BOOL bMini)
{
    m_bMinimized = bMini;
    AdjustButtons();
}

BOOL CWindow::RetMinimized()
{
    return m_bMinimized;
}

void CWindow::SetFixed(BOOL bFix)
{
    m_bFixed = bFix;
}

BOOL CWindow::RetFixed()
{
    return m_bFixed;
}


// Adjusts the buttons in the title bar.

void CWindow::AdjustButtons()
{
    char    res[100];

    if ( m_buttonFull != 0 )
    {
        if ( m_bMaximized )
        {
            m_buttonFull->SetIcon(54);
            GetResource(RES_TEXT, RT_WINDOW_STANDARD, res);
            m_buttonFull->SetTooltip(res);
        }
        else
        {
            m_buttonFull->SetIcon(52);
            GetResource(RES_TEXT, RT_WINDOW_MAXIMIZED, res);
            m_buttonFull->SetTooltip(res);
        }
    }

    if ( m_buttonReduce != 0 )
    {
        if ( m_bMinimized )
        {
            m_buttonReduce->SetIcon(54);
            GetResource(RES_TEXT, RT_WINDOW_STANDARD, res);
            m_buttonReduce->SetTooltip(res);
        }
        else
        {
            m_buttonReduce->SetIcon(51);
            GetResource(RES_TEXT, RT_WINDOW_MINIMIZED, res);
            m_buttonReduce->SetTooltip(res);
        }
    }

    if ( m_buttonClose != 0 )
    {
        m_buttonClose->SetIcon(11);  // x
        GetResource(RES_TEXT, RT_WINDOW_CLOSE, res);
        m_buttonClose->SetTooltip(res);
    }
}


void CWindow::SetTrashEvent(BOOL bTrash)
{
    m_bTrashEvent = bTrash;
}

BOOL CWindow::RetTrashEvent()
{
    return m_bTrashEvent;
}


// Returns the message from the button "reduce".

EventMsg CWindow::RetEventMsgReduce()
{
    if ( m_buttonReduce == 0 )  return EVENT_NULL;
    return m_buttonReduce->RetEventMsg();
}

// Returns the message from the button "full".

EventMsg CWindow::RetEventMsgFull()
{
    if ( m_buttonFull == 0 )  return EVENT_NULL;
    return m_buttonFull->RetEventMsg();
}

// Returns the message from the button "close".

EventMsg CWindow::RetEventMsgClose()
{
    if ( m_buttonClose == 0 )  return EVENT_NULL;
    return m_buttonClose->RetEventMsg();
}


// Detects whether the mouse is in an edge of the window, to resize it.
// Bit returns: 0 = left, 1 = down, 2 = right, 3 = up, 1 = all.

int CWindow::BorderDetect(FPOINT pos)
{
    FPOINT  dim;
    float   h;
    int     flags;

    if ( m_bMaximized || m_bMinimized || m_bFixed )  return 0;

    flags = 0;
    if ( pos.x < m_pos.x+0.030f )
    {
        flags |= (1<<0);
    }
    if ( pos.y < m_pos.y+0.020f )
    {
        flags |= (1<<1);
    }
    if ( pos.x > m_pos.x+m_dim.x-0.030f )
    {
        flags |= (1<<2);
    }
    if ( pos.y > m_pos.y+m_dim.y-0.020f )
    {
        flags |= (1<<3);
    }

    if ( pos.x > m_pos.x+        0.015f &&
         pos.x < m_pos.x+m_dim.x-0.015f &&
         pos.y > m_pos.y+        0.010f &&
         pos.y < m_pos.y+m_dim.y-0.010f )
    {
        flags = 0;
    }

    if ( flags == 0 )
    {
        h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);
        dim.y = h*1.2f;
        dim.x = dim.y*0.75f;
        if ( pos.x <  m_pos.x+m_dim.x-0.01f-dim.x*3.0f &&
             pos.y >= m_pos.y+m_dim.y-0.01f-h*1.2f     )
        {
            flags = -1;
        }
    }

    return flags;
}

// Management of an event.

BOOL CWindow::EventProcess(const Event &event)
{
    FPOINT      pos;
    int         i, flags;

    if ( event.event == EVENT_MOUSEMOVE )
    {
        if ( m_bCapture )
        {
            m_engine->SetMouseType(m_pressMouse);
        }
        else
        {
            m_pressMouse = D3DMOUSENORM;

            if ( m_name[0] != 0 && m_bMovable &&  // title bar?
                 Detect(event.pos) )
            {
                flags = BorderDetect(event.pos);
                if ( flags == -1 )
                {
                    m_pressMouse = D3DMOUSEMOVE;  // +
                }
                else if ( ((flags & (1<<0)) && (flags & (1<<3))) ||
                          ((flags & (1<<1)) && (flags & (1<<2))) )
                {
                    m_pressMouse = D3DMOUSEMOVEI;  // \ //
                }
                else if ( ((flags & (1<<0)) && (flags & (1<<1))) ||
                          ((flags & (1<<2)) && (flags & (1<<3))) )
                {
                    m_pressMouse = D3DMOUSEMOVED;  // /
                }
                else if ( (flags & (1<<0)) || (flags & (1<<2)) )
                {
                    m_pressMouse = D3DMOUSEMOVEH;  // -
                }
                else if ( (flags & (1<<1)) || (flags & (1<<3)) )
                {
                    m_pressMouse = D3DMOUSEMOVEV;  // |
                }
            }

            if ( m_pressMouse != D3DMOUSENORM )
            {
                m_engine->SetMouseType(m_pressMouse);
            }
        }
    }

    if ( !m_bCapture )
    {
        for ( i=MAXWINDOW-1 ; i>=0 ; i-- )
        {
            if ( m_table[i] != 0 )
            {
                if ( !m_table[i]->EventProcess(event) )
                {
                    return FALSE;
                }
            }
        }

        if ( m_buttonReduce != 0 )
        {
            m_buttonReduce->EventProcess(event);
        }
        if ( m_buttonFull != 0 )
        {
            m_buttonFull->EventProcess(event);
        }
        if ( m_buttonClose != 0 )
        {
            m_buttonClose->EventProcess(event);
        }
    }

    if ( m_bTrashEvent && event.event == EVENT_LBUTTONDOWN )
    {
        if ( Detect(event.pos) )
        {
            if ( m_name[0] != 0 && m_bMovable )  // title bar?
            {
                m_pressFlags = BorderDetect(event.pos);
                if ( m_pressFlags != 0 )
                {
                    m_bCapture = TRUE;
                    m_pressPos = event.pos;
                }
            }
            return FALSE;
        }
    }

    if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
    {
        pos = event.pos;
        if ( m_pressFlags == -1 )  // all moves?
        {
            m_pos.x += pos.x-m_pressPos.x;
            m_pos.y += pos.y-m_pressPos.y;
        }
        else
        {
            if ( m_pressFlags & (1<<0) )  // left edge?
            {
                if ( pos.x > m_pressPos.x+m_dim.x-m_minDim.x )
                {
                    pos.x = m_pressPos.x+m_dim.x-m_minDim.x;
                }
                m_pos.x += pos.x-m_pressPos.x;
                m_dim.x -= pos.x-m_pressPos.x;
            }
            if ( m_pressFlags & (1<<1) )  // bottom edge?
            {
                if ( pos.y > m_pressPos.y+m_dim.y-m_minDim.y )
                {
                    pos.y = m_pressPos.y+m_dim.y-m_minDim.y;
                }
                m_pos.y += pos.y-m_pressPos.y;
                m_dim.y -= pos.y-m_pressPos.y;
            }
            if ( m_pressFlags & (1<<2) )  // right edge?
            {
                if ( pos.x < m_pressPos.x-m_dim.x+m_minDim.x )
                {
                    pos.x = m_pressPos.x-m_dim.x+m_minDim.x;
                }
                m_dim.x += pos.x-m_pressPos.x;
            }
            if ( m_pressFlags & (1<<3) )  // top edge?
            {
                if ( pos.y < m_pressPos.y-m_dim.y+m_minDim.y )
                {
                    pos.y = m_pressPos.y-m_dim.y+m_minDim.y;
                }
                m_dim.y += pos.y-m_pressPos.y;
            }
        }
        m_pressPos = pos;
        AdjustButtons();

        Event newEvent = event;
        newEvent.event = m_eventMsg;
        m_event->AddEvent(newEvent);
    }

    if ( event.event == EVENT_LBUTTONUP && m_bCapture )
    {
        m_bCapture = FALSE;
    }

    return TRUE;
}


// Draws the window.

void CWindow::Draw()
{
    FPOINT      pos, dim;
    float       width, h, sw;
    int         i;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    DrawVertex(m_pos, m_dim, m_icon);  // draws the background

    if ( m_name[0] != 0 )  // title bar?
    {
        h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);

        // Draws the shadow under the title bar.
        {
            FPOINT  sPos, sDim;

            pos.x = m_pos.x+0.01f;
            dim.x = m_dim.x-0.02f;
            pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
            dim.y = h*1.2f;
            DrawShadow(pos, dim);
        }

        width = m_dim.x;
        if ( m_bRedim    )  width -= h*1.2f*0.75f*2.0f;
        if ( m_bClosable )  width -= h*1.2f*0.75f;

        pos.x = m_pos.x+0.01f;
        dim.x = width-0.02f;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.2f;
        dim.y = h*1.2f;
        DrawVertex(pos, dim, (m_state&STATE_ENABLE)?2:9);

        sw = m_engine->RetText()->RetStringWidth(m_name, strlen(m_name), m_fontSize, m_fontStretch, m_fontType);

        if ( m_state&STATE_ENABLE )
        {
            pos.x = m_pos.x+0.015f;
            dim.x = (width-sw-0.06f)/2.0f;
            pos.y = m_pos.y+m_dim.y-0.01f-h*1.0f;
            dim.y = h*0.8f;
            DrawHach(pos, dim);  // left hatch
            pos.x = m_pos.x+width-dim.x-0.015f;
            DrawHach(pos, dim);  // right hatch
        }

        pos.x = m_pos.x+width/2.0f;
        pos.y = m_pos.y+m_dim.y-0.01f-h*1.10f;
        m_engine->RetText()->DrawText(m_name, pos, width, 0, m_fontSize, m_fontStretch, m_fontType, 0);

        if ( m_buttonReduce != 0 )
        {
            m_buttonReduce->Draw();
        }

        if ( m_buttonFull != 0 )
        {
            m_buttonFull->Draw();
        }

        if ( m_buttonClose != 0 )
        {
            m_buttonClose->Draw();
        }
    }

    for ( i=0 ; i<MAXWINDOW ; i++ )
    {
        if ( m_table[i] != 0 )
        {
            m_table[i]->Draw();
        }
    }
}

// Draws a rectangle.

void CWindow::DrawVertex(FPOINT pos, FPOINT dim, int icon)
{
    FPOINT      p1, p2, uv1, uv2, corner;
    float       dp;
    int         i;

    dp = 0.5f/256.0f;

    if ( icon == 0 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTw);
        uv1.x =  64.0f/256.0f;  // dark blue transparent
        uv1.y =  64.0f/256.0f;
        uv2.x = 128.0f/256.0f;
        uv2.y = 128.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 1 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 128.0f/256.0f;  // yellow tooltip
        uv1.y =   0.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  16.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 2 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 128.0f/256.0f;  // yellow
        uv1.y =  16.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 3 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTb);
        uv1.x =   0.0f/256.0f;  // transparent blue bar with yellow upper
        uv1.y =  64.0f/256.0f;
        uv2.x =  64.0f/256.0f;
        uv2.y = 128.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 4 )  // SatCom ?
    {
        pos.x -=  50.0f/640.0f;
        pos.y -=  30.0f/480.0f;
        dim.x += 100.0f/640.0f;
        dim.y +=  60.0f/480.0f;

        m_engine->SetTexture("human.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 140.0f/256.0f;
        uv1.y =  32.0f/256.0f;
        uv2.x = 182.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);  // clothing

        pos.x += 20.0f/640.0f;
        pos.y -= 10.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y +=  0.0f/480.0f;

        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTw);
        uv1.x = 192.0f/256.0f;
        uv1.y =  32.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 30.0f/640.0f;
        corner.y = 30.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);  // shadow

        pos.x +=  0.0f/640.0f;
        pos.y += 20.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =  64.0f/256.0f;
        uv1.y =   0.0f/256.0f;
        uv2.x =  96.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);  // outside blue

        pos.x += 20.0f/640.0f;
        pos.y += 10.0f/480.0f;
        dim.x -= 40.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        uv1.x =  96.0f/256.0f;
        uv1.y =   0.0f/256.0f;
        uv2.x = 128.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);  // inside blue

        pos.x += 10.0f/640.0f;
        pos.y += 10.0f/480.0f;
        dim.x -= 20.0f/640.0f;
        dim.y -= 20.0f/480.0f;

        m_engine->SetTexture("button3.tga");
        uv1.x =   0.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y = 256.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);  // dark blue background

        m_engine->SetTexture("button2.tga");
        uv1.x = 224.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x = 249.0f/256.0f;
        uv2.y = 235.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        pos.x = 20.0f/640.0f;
        pos.y = 70.0f/480.0f;
        dim.x = 25.0f/640.0f;
        dim.y = 11.0f/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = bottom/left
            pos.y += 15.0f/480.0f;
        }
        pos.y = (480.0f-70.0f-11.0f)/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = top/left
            pos.y -= 15.0f/480.0f;
        }
        pos.x = (640.0f-25.0f-20.0f)/640.0f;
        pos.y = 70.0f/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = bottom/right
            pos.y += 15.0f/480.0f;
        }
        pos.y = (480.0f-70.0f-11.0f)/480.0f;
        for ( i=0 ; i<5 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // = top/right
            pos.y -= 15.0f/480.0f;
        }

        uv1.x = 208.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y = 240.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        dim.x = 10.0f/640.0f;
        dim.y = 10.0f/480.0f;
        pos.x = 534.0f/640.0f;
        pos.y = 430.0f/480.0f;
        for ( i=0 ; i<3 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
        pos.x = 528.0f/640.0f;
        pos.y -= 12.0f/480.0f;
        for ( i=0 ; i<4 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
        pos.x = 534.0f/640.0f;
        pos.y -= 12.0f/480.0f;
        for ( i=0 ; i<3 ; i++ )
        {
            DrawIcon(pos, dim, uv1, uv2);  // micro
            pos.x += 12.0f/640.0f;
        }
    }
    else if ( icon == 5 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTb);
        uv1.x =  64.0f/256.0f;  // transparent green
        uv1.y = 160.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 176.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 6 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTb);
        uv1.x =  64.0f/256.0f;  // transparent red
        uv1.y = 176.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 192.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 7 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTb);
        uv1.x =  64.0f/256.0f;  // transparent blue
        uv1.y = 192.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 208.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 8 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =   0.0f/256.0f;  // opaque orange
        uv1.y =   0.0f/256.0f;
        uv2.x =  32.0f/256.0f;
        uv2.y =  32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 9 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x =  32.0f/256.0f;  // opaque gray
        uv1.y =  32.0f/256.0f;
        uv2.x =  64.0f/256.0f;
        uv2.y =  64.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 14.0f/640.0f;
        corner.y = 14.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else if ( icon == 10 )
    {
        // nothing (in the background image)!
    }
    else if ( icon == 11 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATETTb);
        uv1.x =  64.0f/256.0f;  // transparent yellow
        uv1.y = 224.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 240.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
    else if ( icon == 12 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 128.0f/256.0f;  // dirty opaque gray
        uv1.y = 128.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
    else if ( icon == 13 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 192.0f/256.0f;  //  dirty opaque blue
        uv1.y = 128.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
    else if ( icon == 14 )
    {
        m_engine->SetTexture("button1.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 160.0f/256.0f;  // dirty opaque red
        uv1.y = 128.0f/256.0f;
        uv2.x = 192.0f/256.0f;
        uv2.y = 160.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f/640.0f;
        corner.y = 6.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 5.0f/256.0f);
    }
}

// Draws hatching.

void CWindow::DrawHach(FPOINT pos, FPOINT dim)
{
#if _NEWLOOK
#else
    FPOINT      ppos, ddim, uv1, uv2;
    float       dp, max, ndim;
    BOOL        bStop;

    dp = 0.5f/256.0f;

    m_engine->SetTexture("button2.tga");
    m_engine->SetState(D3DSTATENORMAL);
    uv1.x =  64.0f/256.0f;  // hatching
    uv1.y = 208.0f/256.0f;
    uv2.x = 145.0f/256.0f;
    uv2.y = 224.0f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    max = dim.y*(uv2.x-uv1.x)/(uv2.y-uv1.y);

    ppos = pos;
    ddim = dim;
    bStop = FALSE;
    do
    {
        ddim.x = max;
        if ( ppos.x+ddim.x > pos.x+dim.x )
        {
            ndim = pos.x+dim.x-ppos.x;
            uv2.x = uv1.x+(uv2.x-uv1.x)*(ndim/ddim.x);
            ddim.x = ndim;
            bStop = TRUE;
        }
        DrawIcon(ppos, ddim, uv1, uv2);

        ppos.x += ddim.x;
    }
    while ( !bStop );
#endif
}

