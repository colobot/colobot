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

// edit.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "scroll.h"
#include "text.h"
#include "edit.h"


#define MARGX           (5.0f/640.0f)
#define MARGY           (5.0f/480.0f)
#define MARGYS          (4.0f/480.0f)
#define MARGY1          (1.0f/480.0f)
#define DELAY_DBCLICK   0.3f            // time limit for double-click
#define DELAY_SCROLL    0.1f            // time limit for scroll
#define BIG_FONT    1.6f            // expansion for \b;




// Indicates whether a character is a space.

BOOL IsSpace(int character)
{
    return ( character == ' '  ||
             character == '\t' ||
             character == '\n' );
}

// Indicates whether a character is part of a word.

BOOL IsWord(int character)
{
    char    c;

    c = tolower(RetNoAccent(character));

    return ( (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') ||
             c == '_' );
}

// Indicates whether a character is a word separator.

BOOL IsSep(int character)
{
    if ( IsSpace(character) )  return FALSE;
    return !IsWord(character);
}



// Object's constructor.

CEdit::CEdit(CInstanceManager* iMan) : CControl(iMan)
{
    FPOINT  pos;
    int     i;

    m_maxChar = 100;
    m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));
    m_format = 0;
    m_len = 0;

    m_fontType = FONT_COURIER;
    m_scroll        = 0;
    m_bEdit         = TRUE;
    m_bHilite       = TRUE;
    m_bInsideScroll = TRUE;
    m_bCapture      = FALSE;
    m_bDisplaySpec  = FALSE;
    m_bSoluce       = FALSE;
    m_bGeneric      = FALSE;
    m_bAutoIndent   = FALSE;
    m_cursor1       = 0;
    m_cursor2       = 0;
    m_column        = 0;
    m_imageTotal    = 0;

    HyperFlush();

    for ( i=0 ; i<EDITUNDOMAX ; i++ )
    {
        m_undo[i].text = 0;
    }
    m_bUndoForce = TRUE;
    m_undoOper = OPERUNDO_SPEC;
}

// Object's destructor.

CEdit::~CEdit()
{
    int     i;

    FreeImage();

    for ( i=0 ; i<EDITUNDOMAX ; i++ )
    {
        delete m_undo[i].text;
    }

    delete m_text;
    delete m_format;
    delete m_scroll;
}


// Creates a new editable line.

BOOL CEdit::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    CScroll*    pc;
    FPOINT      start, end;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();
    CControl::Create(pos, dim, icon, eventMsg);

    m_len = 0;
    m_lineFirst = 0;
    m_time = 0.0f;
    m_timeBlink = 0.0f;
    m_timeLastClick = 0.0f;
    m_timeLastScroll = 0.0f;

    m_bMulti = FALSE;
    MoveAdjust();
    if ( m_lineVisible <= 1 )
    {
        m_bMulti = FALSE;
    }
    else
    {
        m_bMulti = TRUE;
        MoveAdjust();  // readjusts multi-line mode
        m_scroll = new CScroll(m_iMan);
        pc = (CScroll*)m_scroll;
        pc->Create(pos, dim, -1, EVENT_NULL);
        MoveAdjust();
    }

    return TRUE;
}


