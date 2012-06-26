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

// list.cpp

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
#include "button.h"
#include "scroll.h"
#include "text.h"
#include "list.h"



#define MARGING     4.0f



// Object's constructor.

CList::CList(CInstanceManager* iMan) : CControl(iMan)
{
    int     i;

    for ( i=0 ; i<LISTMAXDISPLAY ; i++ )
    {
        m_button[i] = 0;
    }
    m_scroll = 0;

    for ( i=0 ; i<LISTMAXTOTAL ; i++ )
    {
        m_text[i][0] = 0;
        m_check[i] = FALSE;
        m_enable[i] = TRUE;
    }

    for ( i=0 ; i<10 ; i++ )
    {
        m_tabs[i] = 0.0f;
        m_justifs[i] = 1;
    }

    m_totalLine = 0;
    m_displayLine = 0;
    m_selectLine = -1;
    m_firstLine = 0;
    m_bBlink = FALSE;
    m_bSelectCap = TRUE;
    m_blinkTime = 0.0f;
}

// Object's destructor.

CList::~CList()
{
    int     i;

    for ( i=0 ; i<LISTMAXDISPLAY ; i++ )
    {
        delete m_button[i];
    }
    delete m_scroll;
}


// Creates a new list.

BOOL CList::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg,
                   float expand)
{
    m_expand = expand;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
    CControl::Create(pos, dim, icon, eventMsg);

    m_scroll = new CScroll(m_iMan);
    m_scroll->Create(pos, dim, 0, EVENT_NULL);
    m_eventScroll = m_scroll->RetEventMsg();

    return MoveAdjust();
}

// Adjusted after a change of dimensions.

BOOL CList::MoveAdjust()
{
    FPOINT      ipos, idim, ppos, ddim;
    float       marging, h;
    int         i;

    for ( i=0 ; i<LISTMAXDISPLAY ; i++ )
    {
        delete m_button[i];
        m_button[i] = 0;
    }

    if ( m_icon == 0 )  marging = MARGING;
    else                marging = 0.0f;

    ipos.x = m_pos.x+marging/640.f;
    ipos.y = m_pos.y+marging/480.f;
    idim.x = m_dim.x-marging*2.0f/640.f;
    idim.y = m_dim.y-marging*2.0f/480.f;

    h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType)*m_expand;

    m_displayLine = (int)(idim.y/h);
    if ( m_displayLine == 0 )  return FALSE;
    if ( m_displayLine > LISTMAXDISPLAY )  m_displayLine = LISTMAXDISPLAY;
    idim.y = h*m_displayLine;
    m_dim.y = idim.y+marging*2.0f/480.f;

    ppos.x = ipos.x;
    ppos.y = ipos.y+idim.y-h;
    ddim.x = idim.x-SCROLL_WIDTH;
    ddim.y = h;
    for ( i=0 ; i<m_displayLine ; i++ )
    {
        m_button[i] = new CButton(m_iMan);
        m_button[i]->Create(ppos, ddim, -1, EVENT_NULL);
        m_button[i]->SetJustif(1);
        m_button[i]->SetState(STATE_SIMPLY);
        m_button[i]->SetFontType(m_fontType);
        m_button[i]->SetFontSize(m_fontSize);
        ppos.y -= h;

        m_eventButton[i] = m_button[i]->RetEventMsg();
    }

    if ( m_scroll != 0 )
    {
        ppos.x = ipos.x+idim.x-SCROLL_WIDTH;
        ppos.y = ipos.y;
        ddim.x = SCROLL_WIDTH;
        ddim.y = idim.y;
        m_scroll->SetPos(ppos);
        m_scroll->SetDim(ddim);
    }

    UpdateScroll();
    UpdateButton();
    return TRUE;
}


// Returns the message of a button.

EventMsg CList::RetEventMsgButton(int i)
{
    if ( i < 0 || i >= m_displayLine )  return EVENT_NULL;
    if ( m_button[i] == 0 )  return EVENT_NULL;
    return m_button[i]->RetEventMsg();
}

// Returns the message from the elevator.

EventMsg CList::RetEventMsgScroll()
{
    if ( m_scroll == 0 )  return EVENT_NULL;
    return m_scroll->RetEventMsg();
}