void CEdit::SetPos(FPOINT pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CEdit::SetDim(FPOINT dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CEdit::MoveAdjust()
{
    FPOINT      pos, dim;
    float       height;

    m_lineDescent = m_engine->RetText()->RetDescent(m_fontSize, m_fontType);
    m_lineAscent  = m_engine->RetText()->RetAscent(m_fontSize, m_fontType);
    m_lineHeight  = m_engine->RetText()->RetHeight(m_fontSize, m_fontType);

    height = m_dim.y-(m_bMulti?MARGY*2.0f:MARGY1);
    m_lineVisible = (int)(height/m_lineHeight);

    if ( m_scroll != 0 )
    {
        if ( m_bInsideScroll )
        {
            pos.x = m_pos.x+m_dim.x-MARGX-SCROLL_WIDTH;
            pos.y = m_pos.y+MARGYS;
            dim.x = SCROLL_WIDTH;
            dim.y = m_dim.y-MARGYS*2.0f;
        }
        else
        {
            pos.x = m_pos.x+m_dim.x-SCROLL_WIDTH;
            pos.y = m_pos.y;
            dim.x = SCROLL_WIDTH;
            dim.y = m_dim.y;
        }
        m_scroll->SetPos(pos);
        m_scroll->SetDim(dim);
    }

    Justif();

    if ( m_lineFirst > m_lineTotal-m_lineVisible )
    {
        m_lineFirst = m_lineTotal-m_lineVisible;
        if ( m_lineFirst < 0 )  m_lineFirst = 0;
    }

    pos.x = m_pos.x+m_dim.x-(m_bMulti?SCROLL_WIDTH:0.0f);
    pos.y = m_pos.y;
    GlintCreate(pos, FALSE, FALSE);
}


// Management of an event.

BOOL CEdit::EventProcess(const Event &event)
{
    BOOL    bShift, bControl;

    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;

    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELUP    &&
         Detect(event.pos)            )
    {
        Scroll(m_lineFirst-3, TRUE);
        return TRUE;
    }
    if ( event.event == EVENT_KEYDOWN &&
         event.param == VK_WHEELDOWN  &&
         Detect(event.pos)            )
    {
        Scroll(m_lineFirst+3, TRUE);
        return TRUE;
    }

    CControl::EventProcess(event);

    if ( event.event == EVENT_FRAME )
    {
        m_time += event.rTime;
        m_timeBlink += event.rTime;
    }

    if ( event.event == EVENT_MOUSEMOVE )
    {
        if ( Detect(event.pos) &&
             event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
        {
            if ( m_bEdit )
            {
                m_engine->SetMouseType(D3DMOUSEEDIT);
            }
            else
            {
                if ( IsLinkPos(event.pos) )
                {
                    m_engine->SetMouseType(D3DMOUSEHAND);
                }
                else
                {
                    m_engine->SetMouseType(D3DMOUSENORM);
                }
            }
        }
    }

    if ( m_scroll != 0 && !m_bGeneric )
    {
        m_scroll->EventProcess(event);

        if ( event.event == m_scroll->RetEventMsg() )
        {
            Scroll();
            return TRUE;
        }
    }

    if ( event.event == EVENT_KEYDOWN && m_bFocus )
    {
        bShift   = (event.keyState&KS_SHIFT);
        bControl = (event.keyState&KS_CONTROL);

        if ( (event.param == 'X'       && !bShift &&  bControl) ||
             (event.param == VK_DELETE &&  bShift && !bControl) )
        {
            Cut();
            return TRUE;
        }
        if ( (event.param == 'C'       && !bShift &&  bControl) ||
             (event.param == VK_INSERT && !bShift &&  bControl) )
        {
            Copy();
            return TRUE;
        }
        if ( (event.param == 'V'       && !bShift &&  bControl) ||
             (event.param == VK_INSERT &&  bShift && !bControl) )
        {
            Paste();
            return TRUE;
        }

        if ( event.param == 'A' && !bShift && bControl )
        {
            SetCursor(999999, 0);
            return TRUE;
        }

        if ( event.param == 'O' && !bShift && bControl )
        {
            Event   newEvent;
            m_event->MakeEvent(newEvent, EVENT_STUDIO_OPEN);
            m_event->AddEvent(newEvent);
        }
        if ( event.param == 'S' && !bShift && bControl )
        {
            Event   newEvent;
            m_event->MakeEvent(newEvent, EVENT_STUDIO_SAVE);
            m_event->AddEvent(newEvent);
        }

        if ( event.param == 'Z' && !bShift && bControl )
        {
            Undo();
            return TRUE;
        }

        if ( event.param == 'U' && !bShift && bControl )
        {
            if ( MinMaj(FALSE) )  return TRUE;
        }
        if ( event.param == 'U' && bShift && bControl )
        {
            if ( MinMaj(TRUE) )  return TRUE;
        }

        if ( event.param == VK_TAB && !bShift && !bControl && !m_bAutoIndent )
        {
            if ( Shift(FALSE) )  return TRUE;
        }
        if ( event.param == VK_TAB && bShift && !bControl && !m_bAutoIndent )
        {
            if ( Shift(TRUE) )  return TRUE;
        }

        if ( m_bEdit )
        {
            if ( event.param == VK_LEFT )
            {
                MoveChar(-1, bControl, bShift);
                return TRUE;
            }
            if ( event.param == VK_RIGHT )
            {
                MoveChar(1, bControl, bShift);
                return TRUE;
            }
            if ( event.param == VK_UP )
            {
                MoveLine(-1, bControl, bShift);
                return TRUE;
            }
            if ( event.param == VK_DOWN )
            {
                MoveLine(1, bControl, bShift);
                return TRUE;
            }

            if ( event.param == VK_PRIOR )  // PageUp ?
            {
                MoveLine(-(m_lineVisible-1), bControl, bShift);
                return TRUE;
            }
            if ( event.param == VK_NEXT )  // PageDown ?
            {
                MoveLine(m_lineVisible-1, bControl, bShift);
                return TRUE;
            }
        }
        else
        {
            if ( event.param == VK_LEFT ||
                 event.param == VK_UP   )
            {
                Scroll(m_lineFirst-1, TRUE);
                return TRUE;
            }
            if ( event.param == VK_RIGHT ||
                 event.param == VK_DOWN  )
            {
                Scroll(m_lineFirst+1, TRUE);
                return TRUE;
            }

            if ( event.param == VK_PRIOR )  // PageUp ?
            {
                Scroll(m_lineFirst-(m_lineVisible-1), TRUE);
                return TRUE;
            }
            if ( event.param == VK_NEXT )  // PageDown ?
            {
                Scroll(m_lineFirst+(m_lineVisible-1), TRUE);
                return TRUE;
            }
        }

        if ( event.param == VK_HOME )
        {
            MoveHome(bControl, bShift);
            return TRUE;
        }
        if ( event.param == VK_END )
        {
            MoveEnd(bControl, bShift);
            return TRUE;
        }

        if ( event.param == VK_BACK )  // backspace ( <- ) ?
        {
            Delete(-1);
            SendModifEvent();
            return TRUE;
        }
        if ( event.param == VK_DELETE )
        {
            Delete(1);
            SendModifEvent();
            return TRUE;
        }

        if ( event.param == VK_RETURN )
        {
            Insert('\n');
            SendModifEvent();
            return TRUE;
        }
        if ( event.param == VK_TAB )
        {
            Insert('\t');
            SendModifEvent();
            return TRUE;
        }
    }

    if ( event.event == EVENT_CHAR && m_bFocus )
    {
        if ( event.param >= ' ' && event.param <= 255 )
        {
            Insert((char)event.param);
            SendModifEvent();
            return TRUE;
        }
    }

    if ( event.event == EVENT_FOCUS )
    {
        if ( event.param == m_eventMsg )
        {
            m_bFocus = TRUE;
        }
        else
        {
            m_bFocus = FALSE;
        }
    }

    if ( event.event == EVENT_LBUTTONDOWN )
    {
        m_mouseFirstPos = event.pos;
        m_mouseLastPos  = event.pos;
        if ( Detect(event.pos) )
        {
            if ( event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
            {
                MouseClick(event.pos);
                if ( m_bEdit || m_bHilite )  m_bCapture = TRUE;
            }
            m_bFocus = TRUE;
        }
        else
        {
            m_bFocus = FALSE;
        }
    }

    if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
    {
        m_mouseLastPos = event.pos;
        MouseMove(event.pos);
    }

    if ( event.event == EVENT_FRAME && m_bCapture )
    {
        MouseMove(m_mouseLastPos);
    }

    if ( event.event == EVENT_LBUTTONUP )
    {
        if ( Detect(event.pos) )
        {
            if ( event.pos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
            {
                MouseRelease(m_mouseFirstPos);
            }
        }
        if ( m_bCapture )
        {
            if ( m_timeLastClick+DELAY_DBCLICK > m_time )  // double-click ?
            {
                MouseDoubleClick(event.pos);
            }
            m_timeLastClick = m_time;
            m_bCapture = FALSE;
        }
    }

    return TRUE;
}


// Sends an event to indicate that the text was modified.

void CEdit::SendModifEvent()
{
    Event   newEvent;

    m_event->MakeEvent(newEvent, m_eventMsg);
    m_event->AddEvent(newEvent);
}


// Detects whether the mouse is over a hyperlink character.

BOOL CEdit::IsLinkPos(FPOINT pos)
{
    int     i;

    if ( m_format == 0 )  return FALSE;

    i = MouseDetect(pos);
    if ( i == -1 )  return FALSE;
    if ( i >= m_len )  return FALSE;

    if ( (m_format[i]&COLOR_MASK) == COLOR_LINK )  return TRUE;
    return FALSE;
}


// Positions the cursor after a double click.

void CEdit::MouseDoubleClick(FPOINT mouse)
{
    int     i, character;

    if ( m_bMulti )  // Multi-line?
    {
        i = MouseDetect(mouse);
        if ( i == -1 )  return;

        while ( i > 0 )
        {
            character = (unsigned char)m_text[i-1];
            if ( !IsWord(character) )  break;
            i --;
        }
        m_cursor2 = i;

        while ( i < m_len )
        {
            character = (unsigned char)m_text[i];
            if ( !IsWord(character) )  break;
            i ++;
        }
        m_cursor1 = i;
    }
    else    // single-line?
    {
        m_cursor2 = 0;
        m_cursor1 = m_len;  // selects all
    }

    m_bUndoForce = TRUE;

    Justif();
    ColumnFix();
}

// Positions the cursor when clicked.

void CEdit::MouseClick(FPOINT mouse)
{
    int     i;

    i = MouseDetect(mouse);
    if ( i == -1 )  return;

    if ( m_bEdit || m_bHilite )
    {
        m_cursor1 = i;
        m_cursor2 = i;
        m_bUndoForce = TRUE;
        m_timeBlink = 0.0f;  // lights the cursor immediately
        ColumnFix();
    }
}

// Positions the cursor when clicked released.

void CEdit::MouseRelease(FPOINT mouse)
{
    int     i, j, rank;

    i = MouseDetect(mouse);
    if ( i == -1 )  return;

    if ( !m_bEdit )
    {
        if ( m_format != 0 && i < m_len && m_cursor1 == m_cursor2 &&
            (m_format[i]&COLOR_MASK) == COLOR_LINK )
        {
            rank = -1;
            for ( j=0 ; j<=i ; j++ )
            {
                if ( (j == 0 || (m_format[j-1]&COLOR_MASK) != COLOR_LINK) &&
                     (m_format[j+0]&COLOR_MASK) == COLOR_LINK )
                {
                    rank ++;
                }
            }
            HyperJump(m_link[rank].name, m_link[rank].marker);
        }
    }
}

// Positions the cursor after movement.

void CEdit::MouseMove(FPOINT mouse)
{
    int     i;

    if ( m_bMulti &&
         m_timeLastScroll+DELAY_SCROLL <= m_time )
    {
        if ( mouse.y > m_pos.y+m_dim.y )  // above?
        {
            Scroll(m_lineFirst-1, FALSE);
            mouse.y = m_pos.y+m_dim.y-MARGY-m_lineHeight/2.0f;
        }
        if ( mouse.y < m_pos.y )  // lower?
        {
            Scroll(m_lineFirst+1, FALSE);
            mouse.y = m_pos.y+m_dim.y-MARGY-m_lineVisible*m_lineHeight+m_lineHeight/2.0f;
        }
        m_timeLastScroll = m_time;
    }

    i = MouseDetect(mouse);
    if ( i != -1 )
    {
        m_cursor1 = i;
        m_bUndoForce = TRUE;
        m_timeBlink = 0.0f;  // lights the cursor immediately
        ColumnFix();
    }
}

// Positions the cursor when clicked.

int CEdit::MouseDetect(FPOINT mouse)
{
    FPOINT  pos;
    float   indentLength, offset, size;
    int     i, len, c;
    BOOL    bTitle;

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
                        * m_engine->RetEditIndentValue();
    }

    pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1);
    for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
    {
        bTitle = ( m_format != 0 && (m_format[m_lineOffset[i]]&TITLE_MASK) == TITLE_BIG );

        if ( i >= m_lineFirst+m_lineVisible )  break;

        pos.x = m_pos.x+(10.0f/640.0f);
        if ( m_bAutoIndent )
        {
            pos.x += indentLength*m_lineIndent[i];
        }
        offset = mouse.x-pos.x;

        if ( bTitle )  pos.y -= m_lineHeight;

        if ( mouse.y > pos.y )
        {
            len = m_lineOffset[i+1] - m_lineOffset[i];

            if ( m_format == 0 )
            {
                c = m_engine->RetText()->Detect(m_text+m_lineOffset[i],
                                                len, offset, m_fontSize,
                                                m_fontStretch, m_fontType);
            }
            else
            {
                size = m_fontSize;
                if ( bTitle )  size *= BIG_FONT;

                c = m_engine->RetText()->Detect(m_text+m_lineOffset[i],
                                                m_format+m_lineOffset[i],
                                                len, offset, size,
                                                m_fontStretch);
            }
            return m_lineOffset[i]+c;
        }

        if ( bTitle )  i ++;
        pos.y -= m_lineHeight;
    }
    return -1;
}


// Clears all history.

void CEdit::HyperFlush()
{
    m_historyTotal = 0;
    m_historyCurrent = -1;
}

// Indicates which is the home page.

void CEdit::HyperHome(char *filename)
{
    HyperFlush();
    HyperAdd(filename, 0);
}

// Performs a hyper jump through a link.

void CEdit::HyperJump(char *name, char *marker)
{
    char    filename[100];
    char    sMarker[100];
    int     i, line, pos;

    if ( m_historyCurrent >= 0 )
    {
        m_history[m_historyCurrent].firstLine = m_lineFirst;
    }

    strcpy(sMarker, marker);

//? sprintf(filename, "help\\%s.txt", name);
    if ( name[0] == '%' )
    {
        UserDir(filename, name, "");
        strcat(filename, ".txt");
    }
    else
    {
        sprintf(filename, "help\\%s.txt", name);
    }
    if ( ReadText(filename) )
    {
        Justif();

        line = 0;
        for ( i=0 ; i<m_markerTotal ; i++ )
        {
            if ( strcmp(sMarker, m_marker[i].name) == 0 )
            {
                pos = m_marker[i].pos;
                for ( i=0 ; i<m_lineTotal ; i++ )
                {
                    if ( pos >= m_lineOffset[i] )
                    {
                        line = i;
                    }
                }
                break;
            }
        }

        SetFirstLine(line);
        HyperAdd(filename, line);
    }
}

// Adds text to the history of visited.

BOOL CEdit::HyperAdd(char *filename, int firstLine)
{
    if ( m_historyCurrent >= EDITHISTORYMAX-1 )  return FALSE;

    m_historyCurrent ++;
    strcpy(m_history[m_historyCurrent].filename, filename);
    m_history[m_historyCurrent].firstLine = firstLine;

    m_historyTotal = m_historyCurrent+1;
    return TRUE;
}

// Indicates whether a button EVENT_HYPER_ * is active or not.

BOOL CEdit::HyperTest(EventMsg event)
{
    if ( event == EVENT_HYPER_HOME )
    {
        return ( m_historyCurrent > 0 );
    }

    if ( event == EVENT_HYPER_PREV )
    {
        return ( m_historyCurrent > 0 );
    }

    if ( event == EVENT_HYPER_NEXT )
    {
        return ( m_historyCurrent < m_historyTotal-1 );
    }

    return FALSE;
}

// Performs the action corresponding to a button EVENT_HYPER_ *.

BOOL CEdit::HyperGo(EventMsg event)
{
    if ( !HyperTest(event) )  return FALSE;

    m_history[m_historyCurrent].firstLine = m_lineFirst;

    if ( event == EVENT_HYPER_HOME )
    {
        m_historyCurrent = 0;
    }

    if ( event == EVENT_HYPER_PREV )
    {
        m_historyCurrent --;
    }

    if ( event == EVENT_HYPER_NEXT )
    {
        m_historyCurrent ++;
    }

    ReadText(m_history[m_historyCurrent].filename);
    Justif();
    SetFirstLine(m_history[m_historyCurrent].firstLine);
    return TRUE;
}


// Draw the editable line.

void CEdit::Draw()
{
    FPOINT      pos, ppos, dim, start, end;
    float       size, indentLength;
    int         i, j, beg, len, c1, c2, o1, o2, eol, iIndex, line;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    pos.x = m_pos.x;
    pos.y = m_pos.y;
    dim.x = m_dim.x;
    if ( !m_bInsideScroll )  dim.x -= m_bMulti?SCROLL_WIDTH:0.0f;
    dim.y = m_dim.y;
    DrawBack(pos, dim);  // background

    // Displays all lines.
    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Swap(c1, c2);  // always c1 <= c2

    if ( m_bInsideScroll )
    {
        dim.x -= m_bMulti?SCROLL_WIDTH:0.0f + (1.0f/640.0f);
    }

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
                        * m_engine->RetEditIndentValue();
    }

    pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1);
    for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
    {
        if ( i == m_lineFirst && i < m_lineTotal-1 &&
             m_lineOffset[i] == m_lineOffset[i+1] )
        {
            pos.y -= m_lineHeight;  // Double jump line \b;
            i ++;
        }

        if ( i >= m_lineFirst+m_lineVisible )  break;

        pos.x = m_pos.x+(10.0f/640.0f);
        if ( m_bAutoIndent )
        {
            for ( j=0 ; j<m_lineIndent[i] ; j++ )
            {
                char s = '\t';  // line | dotted
                m_engine->RetText()->DrawText(&s, 1, pos, 1.0f, 1, m_fontSize, m_fontStretch, m_fontType, 0);
                pos.x += indentLength;
            }
        }

        beg = m_lineOffset[i];
        len = m_lineOffset[i+1] - m_lineOffset[i];

        ppos = pos;
        size = m_fontSize;

        // Headline \b;?
        if ( beg+len < m_len && m_format != 0 &&
             (m_format[beg]&TITLE_MASK) == TITLE_BIG )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1)-m_lineHeight*(BIG_FONT-1.0f);
            end.y   = m_lineHeight*BIG_FONT;
            DrawPart(start, end, 2);  // blue gradient background ->

            size *= BIG_FONT;
            ppos.y -= m_lineHeight*(BIG_FONT-1.0f);
        }

        // As \t;?
        if ( beg+len < m_len && m_format != 0 &&
             (m_format[beg]&TITLE_MASK) == TITLE_NORM )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 2);  // blue gradient background ->
        }

        // Subtitle \s;?
        if ( beg+len < m_len && m_format != 0 &&
             (m_format[beg]&TITLE_MASK) == TITLE_LITTLE )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 3);  // yellow background gradient ->
        }

        // Table \tab;?
        if ( beg+len < m_len && m_format != 0 &&
             (m_format[beg]&COLOR_MASK) == COLOR_TABLE )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 11);  // fond orange d�grad� ->
        }

        // Image \image; ?
        if ( beg+len < m_len && m_format != 0 &&
             (m_format[beg]&IMAGE_MASK) != 0 )
        {
            line = 1;
            while ( true )  // includes the image slices
            {
                if ( i+line >= m_lineTotal                ||
                     i+line >= m_lineFirst+m_lineVisible  ||
                     (m_format[beg+line]&IMAGE_MASK) == 0 )  break;
                line ++;
            }

            iIndex = m_text[beg];  // character = index in m_image
            pos.y -= m_lineHeight*(line-1);
            DrawImage(pos, m_image[iIndex].name,
                      m_image[iIndex].width*(m_fontSize/SMALLFONT),
                      m_image[iIndex].offset, m_image[iIndex].height*line, line);
            pos.y -= m_lineHeight;
            i += line-1;
            continue;
        }

        if ( ((m_bEdit && m_bFocus && m_bHilite) ||
              (!m_bEdit && m_bHilite)            ) &&
             c1 != c2 && beg <= c2 && beg+len >= c1 )  // selected area?
        {
            o1 = c1;  if ( o1 < beg     )  o1 = beg;
            o2 = c2;  if ( o2 > beg+len )  o2 = beg+len;

            if ( m_format == 0 )
            {
                start.x = ppos.x+m_engine->RetText()->RetStringWidth(m_text+beg, o1-beg, size, m_fontStretch, m_fontType);
                end.x   = m_engine->RetText()->RetStringWidth(m_text+o1, o2-o1, size, m_fontStretch, m_fontType);
            }
            else
            {
                start.x = ppos.x+m_engine->RetText()->RetStringWidth(m_text+beg, m_format+beg, o1-beg, size, m_fontStretch);
                end.x   = m_engine->RetText()->RetStringWidth(m_text+o1, m_format+o1, o2-o1, size, m_fontStretch);
            }

            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            if ( m_format != 0 && (m_format[beg]&TITLE_MASK) == TITLE_BIG )  end.y *= BIG_FONT;
            DrawPart(start, end, 1);  // plain yellow background
        }

        eol = 16;  // >
        if ( len > 0 && m_text[beg+len-1] == '\n' )
        {
            len --;  // does not display the '\ n'
            eol = 0;  // nothing
        }
        if ( beg+len >= m_len )
        {
            eol = 2;  // square (eot)
        }
        if ( !m_bMulti || !m_bDisplaySpec )  eol = 0;
        if ( m_format == 0 )
        {
            m_engine->RetText()->DrawText(m_text+beg, len, ppos, m_dim.x, 1, size, m_fontStretch, m_fontType, eol);
        }
        else
        {
            m_engine->RetText()->DrawText(m_text+beg, m_format+beg, len, ppos, m_dim.x, 1, size, m_fontStretch, eol);
        }

        pos.y -= m_lineHeight;

        if ( i < m_lineTotal-2 && m_lineOffset[i+1] == m_lineOffset[i+2] )
        {
            pos.y -= m_lineHeight;  // double jump line \b;
            i ++;
        }
    }

    // Shows the cursor.
    if ( (m_bEdit && m_bFocus && m_bHilite && Mod(m_timeBlink, 1.0f) <= 0.5f) )  // it blinks
    {
        pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1*2.0f);
        for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
        {
            if ( i == m_lineTotal-1 || m_cursor1 < m_lineOffset[i+1] )
            {
                pos.x = m_pos.x+(10.0f/640.0f);
                if ( m_bAutoIndent )
                {
                    pos.x += indentLength*m_lineIndent[i];
                }

                len = m_cursor1 - m_lineOffset[i];

                if ( m_format == 0 )
                {
                    m_engine->RetText()->DimText(m_text+m_lineOffset[i], len,
                                                 pos, 1, size,
                                                 m_fontStretch, m_fontType,
                                                 start, end);
                }
                else
                {
                    m_engine->RetText()->DimText(m_text+m_lineOffset[i],
                                                 m_format+m_lineOffset[i],
                                                 len, pos, 1, size,
                                                 m_fontStretch,
                                                 start, end);
                }

                pos.x = end.x;
                break;
            }
            pos.y -= m_lineHeight;
        }
        pos.x -= 1.0f/640.0f;
        dim.x = 2.0f/640.0f;
        dim.y = m_lineHeight;
        DrawPart(pos, dim, 0);  // red
    }

    if ( m_scroll != 0 && !m_bGeneric )
    {
        m_scroll->Draw();
    }
}