void CList::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
}

void CList::SetDim(FPOINT dim)
{
    m_dim = dim;
    MoveAdjust();
    CControl::SetDim(dim);
}


BOOL CList::SetState(int state, BOOL bState)
{
    int     i;

    if ( state & STATE_ENABLE )
    {
        for ( i=0 ; i<m_displayLine ; i++ )
        {
            if ( m_button[i] != 0 )  m_button[i]->SetState(state, bState);
        }
        if ( m_scroll != 0 )  m_scroll->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}

BOOL CList::SetState(int state)
{
    int     i;

    if ( state & STATE_ENABLE )
    {
        for ( i=0 ; i<m_displayLine ; i++ )
        {
            if ( m_button[i] != 0 )  m_button[i]->SetState(state);
        }
        if ( m_scroll != 0 )  m_scroll->SetState(state);
    }

    return CControl::SetState(state);
}

BOOL CList::ClearState(int state)
{
    int     i;

    if ( state & STATE_ENABLE )
    {
        for ( i=0 ; i<m_displayLine ; i++ )
        {
            if ( m_button[i] != 0 )  m_button[i]->ClearState(state);
        }
        if ( m_scroll != 0 )  m_scroll->ClearState(state);
    }

    return CControl::ClearState(state);
}


// Management of an event.

BOOL CList::EventProcess(const Event &event)
{
    int     i;

    if ( m_bBlink                   &&  // blinks?
         event.event == EVENT_FRAME )
    {
        i = m_selectLine-m_firstLine;

        if ( i >= 0 && i < 4  &&
             m_button[i] != 0 )
        {
            m_blinkTime += event.rTime;
            if ( Mod(m_blinkTime, 0.7f) < 0.3f )
            {
                m_button[i]->ClearState(STATE_ENABLE);
                m_button[i]->ClearState(STATE_CHECK);
            }
            else
            {
                m_button[i]->SetState(STATE_ENABLE);
                m_button[i]->SetState(STATE_CHECK);
            }
        }
    }

    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
    if ( (m_state & STATE_ENABLE) == 0 )  return TRUE;

    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELUP    &&
         Detect(event.pos)            )
    {
        if ( m_firstLine > 0 )  m_firstLine --;
        UpdateScroll();
        UpdateButton();
        return TRUE;
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELDOWN  &&
         Detect(event.pos)            )
    {
        if ( m_firstLine < m_totalLine-m_displayLine )  m_firstLine ++;
        UpdateScroll();
        UpdateButton();
        return TRUE;
    }

    CControl::EventProcess(event);

    if ( event.event == EVENT_MOUSEMOVE && Detect(event.pos) )
    {
        m_engine->SetMouseType(D3DMOUSENORM);
        for ( i=0 ; i<m_displayLine ; i++ )
        {
            if ( i+m_firstLine >= m_totalLine )  break;
            if ( m_button[i] != 0 )
            {
                m_button[i]->EventProcess(event);
            }
        }
    }

    if ( m_bSelectCap )
    {
        for ( i=0 ; i<m_displayLine ; i++ )
        {
            if ( i+m_firstLine >= m_totalLine )  break;
            if ( m_button[i] != 0 )
            {
                if ( !m_button[i]->EventProcess(event) )  return FALSE;

                if ( event.event == m_eventButton[i] )
                {
                    SetSelect(m_firstLine+i);

                    Event newEvent = event;
                    newEvent.event = m_eventMsg;
                    m_event->AddEvent(newEvent);  // selected line changes
                }
            }
        }
    }

    if ( m_scroll != 0 )
    {
        if ( !m_scroll->EventProcess(event) )  return FALSE;

        if ( event.event == m_eventScroll )
        {
            MoveScroll();
            UpdateButton();
        }
    }

    return TRUE;
}


// Draws the list.