// Draw an image part.

void CEdit::DrawImage(FPOINT pos, char *name, float width,
                      float offset, float height, int nbLine)
{
    FPOINT      uv1, uv2, dim;
    float       dp;
    char        filename[100];

//? sprintf(filename, "diagram\\%s.bmp", name);
    UserDir(filename, name, "diagram");
    strcat(filename, ".bmp");

    m_engine->SetTexture(filename);
    m_engine->SetState(D3DSTATENORMAL);

    uv1.x = 0.0f;
    uv2.x = 1.0f;
    uv1.y = offset;
    uv2.y = offset+height;

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    dim.x = width;
    dim.y = m_lineHeight*nbLine;
    DrawIcon(pos, dim, uv1, uv2);
}

// Draw the background.

void CEdit::DrawBack(FPOINT pos, FPOINT dim)
{
    FPOINT      uv1,uv2, corner;
    float       dp;

    if ( m_bGeneric )  return;

    m_engine->SetTexture("button2.tga");
    m_engine->SetState(D3DSTATENORMAL);

    if ( m_bMulti )
    {
        uv1.x = 128.0f/256.0f;  // light blue
        uv1.y =  64.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y =  96.0f/256.0f;
    }
    else
    {
        uv1.x = 160.0f/256.0f;  // medium blue
        uv1.y = 192.0f/256.0f;
        uv2.x = 192.0f/256.0f;
        uv2.y = 224.0f/256.0f;
    }
    if ( m_icon == 1 )
    {
        uv1.x = 192.0f/256.0f;  // orange
        uv1.y =  96.0f/256.0f;
        uv2.x = 224.0f/256.0f;
        uv2.y = 128.0f/256.0f;
    }

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    if ( m_bMulti )
    {
        corner.x = 10.0f/640.0f;
        corner.y = 10.0f/480.0f;
        DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    else
    {
        DrawIcon(pos, dim, uv1, uv2, 8.0f/256.0f);
    }
}

// Draws an icon background.

void CEdit::DrawPart(FPOINT pos, FPOINT dim, int icon)
{
    FPOINT      uv1, uv2;
    float       dp;

#if _POLISH
    m_engine->SetTexture("textp.tga");
#else
    m_engine->SetTexture("text.tga");
#endif
    m_engine->SetState(D3DSTATENORMAL);

    uv1.x = (16.0f/256.0f)*(icon%16);
    uv1.y = (240.0f/256.0f);
    uv2.x = (16.0f/256.0f)+uv1.x;
    uv2.y = (16.0f/256.0f)+uv1.y;

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    DrawIcon(pos, dim, uv1, uv2);
}


// Give the text to edit.

void CEdit::SetText(char *text, BOOL bNew)
{
    int     i, j, font;
    BOOL    bBOL;

    if ( !bNew )  UndoMemorize(OPERUNDO_SPEC);

    m_len = strlen(text);
    if ( m_len > m_maxChar )  m_len = m_maxChar;

    if ( m_format == 0 )
    {
        if ( m_bAutoIndent )
        {
            j = 0;
            bBOL = TRUE;
            for ( i=0 ; i<m_len ; i++ )
            {
                if ( text[i] == '\t' )
                {
                    if ( !bBOL )  m_text[j++] = ' ';
                    continue;  // removes tabs
                }
                bBOL = ( text[i] == '\n' );

                m_text[j++] = text[i];
            }
            m_len = j;
        }
        else
        {
            strncpy(m_text, text, m_len);
        }
    }
    else
    {
        font = m_fontType;
        j = 0;
        bBOL = TRUE;
        for ( i=0 ; i<m_len ; i++ )
        {
            if ( m_bAutoIndent )
            {
                if ( text[i] == '\t' )
                {
                    if ( !bBOL )
                    {
                        m_text[j] = ' ';
                        m_format[j] = font;
                        j ++;
                    }
                    continue;  // removes tabs
                }
                bBOL = ( text[i] == '\n' );
            }

            if ( text[i] == '\\' && text[i+2] == ';' )
            {
                if ( text[i+1] == 'n' )  // normal ?
                {
                    font &= ~FONT_MASK;
                    font |= FONT_COLOBOT;
                    i += 2;
                }
                else if ( text[i+1] == 'c' )  // cbot ?
                {
                    font &= ~FONT_MASK;
                    font |= FONT_COURIER;
                    i += 2;
                }
                else if ( text[i+1] == 'b' )  // big title ?
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_BIG;
                    i += 2;
                }
                else if ( text[i+1] == 't' )  // title ?
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_NORM;
                    i += 2;
                }
                else if ( text[i+1] == 's' )  // subtitle ?
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_LITTLE;
                    i += 2;
                }
            }
            else
            {
                m_text[j] = text[i];
                m_format[j] = font;
                j ++;

                font &= ~TITLE_MASK;  // reset title
            }
        }
        m_len = j;
    }

    if ( bNew )  UndoFlush();

    m_cursor1 = 0;
    m_cursor2 = 0;  // cursor to the beginning
    Justif();
    ColumnFix();
}

// Returns a pointer to the edited text.

char* CEdit::RetText()
{
    m_text[m_len] = 0;
    return m_text;
}

// Returns the edited text.

void CEdit::GetText(char *buffer, int max)
{
    if ( m_len < max )  max = m_len;
    if ( m_len > max )  max = max-1;

    strncpy(buffer, m_text, max);
    buffer[max] = 0;
}

// Returns the length of the text.

int CEdit::RetTextLength()
{
    return m_len;
}



// Returns a name in a command.
// \x nom1 nom2 nom3;

void GetNameParam(char *cmd, int rank, char *buffer)
{
    int     i;

    for ( i=0 ; i<rank ; i++ )
    {
        while ( *cmd != ' ' && *cmd != ';' )
        {
            cmd ++;
        }
        if ( *cmd != ';' )  cmd ++;
    }

    while ( *cmd != ' ' && *cmd != ';' )
    {
        *buffer++ = *cmd++;
    }
    *buffer = 0;
}

// Returns a number of a command.
// \x nom n1 n2;

int RetValueParam(char *cmd, int rank)
{
    int     n, i;

    for ( i=0 ; i<rank ; i++ )
    {
        while ( *cmd != ' ' && *cmd != ';' )
        {
            cmd ++;
        }
        if ( *cmd != ';' )  cmd ++;
    }

    sscanf(cmd, "%d", &n);
    return n;
}

// Frees all images.

void CEdit::FreeImage()
{
    char    filename[100];
    int     i;

    for ( i=0 ; i<m_imageTotal ; i++ )
    {
//?     sprintf(filename, "diagram\\%s.bmp", m_image[i].name);
        UserDir(filename, m_image[i].name, "diagram");
        strcat(filename, ".bmp");
        m_engine->FreeTexture(filename);
    }
}

// Reads the texture of an image.

void CEdit::LoadImage(char *name)
{
    char    filename[100];

//? sprintf(filename, "diagram\\%s.bmp", name);
    UserDir(filename, name, "diagram");
    strcat(filename, ".bmp");
    m_engine->LoadTexture(filename);
}

// Read from a text file.

BOOL CEdit::ReadText(char *filename, int addSize)
{
    FILE        *file = NULL;
    char        *buffer;
    int         len, i, j, n, font, iIndex, iLines, iCount, iLink, res;
    char        iName[50];
    char        text[50];
    float       iWidth;
    KeyRank     key;
    BOOL        bInSoluce, bBOL;

    if ( filename[0] == 0 )  return FALSE;
    file = fopen(filename, "rb");
    if ( file == NULL )  return FALSE;

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    m_maxChar = len+addSize+100;
    m_len = len;
    m_cursor1 = 0;
    m_cursor2 = 0;

    FreeImage();
    delete m_text;
    m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));
    buffer = (char*)malloc(sizeof(char)*(m_maxChar+1));
    fread(buffer, 1, len, file);

    if ( m_format != 0 )
    {
        delete m_format;
        m_format = (char*)malloc(sizeof(char)*m_maxChar);
    }

    fclose(file);

    bInSoluce = FALSE;
    font = m_fontType;
    iIndex = 0;
    iLink = 0;
    m_imageTotal = 0;
    m_markerTotal = 0;
    i = j = 0;
    bBOL = TRUE;
    while ( i < m_len )
    {
        if ( m_bAutoIndent )
        {
            if ( buffer[i] == '\t' )
            {
                if ( !bBOL )
                {
                    m_text[j] = buffer[i];
                    if ( m_format != 0 )  m_format[j] = font;
                    j ++;
                }
                i ++;
                continue;  // removes the tabs
            }
            bBOL = ( buffer[i] == '\n' || buffer[i] == '\r' );
        }

        if ( buffer[i] == '\r' )  // removes \ r
        {
            i ++;
        }
        else if ( m_format != 0 && buffer[i] == '\\' && buffer[i+2] == ';' )
        {
            if ( buffer[i+1] == 'n' )  // normal ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~FONT_MASK;
                    font |= FONT_COLOBOT;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'c' )  // cbot ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~FONT_MASK;
                    font |= FONT_COURIER;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'b' )  // big title ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_BIG;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 't' )  // title ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_NORM;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 's' )  // subtitle ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~TITLE_MASK;
                    font |= TITLE_LITTLE;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'l' )  // link ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~COLOR_MASK;
                    font |= COLOR_LINK;
                }
                i += 3;
            }
            else
            {
                i += 3;
            }
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \u marker name; ?
                  buffer[i+1] == 'u'  &&
                  buffer[i+2] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( iLink < EDITLINKMAX )
                {
                    GetNameParam(buffer+i+3, 0, m_link[iLink].name);
                    GetNameParam(buffer+i+3, 1, m_link[iLink].marker);
                    iLink ++;
                }
                font &= ~COLOR_MASK;
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \m marker; ?
                  buffer[i+1] == 'm'  &&
                  buffer[i+2] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( m_markerTotal < EDITLINKMAX )
                {
                    GetNameParam(buffer+i+3, 0, m_marker[m_markerTotal].name);
                    m_marker[m_markerTotal].pos = j;
                    m_markerTotal ++;
                }
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \image name lx ly; ?
                  buffer[i+1] == 'i'  &&
                  buffer[i+2] == 'm'  &&
                  buffer[i+3] == 'a'  &&
                  buffer[i+4] == 'g'  &&
                  buffer[i+5] == 'e'  &&
                  buffer[i+6] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