void CList::Draw()
{
    FPOINT  uv1, uv2, corner, pos, dim, ppos, ddim;
    float   dp;
    int     i, j;
    char    text[100];
    char    *pb, *pe;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    dp = 0.5f/256.0f;

    if ( m_icon != -1 )
    {
        dim = m_dim;

        if ( m_icon == 0 )
        {
            m_engine->SetTexture("button2.tga");
            m_engine->SetState(D3DSTATENORMAL);

            uv1.x = 128.0f/256.0f;
            uv1.y =  64.0f/256.0f;  // u-v texture
            uv2.x = 160.0f/256.0f;
            uv2.y =  96.0f/256.0f;
        }
        else
        {
            m_engine->SetTexture("button2.tga");
            m_engine->SetState(D3DSTATENORMAL);

            uv1.x = 132.0f/256.0f;
            uv1.y =  68.0f/256.0f;  // u-v texture
            uv2.x = 156.0f/256.0f;
            uv2.y =  92.0f/256.0f;

            if ( m_button[0] != 0 )
            {
                dim = m_button[0]->RetDim();
                dim.y *= m_displayLine;  // background sounds spot behind
            }
        }

        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        corner.x = 10.0f/640.0f;
        corner.y = 10.0f/480.0f;
        DrawIcon(m_pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }

    if ( m_totalLine < m_displayLine )  // no buttons to the bottom?
    {
        i = m_totalLine;
        if ( m_button[i] != 0 )
        {
            pos = m_button[i]->RetPos();
            dim = m_button[i]->RetDim();
            pos.y += dim.y*1.1f;
            dim.y *= 0.4f;
            pos.y -= dim.y;

            m_engine->SetTexture("button2.tga");
            m_engine->SetState(D3DSTATETTw);
            uv1.x = 120.0f/256.0f;
            uv1.y =  64.0f/256.0f;
            uv2.x = 128.0f/256.0f;
            uv2.y =  48.0f/256.0f;
            uv1.x += dp;
            uv1.y -= dp;
            uv2.x -= dp;
            uv2.y += dp;
            DrawIcon(pos, dim, uv1, uv2);  // ch'tite shadow cute  (?)
        }
    }

    for ( i=0 ; i<m_displayLine ; i++ )
    {
        if ( i+m_firstLine >= m_totalLine )  break;

        if ( m_button[i] != 0 )
        {
            if ( !m_bBlink && i+m_firstLine < m_totalLine )
            {
                m_button[i]->SetState(STATE_ENABLE, m_enable[i+m_firstLine] && (m_state & STATE_ENABLE) );
            }
            m_button[i]->Draw();  // draws a box without text

            // draws text in the box
            pos = m_button[i]->RetPos();
            dim = m_button[i]->RetDim();
            if ( m_tabs[0] == 0.0f )
            {
                ppos.x = pos.x+dim.y*0.5f;
                ppos.y = pos.y+dim.y*0.5f;
                ppos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
                ddim.x = dim.x-dim.y;
                DrawCase(m_text[i+m_firstLine], ppos, ddim.x, 1);
            }
            else
            {
                ppos.x = pos.x+dim.y*0.5f;
                ppos.y = pos.y+dim.y*0.5f;
                ppos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
                pb = m_text[i+m_firstLine];
                for ( j=0 ; j<10 ; j++ )
                {
                    pe = strchr(pb, '\t');
                    if ( pe == 0 )
                    {
                        strcpy(text, pb);
                    }
                    else
                    {
                        strncpy(text, pb, pe-pb);
                        text[pe-pb] = 0;
                    }
                    DrawCase(text, ppos, m_tabs[j], m_justifs[j]);

                    if ( pe == 0 )  break;
                    ppos.x += m_tabs[j];
                    pb = pe+1;
                }
            }

            if ( (m_state & STATE_EXTEND) && i < m_totalLine )
            {
                pos = m_button[i]->RetPos();
                dim = m_button[i]->RetDim();
                pos.x += dim.x-dim.y*0.75f;
                dim.x = dim.y*0.75f;
                pos.x += 2.0f/640.0f;
                pos.y += 2.0f/480.0f;
                dim.x -= 4.0f/640.0f;
                dim.y -= 4.0f/480.0f;

                if ( m_check[i+m_firstLine] )
                {
                    m_engine->SetTexture("button1.tga");
                    m_engine->SetState(D3DSTATENORMAL);
                    uv1.x = 64.0f/256.0f;
                    uv1.y =  0.0f/256.0f;
                    uv2.x = 96.0f/256.0f;
                    uv2.y = 32.0f/256.0f;
                    uv1.x += dp;
                    uv1.y += dp;
                    uv2.x -= dp;
                    uv2.y -= dp;
                    DrawIcon(pos, dim, uv1, uv2);  // square shape

                    m_engine->SetState(D3DSTATETTw);
                    uv1.x =  0.0f/256.0f;  // v
                    uv1.y = 64.0f/256.0f;
                    uv2.x = 32.0f/256.0f;
                    uv2.y = 96.0f/256.0f;
                    uv1.x += dp;
                    uv1.y += dp;
                    uv2.x -= dp;
                    uv2.y -= dp;
                    DrawIcon(pos, dim, uv1, uv2);  // draws v
                }
                else
                {
                    m_engine->SetTexture("button1.tga");
                    m_engine->SetState(D3DSTATETTw);
                    if ( i+m_firstLine == m_selectLine )
                    {
                        uv1.x =224.0f/256.0f;  // <
                        uv1.y =192.0f/256.0f;
                        uv2.x =256.0f/256.0f;
                        uv2.y =224.0f/256.0f;
                    }
                    else
                    {
                        uv1.x = 96.0f/256.0f;  // x
                        uv1.y = 32.0f/256.0f;
                        uv2.x =128.0f/256.0f;
                        uv2.y = 64.0f/256.0f;
                    }
                    uv1.x += dp;
                    uv1.y += dp;
                    uv2.x -= dp;
                    uv2.y -= dp;
                    DrawIcon(pos, dim, uv1, uv2);  // draws x
                }
            }
        }
    }

    if ( m_scroll != 0 )
    {
        m_scroll->Draw();  // draws the lift
    }
}

// Displays text in a box.

void CList::DrawCase(char *text, FPOINT pos, float width, int justif)
{
    if ( justif == 1 )
    {
        m_engine->RetText()->DrawText(text, pos, width, 1, m_fontSize, m_fontStretch, m_fontType, 0);
    }
    else if ( justif == 0 )
    {
        pos.x += width/2.0f;
        m_engine->RetText()->DrawText(text, pos, width, 0, m_fontSize, m_fontStretch, m_fontType, 0);
    }
    else
    {
        pos.x += width;
        m_engine->RetText()->DrawText(text, pos, width, -1, m_fontSize, m_fontStretch, m_fontType, 0);
    }
}


// Empty the list completely.

void CList::Flush()
{
    m_totalLine = 0;
    m_selectLine = -1;
    m_firstLine = 0;
    UpdateButton();
    UpdateScroll();
}


// Specifies the total number of lines.

void CList::SetTotal(int i)
{
    m_totalLine = i;
}

// Returns the total number of lines.

int CList::RetTotal()
{
    return m_totalLine;
}


// Selects a line.

void CList::SetSelect(int i)
{
    if ( m_bSelectCap )
    {
        m_selectLine = i;
    }
    else
    {
        m_firstLine = i;
        UpdateScroll();
    }

    UpdateButton();
}

// Returns the selected line.

int CList::RetSelect()
{
    if ( m_bSelectCap )
    {
        return m_selectLine;
    }
    else
    {
        return m_firstLine;
    }
}


// Management of capability has a select box.

void CList::SetSelectCap(BOOL bEnable)
{
    m_bSelectCap = bEnable;
}

BOOL CList::RetSelectCap()
{
    return m_bSelectCap;
}


// Blink a line.

void CList::SetBlink(BOOL bEnable)
{
    int     i;

    m_bBlink = bEnable;
    m_blinkTime = 0.0f;

    i = m_selectLine-m_firstLine;

    if ( i >= 0 && i < 4  &&
         m_button[i] != 0 )
    {
        if ( !bEnable )
        {
            m_button[i]->SetState(STATE_CHECK);
            m_button[i]->ClearState(STATE_ENABLE);
        }
    }
}

BOOL CList::RetBlink()
{
    return m_bBlink;
}


// Specifies the text of a line.

void CList::SetName(int i, char* name)
{
    if ( i < 0 || i >= LISTMAXTOTAL )  return;

    if ( i >= m_totalLine )
    {
        m_totalLine = i+1;  // expands the list
    }

    if ( name[0] == 0 )
    {
        strcpy(m_text[i], " ");
    }
    else
    {
        strcpy(m_text[i], name);
    }
    UpdateButton();
    UpdateScroll();
}

// Returns the text of a line.

char* CList::RetName(int i)
{
    if ( i < 0 || i >= m_totalLine )  return 0;

    return m_text[i];
}


// Specifies the bit "check" for a box.

void CList::SetCheck(int i, BOOL bMode)
{
    if ( i < 0 || i >= m_totalLine )  return;

    m_check[i] = bMode;
}

// Returns the bit "check" for a box.

BOOL CList::RetCheck(int i)
{
    if ( i < 0 || i >= m_totalLine )  return FALSE;

    return m_check[i];
}


// Specifies the bit "enable" for a box.

void CList::SetEnable(int i, BOOL bMode)
{
    if ( i < 0 || i >= m_totalLine )  return;

    m_enable[i] = bMode;
}

// Returns the bit "enable" for a box.

BOOL CList::RetEnable(int i)
{
    if ( i < 0 || i >= m_totalLine )  return FALSE;

    return m_enable[i];
}


// Management of the position of the tabs.

void CList::SetTabs(int i, float pos, int justif)
{
    if ( i < 0 || i >= 10 )  return;
    m_tabs[i] = pos;
    m_justifs[i] = justif;
}

float  CList::RetTabs(int i)
{
    if ( i < 0 || i >= 10 )  return 0.0f;
    return m_tabs[i];
}


// Moves the lift to see the list of the selected line.

void CList::ShowSelect(BOOL bFixed)
{
    int     sel;

    if ( bFixed &&
         m_selectLine >= m_firstLine &&
         m_selectLine <  m_firstLine+m_displayLine )  return;  // all good

    sel = m_selectLine;

    // Down from 1/2 * h.
    sel += m_displayLine/2;
    if ( sel > m_totalLine-1 )  sel = m_totalLine-1;

    // Back to h-1.
    sel -= m_displayLine-1;
    if ( sel < 0 )  sel = 0;

    m_firstLine = sel;

    UpdateButton();
    UpdateScroll();
}


// Updates all button names.

void CList::UpdateButton()
{
    int     state, i, j;

    state = CControl::RetState();

    j = m_firstLine;
    for ( i=0 ; i<m_displayLine ; i++ )
    {
        if ( m_button[i] == 0 )  continue;

        m_button[i]->SetState(STATE_CHECK, (j == m_selectLine));

        if ( j < m_totalLine )
        {
//?         m_button[i]->SetName(m_text[j]);
            m_button[i]->SetName(" ");  // blank button
            m_button[i]->SetState(STATE_ENABLE, (state & STATE_ENABLE));
        }
        else
        {
            m_button[i]->SetName(" ");  // blank button
            m_button[i]->ClearState(STATE_ENABLE);
        }
        j ++;
    }
}

// Updates the lift.

void CList::UpdateScroll()
{
    float   ratio, value, step;

    if ( m_scroll == 0 )  return;

    if ( m_totalLine <= m_displayLine )
    {
        ratio = 1.0f;
        value = 0.0f;
        step = 0.0f;
    }
    else
    {
        ratio = (float)m_displayLine/m_totalLine;
        if ( ratio > 1.0f )  ratio = 1.0f;

        value = (float)m_firstLine/(m_totalLine-m_displayLine);
        if ( value < 0.0f )  value = 0.0f;
        if ( value > 1.0f )  value = 1.0f;

        step = (float)1.0f/(m_totalLine-m_displayLine);
        if ( step < 0.0f )  step = 0.0f;
    }

    m_scroll->SetVisibleRatio(ratio);
    m_scroll->SetVisibleValue(value);
    m_scroll->SetArrowStep(step);
}

// Update when the lift is moving.

void CList::MoveScroll()
{
    float   pos;
    int     n;

    if ( m_scroll == 0 )  return;

    n = m_totalLine-m_displayLine;
    pos = m_scroll->RetVisibleValue();
    pos += m_scroll->RetArrowStep()/2.0f;  // it's magic!
    m_firstLine = (int)(pos*n);
    if ( m_firstLine < 0 )  m_firstLine = 0;
    if ( m_firstLine > n )  m_firstLine = n;
}