#if _DEMO
                strcpy(iName, "demo");
#else
                GetNameParam(buffer+i+7, 0, iName);
#endif
//?             iWidth = m_lineHeight*RetValueParam(buffer+i+7, 1);
                iWidth = (float)RetValueParam(buffer+i+7, 1);
                iWidth *= m_engine->RetText()->RetHeight(SMALLFONT, FONT_COLOBOT);
                iLines = RetValueParam(buffer+i+7, 2);
                LoadImage(iName);

                // A part of image per line of text.
                for ( iCount=0 ; iCount<iLines ; iCount++ )
                {
                    strcpy(m_image[iIndex].name, iName);
                    m_image[iIndex].offset = (float)iCount/iLines;
                    m_image[iIndex].height = 1.0f/iLines;
                    m_image[iIndex].width = iWidth*0.75f;

                    m_text[j] = (char)(iIndex++);  // as an index into m_image
                    m_format[j] = (unsigned char)IMAGE_MASK;
                    j ++;
                }
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \button; ?
                  buffer[i+1] == 'b'  &&
                  buffer[i+2] == 'u'  &&
                  buffer[i+3] == 't'  &&
                  buffer[i+4] == 't'  &&
                  buffer[i+5] == 'o'  &&
                  buffer[i+6] == 'n'  &&
                  buffer[i+7] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                m_text[j] = RetValueParam(buffer+i+8, 0);
                m_format[j] = font|FONT_BUTTON;
                j ++;
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \token; ?
                  buffer[i+1] == 't'  &&
                  buffer[i+2] == 'o'  &&
                  buffer[i+3] == 'k'  &&
                  buffer[i+4] == 'e'  &&
                  buffer[i+5] == 'n'  &&
                  buffer[i+6] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~COLOR_MASK;
                font |= COLOR_TOKEN;
            }
            i += 7;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \type; ?
                  buffer[i+1] == 't'  &&
                  buffer[i+2] == 'y'  &&
                  buffer[i+3] == 'p'  &&
                  buffer[i+4] == 'e'  &&
                  buffer[i+5] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~COLOR_MASK;
                font |= COLOR_TYPE;
            }
            i += 6;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \const; ?
                  buffer[i+1] == 'c'  &&
                  buffer[i+2] == 'o'  &&
                  buffer[i+3] == 'n'  &&
                  buffer[i+4] == 's'  &&
                  buffer[i+5] == 't'  &&
                  buffer[i+6] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~COLOR_MASK;
                font |= COLOR_CONST;
            }
            i += 7;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \key; ?
                  buffer[i+1] == 'k'  &&
                  buffer[i+2] == 'e'  &&
                  buffer[i+3] == 'y'  &&
                  buffer[i+4] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~COLOR_MASK;
                font |= COLOR_KEY;
            }
            i += 5;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \tab; ?
                  buffer[i+1] == 't'  &&
                  buffer[i+2] == 'a'  &&
                  buffer[i+3] == 'b'  &&
                  buffer[i+4] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font |= COLOR_TABLE;
            }
            i += 5;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \norm; ?
                  buffer[i+1] == 'n'  &&
                  buffer[i+2] == 'o'  &&
                  buffer[i+3] == 'r'  &&
                  buffer[i+4] == 'm'  &&
                  buffer[i+5] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~COLOR_MASK;
            }
            i += 6;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \begin soluce; ?
                  buffer[i+1] == 'b'  &&
                  buffer[i+2] == 's'  &&
                  buffer[i+3] == ';'  )
        {
            bInSoluce = TRUE;
            i += 4;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \end soluce; ?
                  buffer[i+1] == 'e'  &&
                  buffer[i+2] == 's'  &&
                  buffer[i+3] == ';'  )
        {
            bInSoluce = FALSE;
            i += 4;
        }
        else if ( m_format != 0       &&
                  buffer[i+0] == '\\' &&  // \key name; ?
                  buffer[i+1] == 'k'  &&
                  buffer[i+2] == 'e'  &&
                  buffer[i+3] == 'y'  &&
                  buffer[i+4] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( SearchKey(buffer+i+5, key) )
                {
                    res = m_engine->RetKey(key, 0);
                    if ( res != 0 )
                    {
                        if ( GetResource(RES_KEY, res, iName) )
                        {
                            m_text[j] = ' ';
                            m_format[j] = font;
                            j ++;
                            n = 0;
                            while ( iName[n] != 0 )
                            {
                                m_text[j] = iName[n++];
                                m_format[j] = font;
                                j ++;
                            }
                            m_text[j] = ' ';
                            m_format[j] = font;
                            j ++;

                            res = m_engine->RetKey(key, 1);
                            if ( res != 0 )
                            {
                                if ( GetResource(RES_KEY, res, iName) )
                                {
                                    GetResource(RES_TEXT, RT_KEY_OR, text);
                                    n = 0;
                                    while ( text[n] != 0 )
                                    {
                                        m_text[j] = text[n++];
                                        m_format[j] = font&~COLOR_MASK;
                                        j ++;
                                    }
                                    n = 0;
                                    while ( iName[n] != 0 )
                                    {
                                        m_text[j] = iName[n++];
                                        m_format[j] = font;
                                        j ++;
                                    }
                                    m_text[j] = ' ';
                                    m_format[j] = font;
                                    j ++;
                                }
                            }
                            while ( buffer[i++] != ';' );
                            continue;
                        }
                    }
                }
                m_text[j] = '?';
                m_format[j] = font;
                j ++;
            }
            while ( buffer[i++] != ';' );
        }
        else
        {
            if ( m_bSoluce || !bInSoluce )
            {
                m_text[j] = buffer[i];
                if ( m_format != 0 )  m_format[j] = font;
                j ++;
            }
            i ++;

            font &= ~TITLE_MASK;  // reset title

            if ( (font&COLOR_MASK) == COLOR_TABLE )
            {
                font &= ~COLOR_TABLE;
            }
        }
    }
    m_len = j;
    m_imageTotal = iIndex;

    delete buffer;

    Justif();
    ColumnFix();
    return TRUE;
}

// Writes all the text in a file.

BOOL CEdit::WriteText(char *filename)
{
    FILE*       file;
    char        buffer[1000+20];
    int         i, j, k, n;
    float       iDim;

    if ( filename[0] == 0 )  return FALSE;
    file = fopen(filename, "wb");
    if ( file == NULL )  return FALSE;

    if ( m_bAutoIndent )
    {
        iDim = m_dim.x;
        m_dim.x = 1000.0f;  // puts an infinite width!
        Justif();
    }

    i = j = k = 0;
    while ( m_text[i] != 0 && i < m_len )
    {
        if ( m_bAutoIndent && i == m_lineOffset[k] )
        {
            for ( n=0 ; n<m_lineIndent[k] ; n++ )
            {
                buffer[j++] = '\t';
            }
            k ++;
        }

        buffer[j++] = m_text[i];

        if ( m_text[i] == '\n' )
        {
            buffer[j-1] = '\r';
            buffer[j++] = '\n';  // \r\n (0x0D, 0x0A)
        }

        if ( j >= 1000-1 )
        {
            fwrite(buffer, 1, j, file);
            j = 0;
        }

        i ++;
    }
    if ( j > 0 )
    {
        fwrite(buffer, 1, j, file);
    }

    fclose(file);

    if ( m_bAutoIndent )
    {
        m_dim.x = iDim;  // presents the initial width
        Justif();
    }

    return TRUE;
}


// Manage the number of max characters editable.

void CEdit::SetMaxChar(int max)
{
    m_maxChar = max;
    FreeImage();
    delete m_text;
    m_text = (char*)malloc(sizeof(char)*(m_maxChar+1));

    if ( m_format != 0 )
    {
        delete m_format;
        m_format = (char*)malloc(sizeof(char)*m_maxChar);
    }

    m_len = 0;
    m_cursor1 = 0;
    m_cursor2 = 0;
    Justif();
    UndoFlush();
}

int CEdit::RetMaxChar()
{
    return m_maxChar;
}


// Mode management "editable".

void CEdit::SetEditCap(BOOL bMode)
{
    m_bEdit = bMode;
}

BOOL CEdit::RetEditCap()
{
    return m_bEdit;
}

// Mode management "hilitable" (that's the franch).

void CEdit::SetHiliteCap(BOOL bEnable)
{
    m_bHilite = bEnable;
}

BOOL CEdit::RetHiliteCap()
{
    return m_bHilite;
}

// Lift in / out connection.

void CEdit::SetInsideScroll(BOOL bInside)
{
    m_bInsideScroll = bInside;
}

BOOL CEdit::RetInsideScroll()
{
    return m_bInsideScroll;
}

// Specifies whether to display the links showing the solution.

void CEdit::SetSoluceMode(BOOL bSoluce)
{
    m_bSoluce = bSoluce;
}

BOOL CEdit::RetSoluceMode()
{
    return m_bSoluce;
}

// Indicates whether the text is a defile that generic.

void CEdit::SetGenericMode(BOOL bGeneric)
{
    m_bGeneric = bGeneric;
}

BOOL CEdit::RetGenericMode()
{
    return m_bGeneric;
}


// Management of automatic indentation mode with {}.

void CEdit::SetAutoIndent(BOOL bMode)
{
    m_bAutoIndent = bMode;
}

BOOL CEdit::RetAutoIndent()
{
    return m_bAutoIndent;
}



// Moves the cursors.

void CEdit::SetCursor(int cursor1, int cursor2)
{
    if ( cursor1 > m_len )  cursor1 = m_len;
    if ( cursor2 > m_len )  cursor2 = m_len;

    m_cursor1 = cursor1;
    m_cursor2 = cursor2;
    m_bUndoForce = TRUE;
    ColumnFix();
}

// Returns the sliders.

void CEdit::GetCursor(int &cursor1, int &cursor2)
{
    cursor1 = m_cursor1;
    cursor2 = m_cursor2;
}


// Displayed line modifies the first.

void CEdit::SetFirstLine(int rank)
{
    Scroll(rank, TRUE);
}

// Returns the first displayed line.

int CEdit::RetFirstLine()
{
    if ( m_historyTotal > 0 )
    {
        if ( m_historyCurrent == 0 )
        {
            return m_lineFirst;
        }
        else
        {
            return m_history[0].firstLine;
        }
    }
    return m_lineFirst;
}


// Shows the selected area.

void CEdit::ShowSelect()
{
    int     cursor1, cursor2, line;

    if ( m_cursor1 < m_cursor2 )
    {
        cursor1 = m_cursor1;
        cursor2 = m_cursor2;
    }
    else
    {
        cursor1 = m_cursor2;
        cursor2 = m_cursor1;
    }

    line = RetCursorLine(cursor2);
    if ( line >= m_lineFirst+m_lineVisible )
    {
        line -= m_lineVisible-1;
        if ( line < 0 )  line = 0;
        Scroll(line, FALSE);
    }

    line = RetCursorLine(cursor1);
    if ( line < m_lineFirst )
    {
        Scroll(line, FALSE);
    }
}


// Management of the display mode of special characters.

void CEdit::SetDisplaySpec(BOOL bDisplay)
{
    m_bDisplaySpec = bDisplay;
}

BOOL CEdit::RetDisplaySpec()
{
    return m_bDisplaySpec;
}


// Multi-fonts mode management.

void CEdit::SetMultiFont(BOOL bMulti)
{
    if ( bMulti )
    {
        delete m_format;
        m_format = (char*)malloc(sizeof(char)*m_maxChar);
        memset(m_format, 0, m_maxChar);
    }
    else
    {
        delete m_format;
        m_format = 0;
    }
}

BOOL CEdit::RetMultiFont()
{
    return ( m_format != 0 );
}


// Management of the character size.

void CEdit::SetFontSize(float size)
{
    CControl::SetFontSize(size);

    MoveAdjust();
}


// Moves according to the visible lift.

void CEdit::Scroll()
{
    float   value;

    if ( m_scroll != 0 )
    {
        value = m_scroll->RetVisibleValue();
        value *= m_lineTotal-m_lineVisible;
        Scroll((int)(value+0.5f), TRUE);
    }
}

// Moves according to the visible lift.

void CEdit::Scroll(int pos, BOOL bAdjustCursor)
{
    int     max, line;

    m_lineFirst = pos;

    if ( m_lineFirst < 0 )  m_lineFirst = 0;

    max = m_lineTotal-m_lineVisible;
    if ( max < 0 )  max = 0;
    if ( m_lineFirst > max )  m_lineFirst = max;

    line = RetCursorLine(m_cursor1);

    if ( bAdjustCursor && m_bEdit )
    {
        // Cursor too high?
        if ( line < m_lineFirst )
        {
            MoveLine(m_lineFirst-line, FALSE, FALSE);
            return;
        }

        // Cursor too low?
        if ( line >= m_lineFirst+m_lineVisible )
        {
            MoveLine(m_lineFirst+m_lineVisible-line-1, FALSE, FALSE);
            return;
        }
    }

    Justif();
}

// Moves the cursor to the beginning of the line.

void CEdit::MoveHome(BOOL bWord, BOOL bSelect)
{
    int     begin, tab;

    if ( bWord )
    {
        m_cursor1 = 0;
    }
    else
    {
        begin = m_cursor1;
        while ( begin > 0 && m_text[begin-1] != '\n' )
        {
            begin --;
        }

        tab = begin;
        while ( tab < m_len && (m_text[tab] == '\t' || m_text[tab] == ' ') )
        {
            tab ++;
        }

        if ( m_cursor1 == tab )
        {
            m_cursor1 = begin;
        }
        else
        {
            m_cursor1 = tab;
        }
    }
    if ( !bSelect )  m_cursor2 = m_cursor1;

    m_bUndoForce = TRUE;
    Justif();
    ColumnFix();
}

// Moves the cursor to the end of the line.

void CEdit::MoveEnd(BOOL bWord, BOOL bSelect)
{
    if ( bWord )
    {
        m_cursor1 = m_len;
    }
    else
    {
        while ( m_cursor1 < m_len && m_text[m_cursor1] != '\n' )
        {
            m_cursor1 ++;
        }
    }
    if ( !bSelect )  m_cursor2 = m_cursor1;

    m_bUndoForce = TRUE;
    Justif();
    ColumnFix();
}

// Moves the cursor through characters.

void CEdit::MoveChar(int move, BOOL bWord, BOOL bSelect)
{
    int     character;

    if ( move == -1 )  // back?
    {
        if ( bWord )
        {
            while ( m_cursor1 > 0 )
            {
                character = (unsigned char)m_text[m_cursor1-1];
                if ( !IsSpace(character) )  break;
                m_cursor1 --;
            }

            if ( m_cursor1 > 0 )
            {
                character = (unsigned char)m_text[m_cursor1-1];
                if ( IsSpace(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = (unsigned char)m_text[m_cursor1-1];
                        if ( !IsSpace(character) )  break;
                        m_cursor1 --;
                    }
                }
                else if ( IsWord(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = (unsigned char)m_text[m_cursor1-1];
                        if ( !IsWord(character) )  break;
                        m_cursor1 --;
                    }
                }
                else if ( IsSep(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = (unsigned char)m_text[m_cursor1-1];
                        if ( !IsSep(character) )  break;
                        m_cursor1 --;
                    }
                }
            }
        }
        else
        {
            m_cursor1 --;
            if ( m_cursor1 < 0 )  m_cursor1 = 0;
        }
    }

    if ( move == 1 )  // advance?
    {
        if ( bWord )
        {
            if ( m_cursor1 < m_len )
            {
                character = (unsigned char)m_text[m_cursor1];
                if ( IsSpace(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = (unsigned char)m_text[m_cursor1];
                        if ( !IsSpace(character) )  break;
                        m_cursor1 ++;
                    }
                }
                else if ( IsWord(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = (unsigned char)m_text[m_cursor1];
                        if ( !IsWord(character) )  break;
                        m_cursor1 ++;
                    }
                }
                else if ( IsSep(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = (unsigned char)m_text[m_cursor1];
                        if ( !IsSep(character) )  break;
                        m_cursor1 ++;
                    }
                }
            }

            while ( m_cursor1 < m_len )
            {
                character = (unsigned char)m_text[m_cursor1];
                if ( !IsSpace(character) )  break;
                m_cursor1 ++;
            }
        }
        else
        {
            m_cursor1 ++;
            if ( m_cursor1 > m_len )  m_cursor1 = m_len;
        }
    }

    if ( !bSelect )  m_cursor2 = m_cursor1;

    m_bUndoForce = TRUE;
    Justif();
    ColumnFix();
}

// Moves the cursor lines.

void CEdit::MoveLine(int move, BOOL bWord, BOOL bSelect)
{
    float   column, indentLength;
    int     i, line, c;

    if ( move == 0 )  return;

    for ( i=0 ; i>move ; i-- )  // back?
    {
        while ( m_cursor1 > 0 && m_text[m_cursor1-1] != '\n' )
        {
            m_cursor1 --;
        }
        if ( m_cursor1 != 0 )
        {
            m_cursor1 --;
            while ( m_cursor1 > 0 )
            {
                if ( m_text[--m_cursor1] == '\n' )
                {
                    m_cursor1 ++;
                    break;
                }
            }
        }
    }

    for ( i=0 ; i<move ; i++ )  // advance?
    {
        while ( m_cursor1 < m_len )
        {
            if ( m_text[m_cursor1++] == '\n' )
            {
                break;
            }
        }
    }

    line = RetCursorLine(m_cursor1);

    column = m_column;
    if ( m_bAutoIndent )
    {
        indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
                        * m_engine->RetEditIndentValue();
        column -= indentLength*m_lineIndent[line];
    }

    if ( m_format == 0 )
    {
        c = m_engine->RetText()->Detect(m_text+m_lineOffset[line],
                                        m_lineOffset[line+1]-m_lineOffset[line],
                                        column, m_fontSize,
                                        m_fontStretch, m_fontType);
    }
    else
    {
        c = m_engine->RetText()->Detect(m_text+m_lineOffset[line],
                                        m_format+m_lineOffset[line],
                                        m_lineOffset[line+1]-m_lineOffset[line],
                                        column, m_fontSize, m_fontStretch);
    }

    m_cursor1 = m_lineOffset[line]+c;
    if ( !bSelect )  m_cursor2 = m_cursor1;

    m_bUndoForce = TRUE;
    Justif();
}

// Sets the horizontal position.

void CEdit::ColumnFix()
{
    float   indentLength;
    int     line;

    line = RetCursorLine(m_cursor1);

    if ( m_format == 0 )
    {
        m_column = m_engine->RetText()->RetStringWidth
                            (
                                m_text+m_lineOffset[line],
                                m_cursor1-m_lineOffset[line],
                                m_fontSize, m_fontStretch, m_fontType
                            );
    }
    else
    {
        m_column = m_engine->RetText()->RetStringWidth
                            (
                                m_text+m_lineOffset[line],
                                m_format+m_lineOffset[line],
                                m_cursor1-m_lineOffset[line],
                                m_fontSize, m_fontStretch
                            );
    }

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
                        * m_engine->RetEditIndentValue();
        m_column += indentLength*m_lineIndent[line];
    }
}


// Cut the selected characters or entire line.

BOOL CEdit::Cut()
{
    HGLOBAL hg;
    char*   text;
    char    c;
    int     c1, c2, start, len, i, j;

    if ( !m_bEdit )  return FALSE;

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Swap(c1, c2);  // always c1 <= c2

    if ( c1 == c2 )
    {
        while ( c1 > 0 )
        {
            if ( m_text[c1-1] == '\n' )  break;
            c1 --;
        }
        while ( c2 < m_len )
        {
            c2 ++;
            if ( m_text[c2-1] == '\n' )  break;
        }
    }

    if ( c1 == c2 )  return FALSE;

    start = c1;
    len   = c2-c1;

    if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
    {
        return FALSE;
    }
    if ( !(text = (char*)GlobalLock(hg)) )
    {
        GlobalFree(hg);
        return FALSE;
    }

    j = 0;
    for ( i=start ; i<start+len ; i++ )
    {
        c = m_text[i];
        if ( c == '\n' )  text[j++] = '\r';
        text[j++] = c;
    }
    text[j] = 0;
    GlobalUnlock(hg);

    if ( !OpenClipboard(NULL) )
    {
        GlobalFree(hg);
        return FALSE;
    }
    if ( !EmptyClipboard() )
    {
        GlobalFree(hg);
        return FALSE;
    }
    if ( !SetClipboardData(CF_TEXT, hg) )
    {
        GlobalFree(hg);
        return FALSE;
    }
    CloseClipboard();

    UndoMemorize(OPERUNDO_SPEC);
    m_cursor1 = c1;
    m_cursor2 = c2;
    DeleteOne(0);  // deletes the selected characters
    Justif();
    ColumnFix();
    SendModifEvent();
    return TRUE;
}

// Copy the selected characters or entire line.

BOOL CEdit::Copy()
{
    HGLOBAL hg;
    char*   text;
    char    c;
    int     c1, c2, start, len, i, j;

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Swap(c1, c2);  // always c1 <= c2

    if ( c1 == c2 )
    {
        while ( c1 > 0 )
        {
            if ( m_text[c1-1] == '\n' )  break;
            c1 --;
        }
        while ( c2 < m_len )
        {
            c2 ++;
            if ( m_text[c2-1] == '\n' )  break;
        }
    }

    if ( c1 == c2 )  return FALSE;

    start = c1;
    len   = c2-c1;

    if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
    {
        return FALSE;
    }
    if ( !(text = (char*)GlobalLock(hg)) )
    {
        GlobalFree(hg);
        return FALSE;
    }

    j = 0;
    for ( i=start ; i<start+len ; i++ )
    {
        c = m_text[i];
        if ( c == '\n' )  text[j++] = '\r';
        text[j++] = c;
    }
    text[j] = 0;
    GlobalUnlock(hg);

    if ( !OpenClipboard(NULL) )
    {
        GlobalFree(hg);
        return FALSE;
    }
    if ( !EmptyClipboard() )
    {
        GlobalFree(hg);
        return FALSE;
    }
    if ( !SetClipboardData(CF_TEXT, hg) )
    {
        GlobalFree(hg);
        return FALSE;
    }
    CloseClipboard();

    return TRUE;
}

// Paste the contents of the notebook.

BOOL CEdit::Paste()
{
    HANDLE  h;
    char    c;
    char*   p;

    if ( !m_bEdit )  return FALSE;

    if ( !OpenClipboard(NULL) )
    {
        return FALSE;
    }

    if ( !(h = GetClipboardData(CF_TEXT)) )
    {
        CloseClipboard();
        return FALSE;
    }

    if ( !(p = (char*)GlobalLock(h)) )
    {
        CloseClipboard();
        return FALSE;
    }

    UndoMemorize(OPERUNDO_SPEC);

    while ( *p != 0 )
    {
        c = *p++;
        if ( c == '\r' )  continue;
        if ( c == '\t' && m_bAutoIndent )  continue;
        InsertOne(c);
    }

    GlobalUnlock(h);
    CloseClipboard();

    Justif();
    ColumnFix();
    SendModifEvent();
    return TRUE;
}


// Cancels the last action.

BOOL CEdit::Undo()
{
    if ( !m_bEdit )  return FALSE;

    return UndoRecall();
}


// Inserts a character.

void CEdit::Insert(char character)
{
    int     i, level, tab;

    if ( !m_bEdit )  return;

    if ( !m_bMulti )  // single-line?
    {
        if ( character == '\n' ||
             character == '\t' )  return;
    }

    UndoMemorize(OPERUNDO_INSERT);

    if ( m_bMulti && !m_bAutoIndent )
    {
        if ( character == '\n' )
        {
            InsertOne(character);
            level = IndentCompute();
            for ( i=0 ; i<level ; i++ )
            {
                InsertOne('\t');
            }
        }
        else if ( character == '{' )
        {
            tab = IndentTabCount();
            if ( tab != -1 )
            {
                level = IndentCompute();
                IndentTabAdjust(level-tab);
            }
            InsertOne(character);
        }
        else if ( character == '}' )
        {
            tab = IndentTabCount();
            if ( tab != -1 )
            {
                level = IndentCompute()-1;
                IndentTabAdjust(level-tab);
            }
            InsertOne(character);
        }
        else
        {
            InsertOne(character);
        }
    }
    else if ( m_bAutoIndent )
    {
        if ( character == '{' )
        {
            InsertOne(character);
            InsertOne('\n');
            InsertOne('\n');
            InsertOne('}');
            MoveChar(-1, FALSE, FALSE);
            MoveChar(-1, FALSE, FALSE);
        }
#if 0
        else if ( character == '(' )
        {
            InsertOne(character);
            InsertOne(')');
            MoveChar(-1, FALSE, FALSE);
        }
        else if ( character == '[' )
        {
            InsertOne(character);
            InsertOne(']');
            MoveChar(-1, FALSE, FALSE);
        }
#endif
        else if ( character == '\t' )
        {
            for ( i=0 ; i<m_engine->RetEditIndentValue() ; i++ )
            {
                InsertOne(' ');
            }
        }
        else
        {
            InsertOne(character);
        }
    }
    else
    {
        InsertOne(character);
    }

    Justif();
    ColumnFix();
}

// Inserts a plain character.

void CEdit::InsertOne(char character)
{
    int     i;

    if ( !m_bEdit )  return;
    if ( !m_bMulti && character == '\n' )  return;

    if ( m_cursor1 != m_cursor2 )
    {
        DeleteOne(0);  // deletes the selected characters
    }

    if ( m_len >= m_maxChar )  return;

    for ( i=m_len ; i>=m_cursor1 ; i-- )
    {
        m_text[i] = m_text[i-1];  // shoot

        if ( m_format != 0 )
        {
            m_format[i] = m_format[i-1];  // shoot
        }
    }

    m_len ++;

    m_text[m_cursor1] = character;

    if ( m_format != 0 )
    {
        m_format[m_cursor1] = 0;
    }

    m_cursor1++;
    m_cursor2 = m_cursor1;
}

// Deletes the character left of cursor or all selected characters.

void CEdit::Delete(int dir)
{
    if ( !m_bEdit )  return;

    UndoMemorize(OPERUNDO_DELETE);
    DeleteOne(dir);

    Justif();
    ColumnFix();
}

// Deletes the character left of cursor or all selected plain characters.

void CEdit::DeleteOne(int dir)
{
    int     i, end, hole;

    if ( !m_bEdit )  return;

    if ( m_cursor1 == m_cursor2 )
    {
        if ( dir < 0 )
        {
            if ( m_cursor1 == 0 )  return;
            m_cursor1 --;
        }
        else
        {
            if ( m_cursor2 == m_len )  return;
            m_cursor2 ++;
        }
    }

    if ( m_cursor1 > m_cursor2 )  Swap(m_cursor1, m_cursor2);
    hole = m_cursor2-m_cursor1;
    end = m_len-hole;
    for ( i=m_cursor1 ; i<end ; i++ )
    {
        m_text[i] = m_text[i+hole];

        if ( m_format != 0 )
        {
            m_format[i] = m_format[i+hole];
        }
    }
    m_len -= hole;
    m_cursor2 = m_cursor1;
}


// Calculates the indentation level of brackets {and}.

int CEdit::IndentCompute()
{
    int     i, level;

    level = 0;
    for ( i=0 ; i<m_cursor1 ; i++ )
    {
        if ( m_text[i] == '{' )  level ++;
        if ( m_text[i] == '}' )  level --;
    }

    if ( level < 0 )  level = 0;
    return level;
}

// Counts the number of tabs before the cursor.
// Returns -1 if there is something else.

int CEdit::IndentTabCount()
{
    int     i, nb;

    if ( m_cursor1 != m_cursor2 )  return -1;

    i = m_cursor1;
    nb = 0;
    while ( i > 0 )
    {
        if ( m_text[i-1] == '\n' )  return nb;
        if ( m_text[i-1] != '\t' )  return -1;
        nb ++;
        i --;
    }
    return nb;
}

// Adds or removes qq tabs.

void CEdit::IndentTabAdjust(int number)
{
    int     i;

    for ( i=0 ; i<number ; i++ )  // add?
    {
        InsertOne('\t');
    }

    for ( i=0 ; i>number ; i-- )  // delete?
    {
        DeleteOne(-1);
    }
}


// Indent the left or right the entire selection.

BOOL CEdit::Shift(BOOL bLeft)
{
    BOOL    bInvert = FALSE;
    int     c1, c2, i;

    if ( m_cursor1 == m_cursor2 )  return FALSE;

    UndoMemorize(OPERUNDO_SPEC);

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )
    {
        Swap(c1, c2);  // always c1 <= c2
        bInvert = TRUE;
    }

    if ( c1 > 0 )
    {
        if ( m_text[c1-1] != '\n' )  return FALSE;
    }
    if ( c2 < m_len )
    {
        if ( m_text[c2-1] != '\n' )  return FALSE;
    }

    if ( bLeft )  // shifts left?
    {
        i = c1;
        while ( i < c2 )
        {
            if ( m_text[i] == '\t' )
            {
                m_cursor1 = i;
                m_cursor2 = i+1;
                DeleteOne(0);
                c2 --;
            }
            while ( i < c2 && m_text[i++] != '\n' );
        }
    }
    else    // shifts right?
    {
        i = c1;
        while ( i < c2 )
        {
            m_cursor1 = m_cursor2 = i;
            InsertOne('\t');
            c2 ++;
            while ( i < c2 && m_text[i++] != '\n' );
        }
    }

    if ( bInvert )  Swap(c1, c2);
    m_cursor1 = c1;
    m_cursor2 = c2;

    Justif();
    ColumnFix();
    SendModifEvent();
    return TRUE;
}

// Min conversion <-> shift the selection.

BOOL CEdit::MinMaj(BOOL bMaj)
{
    int     c1, c2, i, character;

    if ( m_cursor1 == m_cursor2 )  return FALSE;

    UndoMemorize(OPERUNDO_SPEC);

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Swap(c1, c2);  // alwyas c1 <= c2

    for ( i=c1 ; i<c2 ; i++ )
    {
        character = (unsigned char)m_text[i];
        if ( bMaj )  character = RetToUpper(character);
        else         character = RetToLower(character);
        m_text[i] = character;
    }

    Justif();
    ColumnFix();
    SendModifEvent();
    return TRUE;
}


// Cut all text lines.

void CEdit::Justif()
{
    float   width, value, size, indentLength;
    int     i, j, line, indent;
    BOOL    bDual, bString, bRem;

    indent = 0;
    m_lineTotal = 0;
    m_lineOffset[m_lineTotal] = 0;
    m_lineIndent[m_lineTotal] = indent;
    m_lineTotal ++;

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->RetText()->RetCharWidth(' ', 0.0f, m_fontSize, m_fontStretch, m_fontType)
                        * m_engine->RetEditIndentValue();
    }

    bString = bRem = FALSE;
    i = 0;
    while ( TRUE )
    {
        bDual = FALSE;

        width = m_dim.x-(10.0f/640.0f)*2.0f-(m_bMulti?MARGX*2.0f+SCROLL_WIDTH:0.0f);
        if ( m_bAutoIndent )
        {
            width -= indentLength*m_lineIndent[m_lineTotal-1];
        }

        if ( m_format == 0 )
        {
            i += m_engine->RetText()->Justif(m_text+i, m_len-i, width,
                                             m_fontSize, m_fontStretch,
                                             m_fontType);
        }
        else
        {
            size = m_fontSize;

            if ( (m_format[i]&TITLE_MASK) == TITLE_BIG )  // headline?
            {
                size *= BIG_FONT;
                bDual = TRUE;
            }

            if ( (m_format[i]&IMAGE_MASK) != 0 )  // image part?
            {
                i ++;  // jumps just a character (index in m_image)
            }
            else
            {
                i += m_engine->RetText()->Justif(m_text+i, m_format+i,
                                                 m_len-i, width,
                                                 size, m_fontStretch);
            }
        }

        if ( i >= m_len )  break;

        if ( m_bAutoIndent )
        {
            for ( j=m_lineOffset[m_lineTotal-1] ; j<i ; j++ )
            {
                if ( !bRem && m_text[j] == '\"' )  bString = !bString;
                if ( !bString &&
                     m_text[j] == '/' &&
                     m_text[j+1] == '/' )  bRem = TRUE;
                if ( m_text[j] == '\n' )  bString = bRem = FALSE;
                if ( m_text[j] == '{' && !bString && !bRem )  indent ++;
                if ( m_text[j] == '}' && !bString && !bRem )  indent --;
            }
            if ( indent < 0 )  indent = 0;
        }

        m_lineOffset[m_lineTotal] = i;
        m_lineIndent[m_lineTotal] = indent;
        m_lineTotal ++;
        if ( bDual )
        {
            m_lineOffset[m_lineTotal] = i;
            m_lineIndent[m_lineTotal] = indent;
            m_lineTotal ++;
        }
        if ( m_lineTotal >= EDITLINEMAX-2 )  break;
    }
    if ( m_len > 0 && m_text[m_len-1] == '\n' )
    {
        m_lineOffset[m_lineTotal] = m_len;
        m_lineIndent[m_lineTotal] = 0;
        m_lineTotal ++;
    }
    m_lineOffset[m_lineTotal] = m_len;
    m_lineIndent[m_lineTotal] = 0;

    if ( m_bAutoIndent )
    {
        for ( i=0 ; i<=m_lineTotal ; i++ )
        {
            if ( m_text[m_lineOffset[i]] == '}' )
            {
                if ( m_lineIndent[i] > 0 )  m_lineIndent[i] --;
            }
        }
    }

    if ( m_bMulti )
    {
        if ( m_bEdit )
        {
            line = RetCursorLine(m_cursor1);
            if ( line < m_lineFirst )
            {
                m_lineFirst = line;
            }
            if ( line >= m_lineFirst+m_lineVisible )
            {
                m_lineFirst = line-m_lineVisible+1;
            }
        }
    }
    else
    {
        m_lineFirst = 0;
    }

    if ( m_scroll != 0 )
    {
        if ( m_lineTotal <= m_lineVisible )
        {
            m_scroll->SetVisibleRatio(1.0f);
            m_scroll->SetVisibleValue(0.0f);
            m_scroll->SetArrowStep(0.0f);
        }
        else
        {
            value = (float)m_lineVisible/m_lineTotal;
            m_scroll->SetVisibleRatio(value);

            value = (float)m_lineFirst/(m_lineTotal-m_lineVisible);
            m_scroll->SetVisibleValue(value);

            value = (float)1.0f/(m_lineTotal-m_lineVisible);
            m_scroll->SetArrowStep(value);
        }
    }

    m_timeBlink = 0.0f;  // lights the cursor immediately
}

// Returns the rank of the line where the cursor is located.

int CEdit::RetCursorLine(int cursor)
{
    int     line, i;

    line = 0;
    for ( i=0 ; i<m_lineTotal ; i++ )
    {
        if ( cursor >= m_lineOffset[i] )
        {
            line = i;
        }
    }
    return line;
}


// Flush the buffer undo.

void CEdit::UndoFlush()
{
    int     i;

    for ( i=0 ; i<EDITUNDOMAX ; i++ )
    {
        delete m_undo[i].text;
        m_undo[i].text = 0;
    }

    m_bUndoForce = TRUE;
    m_undoOper = OPERUNDO_SPEC;
}

// Memorize the current state before a change.

void CEdit::UndoMemorize(OperUndo oper)
{
    int     i, len;

    if ( !m_bUndoForce               &&
         oper       != OPERUNDO_SPEC &&
         m_undoOper != OPERUNDO_SPEC &&
         oper == m_undoOper          )  return;

    m_bUndoForce = FALSE;
    m_undoOper = oper;

    delete m_undo[EDITUNDOMAX-1].text;

    for ( i=EDITUNDOMAX-1 ; i>=1 ; i-- )
    {
        m_undo[i] = m_undo[i-1];
    }

    len = m_len;
    if ( len == 0 )  len ++;
    m_undo[0].text = (char*)malloc(sizeof(char)*(len+1));
    memcpy(m_undo[0].text, m_text, m_len);
    m_undo[0].len = m_len;

    m_undo[0].cursor1 = m_cursor1;
    m_undo[0].cursor2 = m_cursor2;
    m_undo[0].lineFirst = m_lineFirst;
}

// Back to previous state.

BOOL CEdit::UndoRecall()
{
    int     i;

    if ( m_undo[0].text == 0 )  return FALSE;

    m_len = m_undo[0].len;
    memcpy(m_text, m_undo[0].text, m_len);

    m_cursor1 = m_undo[0].cursor1;
    m_cursor2 = m_undo[0].cursor2;
    m_lineFirst = m_undo[0].lineFirst;

    for ( i=0 ; i<EDITUNDOMAX-1 ; i++ )
    {
        m_undo[i] = m_undo[i+1];
    }
    m_undo[EDITUNDOMAX-1].text = 0;

    m_bUndoForce = TRUE;
    Justif();
    ColumnFix();
    SendModifEvent();
    return TRUE;
}


// Clears the format of all characters.

BOOL CEdit::ClearFormat()
{
    if ( m_format == 0 )
    {
        SetMultiFont(TRUE);
    }
    memset(m_format, m_fontType, m_len);

    return TRUE;
}

// Changes the format of a sequence of characters.

BOOL CEdit::SetFormat(int cursor1, int cursor2, int format)
{
    int     i;

    if ( m_format == 0 )  return FALSE;

    for ( i=cursor1 ; i<cursor2 ; i++ )
    {
        m_format[i] |= format;
    }

    return TRUE;
}


