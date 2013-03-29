// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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


#include "ui/edit.h"

#include "app/app.h"

#include <string.h>

namespace Ui {

const float MARGX           = (5.0f/640.0f);
const float MARGY           = (5.0f/480.0f);
const float MARGYS          = (4.0f/480.0f);
const float MARGY1          = (1.0f/480.0f);
//! time limit for double-click
const float DELAY_DBCLICK   = 0.3f;
//! time limit for scroll
const float DELAY_SCROLL    = 0.1f;
//! expansion for \b;
const float BIG_FONT        = 1.6f;




//! Indicates whether a character is a space.

bool IsSpace(int character)
{
    return ( character == ' '  ||
             character == '\t' ||
             character == '\n' );
}

//! Indicates whether a character is part of a word.

bool IsWord(int character)
{
    char    c;

    c = tolower(GetNoAccent(character));

    return ( (c >= 'a' && c <= 'z') ||
             (c >= '0' && c <= '9') ||
             c == '_' );
}

//! Indicates whether a character is a word separator.

bool IsSep(int character)
{
    if ( IsSpace(character) )  return false;
    return !IsWord(character);
}



//! Object's constructor.
CEdit::CEdit () : CControl ()
{
    Math::Point pos;
    int     i;

    m_maxChar = 100;
    m_text = new char[m_maxChar+1];
    memset(m_text, 0, m_maxChar+1);
    m_len = 0;

    memset(m_lineOffset, 0, sizeof(int) * EDITLINEMAX);

    m_fontType = Gfx::FONT_COURIER;
    m_scroll        = 0;
    m_bEdit         = true;
    m_bHilite       = true;
    m_bInsideScroll = true;
    m_bCapture      = false;
    m_bDisplaySpec  = false;
    m_bSoluce       = false;
    m_bGeneric      = false;
    m_bAutoIndent   = false;
    m_cursor1       = 0;
    m_cursor2       = 0;
    m_column        = 0;
    m_imageTotal    = 0;

    HyperFlush();

    for ( i=0 ; i<EDITUNDOMAX ; i++ )
    {
        m_undo[i].text = nullptr;
    }
    m_bUndoForce = true;
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
        m_undo[i].text = nullptr;
    }

    if (m_text != nullptr)
    {
        delete[] m_text;
        m_text = nullptr;
    }

    delete m_scroll;
    m_scroll = nullptr;
}


// Creates a new editable line.

bool CEdit::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    CScroll*    pc;
    Math::Point     start, end;

    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();
    CControl::Create(pos, dim, icon, eventType);

    m_len = 0;
    m_lineFirst = 0;
    m_time = 0.0f;
    m_timeBlink = 0.0f;
    m_timeLastClick = 0.0f;
    m_timeLastScroll = 0.0f;

    m_bMulti = false;
    MoveAdjust();
    if ( m_lineVisible <= 1 )
    {
        m_bMulti = false;
    }
    else
    {
        m_bMulti = true;
        MoveAdjust();  // readjusts multi-line mode
        m_scroll = new Ui::CScroll();
        pc = static_cast<CScroll*>(m_scroll);
        pc->Create(pos, dim, -1, EVENT_NULL);
        MoveAdjust();
    }

    return true;
}


void CEdit::SetPos(Math::Point pos)
{
    CControl::SetPos(pos);
    MoveAdjust();
}

void CEdit::SetDim(Math::Point dim)
{
    CControl::SetDim(dim);
    MoveAdjust();
}

void CEdit::MoveAdjust()
{
    Math::Point     pos, dim;
    float       height;

    m_lineDescent = m_engine->GetText()->GetDescent(m_fontType, m_fontSize);
    m_lineAscent  = m_engine->GetText()->GetAscent(m_fontType, m_fontSize);
    m_lineHeight  = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);

    height = m_dim.y-(m_bMulti?MARGY*2.0f:MARGY1);
    m_lineVisible = static_cast<int>((height/m_lineHeight));

    if ( m_scroll != 0 )
    {
        if ( m_bInsideScroll )
        {
            pos.x = m_pos.x + m_dim.x - MARGX-SCROLL_WIDTH;
            pos.y = m_pos.y + MARGYS;
            dim.x = SCROLL_WIDTH;
            dim.y = m_dim.y - MARGYS*2.0f;
        }
        else
        {
            pos.x = m_pos.x + m_dim.x - SCROLL_WIDTH;
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
    GlintCreate(pos, false, false);
}


// Management of an event.

bool CEdit::EventProcess(const Event &event)
{
    bool    bShift, bControl;

    if ( (m_state & STATE_VISIBLE) == 0 )  return true;

    if (event.type == EVENT_MOUSE_WHEEL &&
        event.mouseWheel.dir == WHEEL_UP &&
         Detect(event.mousePos)            )
    {
        Scroll(m_lineFirst-3, true);
        return true;
    }
    if (event.type == EVENT_MOUSE_WHEEL  &&
        event.mouseWheel.dir == WHEEL_DOWN  &&
        Detect(event.mousePos)            )
    {
        Scroll(m_lineFirst+3, true);
        return true;
    }

    CControl::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        m_time += event.rTime;
        m_timeBlink += event.rTime;
    }

    if ( event.type == EVENT_MOUSE_MOVE )
    {
        if ( Detect(event.mousePos) && 
             event.mousePos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
        {
            if ( m_bEdit )
            {
                m_engine->SetMouseType(Gfx::ENG_MOUSE_EDIT);
            }
            else
            {
                if ( IsLinkPos(event.mousePos) )
                {
                    m_engine->SetMouseType(Gfx::ENG_MOUSE_HAND);
                }
                else
                {
                    m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
                }
            }
        }
    }

    if ( m_scroll != nullptr && !m_bGeneric )
    {
        m_scroll->EventProcess(event);

        if ( event.type == m_scroll->GetEventType() )
        {
            Scroll();
            return true;
        }
    }

    if ( event.type == EVENT_KEY_DOWN && m_bFocus )
    {
        bShift   = ( (event.kmodState & KEY_MOD(SHIFT) ) != 0 );
        bControl = ( (event.kmodState & KEY_MOD(CTRL) ) != 0);

        if ( (event.key.unicode == 'X'       && !bShift &&  bControl) ||
             ((event.kmodState & KEY_MOD(CTRL)) != 0 &&  bShift && !bControl) )
        {
            Cut();
            return true;
        }
        if ( (event.key.unicode == 'C'       && !bShift &&  bControl) ||
             ((event.kmodState & KEY_MOD(CTRL)) != 0 && !bShift &&  bControl) )
        {
            Copy();
            return true;
        }
        if ( (event.key.unicode == 'V'       && !bShift &&  bControl) ||
             ((event.kmodState & KEY_MOD(CTRL)) != 0 &&  bShift && !bControl) )
        {
            Paste();
            return true;
        }

        if ( event.key.unicode == 'A' && !bShift && bControl )
        {
            SetCursor(999999, 0);
            return true;
        }

        if ( event.key.unicode == 'O' && !bShift && bControl )
        {
            Event   newEvent(EVENT_STUDIO_OPEN);
//            m_event->NewEvent(newEvent, EVENT_STUDIO_OPEN);
            m_event->AddEvent(newEvent);
        }
        if ( event.key.unicode == 'S' && !bShift && bControl )
        {
            Event   newEvent( EVENT_STUDIO_SAVE );
//            m_event->MakeEvent(newEvent, EVENT_STUDIO_SAVE);
            m_event->AddEvent(newEvent);
        }

        if ( event.key.unicode == 'Z' && !bShift && bControl )
        {
            Undo();
            return true;
        }

        if ( event.key.unicode == 'U' && !bShift && bControl )
        {
            if ( MinMaj(false) )  return true;
        }
        if ( event.key.unicode == 'U' && bShift && bControl )
        {
            if ( MinMaj(true) )  return true;
        }

        if ( event.key.key == KEY(TAB) && !bShift && !bControl && !m_bAutoIndent )
        {
            if ( Shift(false) )  return true;
        }
        if ( event.key.key == KEY(TAB) && bShift && !bControl && !m_bAutoIndent )
        {
            if ( Shift(true) )  return true;
        }

        if ( m_bEdit )
        {
            if ( event.key.key == KEY(LEFT) )
            {
                MoveChar(-1, bControl, bShift);
                return true;
            }
            if ( event.key.key == KEY(RIGHT) )
            {
                MoveChar(1, bControl, bShift);
                return true;
            }
            if ( event.key.key == KEY(UP) )
            {
                MoveLine(-1, bControl, bShift);
                return true;
            }
            if ( event.key.key == KEY(DOWN) )
            {
                MoveLine(1, bControl, bShift);
                return true;
            }

            if ( event.key.key == KEY(PAGEUP) )  // PageUp ?
            {
                MoveLine(-(m_lineVisible-1), bControl, bShift);
                return true;
            }
            if ( event.key.key == KEY(PAGEDOWN) )  // PageDown ?
            {
                MoveLine(m_lineVisible-1, bControl, bShift);
                return true;
            }
        }
        else
        {
            if ( event.key.key == KEY(LEFT) ||
                 event.key.key == KEY(UP)   )
            {
                Scroll(m_lineFirst-1, true);
                return true;
            }
            if ( event.key.key == KEY(RIGHT) ||
                 event.key.key == KEY(DOWN)  )
            {
                Scroll(m_lineFirst+1, true);
                return true;
            }

            if ( event.key.key == KEY(PAGEUP) )  // PageUp ?
            {
                Scroll(m_lineFirst-(m_lineVisible-1), true);
                return true;
            }
            if ( event.key.key == KEY(PAGEDOWN) )  // PageDown ?
            {
                Scroll(m_lineFirst+(m_lineVisible-1), true);
                return true;
            }
        }

        if ( event.key.key == KEY(HOME) )
        {
            MoveHome(bControl, bShift);
            return true;
        }
        if ( event.key.key == KEY(END) )
        {
            MoveEnd(bControl, bShift);
            return true;
        }

        if ( event.key.key == KEY(BACKSPACE) )  // backspace ( <- ) ?
        {
            Delete(-1);
            SendModifEvent();
            return true;
        }
        if ( event.key.key == KEY(DELETE) )
        {
            Delete(1);
            SendModifEvent();
            return true;
        }

        if ( event.key.key == KEY(RETURN) )
        {
            Insert('\n');
            SendModifEvent();
            return true;
        }
        if ( event.key.key == KEY(TAB) )
        {
            Insert('\t');
            SendModifEvent();
            return true;
        }
    }

    if ( event.type == EVENT_KEY_DOWN && m_bFocus )
    {
        if (event.key.unicode >= ' ')
        {
            Insert(static_cast<char>(event.key.unicode)); // TODO: insert utf-8 char
            SendModifEvent();
            return true;
        }
    }

    if ( event.type == EVENT_FOCUS )
    {
        if ( event.customParam == m_eventType )
        {
            m_bFocus = true;
        }
        else
        {
            m_bFocus = false;
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT)
    {
        m_mouseFirstPos = event.mousePos;
        m_mouseLastPos  = event.mousePos;
        if ( Detect(event.mousePos) )
        {
            if ( event.mousePos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
            {
                MouseClick(event.mousePos);
                if ( m_bEdit || m_bHilite )  m_bCapture = true;
            }
            m_bFocus = true;
        }
        else
        {
            m_bFocus = false;
        }
    }

    if ( event.type == EVENT_MOUSE_MOVE && m_bCapture )
    {
        m_mouseLastPos = event.mousePos;
        MouseMove(event.mousePos);
    }

    if ( event.type == EVENT_FRAME && m_bCapture )
    {
        MouseMove(m_mouseLastPos);
    }

    if ( event.type == EVENT_MOUSE_BUTTON_UP &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT)
    {
        if ( Detect(event.mousePos) )
        {
            if ( event.mousePos.x < m_pos.x+m_dim.x-(m_bMulti?MARGX+SCROLL_WIDTH:0.0f) )
            {
                MouseRelease(m_mouseFirstPos);
            }
        }
        if ( m_bCapture )
        {
            if ( m_timeLastClick+DELAY_DBCLICK > m_time )  // double-click ?
            {
                MouseDoubleClick(event.mousePos);
            }
            m_timeLastClick = m_time;
            m_bCapture = false;
        }
    }

    return true;
}


// Sends an event to indicate that the text was modified.

void CEdit::SendModifEvent()
{
    Event   newEvent (m_eventType);

//    m_event->MakeEvent(newEvent, m_eventType);
    m_event->AddEvent(newEvent);
}


// Detects whether the mouse is over a hyperlink character.

bool CEdit::IsLinkPos(Math::Point pos)
{
    int     i;

    if ( m_format.size() == 0 )  return false;

    i = MouseDetect(pos);
    if ( i == -1 )  return false;
    if ( i >= m_len )  return false;

    if ( m_format.size() > static_cast<unsigned int>(i) && ((m_format[i] & Gfx::FONT_MASK_HIGHLIGHT) == Gfx::FONT_HIGHLIGHT_LINK))  return true; // TODO
    return false;
}


// Positions the cursor after a double click.

void CEdit::MouseDoubleClick(Math::Point mouse)
{
    int     i, character;

    if ( m_bMulti )  // Multi-line?
    {
        i = MouseDetect(mouse);
        if ( i == -1 )  return;

        while ( i > 0 )
        {
            character = static_cast<unsigned char>(m_text[i-1]);
            if ( !IsWord(character) )  break;
            i --;
        }
        m_cursor2 = i;

        while ( i < m_len )
        {
            character = static_cast<unsigned char>(m_text[i]);
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

    m_bUndoForce = true;

    Justif();
    ColumnFix();
}

// Positions the cursor when clicked.

void CEdit::MouseClick(Math::Point mouse)
{
    int     i;

    i = MouseDetect(mouse);
    if ( i == -1 )  return;

    if ( m_bEdit || m_bHilite )
    {
        m_cursor1 = i;
        m_cursor2 = i;
        m_bUndoForce = true;
        m_timeBlink = 0.0f;  // lights the cursor immediately
        ColumnFix();
    }
}

// Positions the cursor when clicked released.

void CEdit::MouseRelease(Math::Point mouse)
{
    int     i, j, rank;

    i = MouseDetect(mouse);
    if ( i == -1 )  return;

    if ( !m_bEdit )
    {
        if ( m_format.size() > 0 && i < m_len && m_cursor1 == m_cursor2 &&
            (m_format[i]&Gfx::FONT_MASK_HIGHLIGHT) == Gfx::FONT_HIGHLIGHT_LINK) //TODO
        {
            rank = -1;
            for ( j=0 ; j<=i ; j++ )
            {
                if ( (j == 0 || (m_format[j-1]&Gfx::FONT_MASK_HIGHLIGHT) != Gfx::FONT_HIGHLIGHT_LINK) && // TODO check if good
                     (m_format[j+0]&Gfx::FONT_MASK_HIGHLIGHT) == Gfx::FONT_HIGHLIGHT_LINK) // TODO
                {
                    rank ++;
                }
            }
            HyperJump(m_link[rank].name, m_link[rank].marker);
        }
    }
}

// Positions the cursor after movement.

void CEdit::MouseMove(Math::Point mouse)
{
    int     i;

    if ( m_bMulti &&
         m_timeLastScroll+DELAY_SCROLL <= m_time )
    {
        if ( mouse.y > m_pos.y+m_dim.y )  // above?
        {
            Scroll(m_lineFirst-1, false);
            mouse.y = m_pos.y+m_dim.y-MARGY-m_lineHeight/2.0f;
        }
        if ( mouse.y < m_pos.y )  // lower?
        {
            Scroll(m_lineFirst+1, false);
            mouse.y = m_pos.y+m_dim.y-MARGY-m_lineVisible*m_lineHeight+m_lineHeight/2.0f;
        }
        m_timeLastScroll = m_time;
    }

    i = MouseDetect(mouse);
    if ( i != -1 )
    {
        m_cursor1 = i;
        m_bUndoForce = true;
        m_timeBlink = 0.0f;  // lights the cursor immediately
        ColumnFix();
    }
}

// Positions the cursor when clicked.

int CEdit::MouseDetect(Math::Point mouse)
{
    Math::Point pos;
    float   indentLength, offset, size;
    int     i, len, c;
    bool    bTitle;

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->GetText()->GetCharWidth(static_cast<Gfx::UTF8Char>(' '), m_fontType, m_fontSize, 0.0f)
                        * m_engine->GetEditIndentValue();
    }

    pos.y = m_pos.y+m_dim.y-m_lineHeight-(m_bMulti?MARGY:MARGY1);
    for ( i=m_lineFirst ; i<m_lineTotal ; i++ )
    {
        bTitle = ( m_format.size() > 0 && (m_format[m_lineOffset[i]]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_BIG );

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

            if ( m_format.size() == 0 )
            {
//                c = m_engine->GetText()->Detect(m_text+m_lineOffset[i],
//                                                len, offset, m_fontSize,
//                                                m_fontStretch, m_fontType);
                c = m_engine->GetText()->Detect(std::string(m_text+m_lineOffset[i]).substr(0, len), m_fontType, m_fontSize, offset); // TODO check if good
            }
            else
            {
                size = m_fontSize;
                if ( bTitle )  size *= Gfx::FONT_SIZE_BIG;

//                c = m_engine->GetText()->Detect(m_text+m_lineOffset[i],
//                                                m_format+m_lineOffset[i],
//                                                len, offset, size,
//                                                m_fontStretch);
                c = m_engine->GetText()->Detect(std::string(m_text+m_lineOffset[i]).substr(0, len),
                                                m_format.begin() + m_lineOffset[i],
                                                m_format.end(),
                                                size,
                                                offset); // TODO check if good
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

void CEdit::HyperHome(std::string filename)
{
    HyperFlush();
    HyperAdd(filename, 0);
}

// Performs a hyper jump through a link.

void CEdit::HyperJump(std::string name, std::string marker)
{
    std::string filename;
    std:: string sMarker;
    int     i, line, pos;

    if ( m_historyCurrent >= 0 )
    {
        m_history[m_historyCurrent].firstLine = m_lineFirst;
    }

    sMarker = marker;

//? sprintf(filename, "help\\%s.txt", name);
      
    if ( name[0] == '%' ) {
        filename = GetProfile().GetUserBasedPath(name, "") + ".txt";
    } else {
        filename = std::string("help/") + CApplication::GetInstancePointer()->GetLanguageChar() + "/" + name + std::string(".txt");
    }
    
    if ( ReadText(filename) )
    {
        Justif();

        line = 0;
        for ( i=0 ; i<m_markerTotal ; i++ )
        {
            if (sMarker == m_marker[i].name)
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

bool CEdit::HyperAdd(std::string filename, int firstLine)
{
    if ( m_historyCurrent >= EDITHISTORYMAX-1 )  return false;

    m_historyCurrent ++;
    m_history[m_historyCurrent].filename = filename;
    m_history[m_historyCurrent].firstLine = firstLine;

    m_historyTotal = m_historyCurrent+1;
    return true;
}

// Indicates whether a button EVENT_HYPER_ * is active or not.

bool CEdit::HyperTest(EventType event)
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

    return false;
}

// Performs the action corresponding to a button EVENT_HYPER_ *.

bool CEdit::HyperGo(EventType event)
{
    if ( !HyperTest(event) )  return false;

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
    return true;
}


// Draw the editable line.

void CEdit::Draw()
{
    Math::Point     pos, ppos, dim, start, end;
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
    if ( c1 > c2 )  Math::Swap(c1, c2);  // always c1 <= c2

    if ( m_bInsideScroll )
    {
        dim.x -= m_bMulti?SCROLL_WIDTH:0.0f + (1.0f/640.0f);
    }

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->GetText()->GetCharWidth(static_cast<Gfx::UTF8Char>(' '), m_fontType, m_fontSize, 0.0f)
                        * m_engine->GetEditIndentValue();
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
            const char *s = "\t";  // line | dotted
            for ( j=0 ; j<m_lineIndent[i] ; j++ )
            {
                m_engine->GetText()->DrawText(s, m_fontType, m_fontSize, pos, 1.0f, Gfx::TEXT_ALIGN_LEFT, 0);
                pos.x += indentLength;
            }
        }

        beg = m_lineOffset[i];
        len = m_lineOffset[i+1] - m_lineOffset[i];

        ppos = pos;
        size = m_fontSize;

        // Headline \b;?
        if ( beg+len < m_len && m_format.size() > static_cast<unsigned int>(beg) &&
             (m_format[beg]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_BIG )
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
        if ( beg+len < m_len && m_format.size() > static_cast<unsigned int>(beg) &&
             (m_format[beg]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_NORM )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 2);  // blue gradient background ->
        }

        // Subtitle \s;?
        if ( beg+len < m_len && m_format.size() > static_cast<unsigned int>(beg) &&
             (m_format[beg]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_LITTLE )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 3);  // yellow background gradient ->
        }

        // Table \tab;?
        if ( beg+len < m_len && m_format.size() > static_cast<unsigned int>(beg) &&
             (m_format[beg]&Gfx::FONT_MASK_HIGHLIGHT) == Gfx::FONT_HIGHLIGHT_TABLE )
        {
            start.x = ppos.x-MARGX;
            end.x   = dim.x-MARGX*2.0f;
            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            DrawPart(start, end, 11);  // fond orange d�grad� ->
        }

        // Image \image; ?
        if ( beg+len < m_len && m_format.size() > static_cast<unsigned int>(beg) &&
             (m_format[beg]&Gfx::FONT_MASK_IMAGE) != 0 )
        {
            line = 1;
            while ( true )  // includes the image slices
            {
                if ( i+line >= m_lineTotal                ||
                     i+line >= m_lineFirst+m_lineVisible  ||
                     (m_format.size() > static_cast<unsigned int>(beg+line) && m_format[beg+line]&Gfx::FONT_MASK_IMAGE) == 0 )  break;
                line ++;
            }

            iIndex = m_text[beg];  // character = index in m_image
            pos.y -= m_lineHeight*(line-1);
            DrawImage(pos, m_image[iIndex].name,
                      m_image[iIndex].width*(m_fontSize/Gfx::FONT_SIZE_SMALL),
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

            if ( m_format.size() == 0 )
            {
                start.x = ppos.x+m_engine->GetText()->GetStringWidth(std::string(m_text+beg).substr(0, o1-beg), m_fontType, size);
                end.x   = m_engine->GetText()->GetStringWidth(std::string(m_text+o1).substr(0, o2-o1), m_fontType, size);
            }
            else
            {
                start.x = ppos.x+m_engine->GetText()->GetStringWidth(std::string(m_text+beg).substr(0, o1-beg),
                                                                     m_format.begin() + beg,
                                                                     m_format.end(),
                                                                     size);
                end.x   = m_engine->GetText()->GetStringWidth(std::string(m_text+o1).substr(0, o2-o1),
                                                              m_format.begin() + o1,
                                                              m_format.end(),
                                                              size);
            }

            start.y = ppos.y-(m_bMulti?0.0f:MARGY1);
            end.y   = m_lineHeight;
            if ( m_format.size() > static_cast<unsigned int>(beg) && (m_format[beg]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_BIG)  end.y *= BIG_FONT;
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
        if ( m_format.size() == 0 )
        {
            m_engine->GetText()->DrawText(std::string(m_text+beg).substr(0, len), m_fontType, size, ppos, m_dim.x, Gfx::TEXT_ALIGN_LEFT, eol);
        }
        else
        {
            m_engine->GetText()->DrawText(std::string(m_text+beg).substr(0, len),
                                          m_format.begin() + beg,
                                          m_format.end(),
                                          size,
                                          ppos,
                                          m_dim.x,
                                          Gfx::TEXT_ALIGN_LEFT,
                                          eol);
        }

        pos.y -= m_lineHeight;

        if ( i < m_lineTotal-2 && m_lineOffset[i+1] == m_lineOffset[i+2] )
        {
            pos.y -= m_lineHeight;  // double jump line \b;
            i ++;
        }
    }

    // Shows the cursor.
    if ( (m_bEdit && m_bFocus && m_bHilite && Math::Mod(m_timeBlink, 1.0f) <= 0.5f) )  // it blinks
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

                if ( m_format.size() == 0 )
                {
                    m_engine->GetText()->SizeText(std::string(m_text+m_lineOffset[i]).substr(0, len), m_fontType,
                                                  size, pos, Gfx::TEXT_ALIGN_LEFT,
                                                  start, end);
                }
                else
                {
                    m_engine->GetText()->SizeText(std::string(m_text+m_lineOffset[i]).substr(0, len),
                                                  m_format.begin() + m_lineOffset[i],
                                                  m_format.end(),
                                                  size, pos, Gfx::TEXT_ALIGN_LEFT,
                                                  start, end);
                }

                pos.x = end.x;
                break;
            }
            pos.y -= m_lineHeight;
        }
        pos.x -= 1.0f / 640.0f;
        dim.x = 2.0f / 640.0f;
        dim.y = m_lineHeight;
        DrawPart(pos, dim, 0);  // red
    }

    if ( m_scroll != 0 && !m_bGeneric )
    {
        m_scroll->Draw();
    }
}

// Draw an image part.

void CEdit::DrawImage(Math::Point pos, std::string name, float width,
                      float offset, float height, int nbLine)
{
    Math::Point uv1, uv2, dim;
    float dp;
    std::string filename;

    filename = GetProfile().GetUserBasedPath(name, "diagram") + ".png";

    m_engine->SetTexture(filename);
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

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

void CEdit::DrawBack(Math::Point pos, Math::Point dim)
{
    Math::Point     uv1,uv2, corner;
    float       dp;

    if ( m_bGeneric )  return;

    m_engine->SetTexture("button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

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

void CEdit::DrawPart(Math::Point pos, Math::Point dim, int icon)
{
    Math::Point     uv1, uv2;
    float       dp;

#if _POLISH
    m_engine->SetTexture("textp.png");
#else
    m_engine->SetTexture("text.png");
#endif
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

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

void CEdit::SetText(const char *text, bool bNew)
{
    int     i, j, font;
    bool    bBOL;
    
    if ( !bNew )  UndoMemorize(OPERUNDO_SPEC);

    m_len = strlen(text);
    if ( m_len > m_maxChar )  m_len = m_maxChar;

    if ( m_format.size() == 0 )
    {
        if ( m_bAutoIndent )
        {
            j = 0;
            bBOL = true;
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
        bBOL = true;
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
                    font &= ~Gfx::FONT_MASK_FONT;
                    font |= Gfx::FONT_COLOBOT;
                    i += 2;
                }
                else if ( text[i+1] == 'c' )  // cbot ?
                {
                    font &= ~Gfx::FONT_MASK_FONT;
                    font |= Gfx::FONT_COURIER;
                    i += 2;
                }
                else if ( text[i+1] == 'b' )  // big title ?
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_BIG;
                    i += 2;
                }
                else if ( text[i+1] == 't' )  // title ?
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_NORM;
                    i += 2;
                }
                else if ( text[i+1] == 's' )  // subtitle ?
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_LITTLE;
                    i += 2;
                }
            }
            else
            {
                m_text[j] = text[i];
                m_format[j] = font;
                j ++;

                font &= ~Gfx::FONT_MASK_TITLE;  // reset title
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

char* CEdit::GetText()
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

int CEdit::GetTextLength()
{
    return m_len;
}



// Returns a name in a command.
// \x nom1 nom2 nom3;

std::string GetNameParam(std::string cmd, int rank)
{
    std::vector<std::string> results;    
    boost::split(results, cmd, boost::is_any_of(" ;"));
    
    if (results.size() > static_cast<unsigned int>(rank)) {
        return results.at(rank);
    }

    return "";
}

// Returns a number of a command.
// \x nom n1 n2;

int GetValueParam(std::string cmd, int rank)
{
    std::vector<std::string> results;    
    boost::split(results, cmd, boost::is_any_of(" ;"));
    int return_value = 0;
    
    if (results.size() > static_cast<unsigned int>(rank)) {
        return_value = atoi(results.at(rank).c_str());
    }

    return return_value;
}

// Frees all images.

void CEdit::FreeImage()
{
    std::string filename;

    for (int i = 0 ; i < m_imageTotal; i++ ) {
        filename = GetProfile().GetUserBasedPath(m_image[i].name, "diagram") + ".png";
        m_engine->DeleteTexture(filename);
    }
}

// Reads the texture of an image.

void CEdit::LoadImage(std::string name)
{
    std::string filename;
    filename = GetProfile().GetUserBasedPath(name, "diagram") + ".png";
    m_engine->LoadTexture(filename);
}

// Read from a text file.

bool CEdit::ReadText(std::string filename, int addSize)
{
    FILE        *file = NULL;
    char        *buffer;
    int         len, i, j, n, font, iIndex, iLines, iCount, iLink, res;
    char        iName[50];
    char        text[50];
    float       iWidth;
    InputSlot   slot;
    bool        bInSoluce, bBOL;

    if ( filename[0] == 0 )  return false;
    boost::replace_all(filename, "\\", "/");
    
    /* This is ugly but doesn't require many changes in code. If file doesn't
       exists it's posible filename is absolute not full path */
    std::string path = filename;
    if (!fs::exists(path)) {
        path = CApplication::GetInstancePointer()->GetDataDirPath() + "/" + filename;
    }

    file = fopen(fs::path(path).make_preferred().string().c_str(), "rb");
    if ( file == NULL )  return false;

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    m_maxChar = len+addSize+100;
    m_len = len;
    m_cursor1 = 0;
    m_cursor2 = 0;

    FreeImage();

    if (m_text != nullptr)
        delete[] m_text;

    m_text = new char[m_maxChar+1];
    memset(m_text, 0, m_maxChar+1);

    buffer = new char[m_maxChar+1];
    memset(buffer, 0, m_maxChar+1);

    fread(buffer, 1, len, file);

    m_format.clear();
    m_format.reserve(m_maxChar+1);
    for (i = 0; i <= m_maxChar+1; i++) {
        m_format.push_back(0);
    }
    
    fclose(file);

    bInSoluce = false;
    font = m_fontType;
    iIndex = 0;
    iLink = 0;
    m_imageTotal = 0;
    m_markerTotal = 0;
    i = j = 0;
    bBOL = true;
    while ( i < m_len )
    {
        if ( m_bAutoIndent )
        {
            if ( buffer[i] == '\t' )
            {
                if ( !bBOL )
                {
                    m_text[j] = buffer[i];
                    //if ( m_format.size() > 0 )
                    m_format[j] = font;
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
        else if ( buffer[i] == '\\' && buffer[i+2] == ';' )
        {
            if ( buffer[i+1] == 'n' )  // normal ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_FONT;
                    font |= Gfx::FONT_COLOBOT;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'c' )  // cbot ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_FONT;
                    font |= Gfx::FONT_COURIER;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'b' )  // big title ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_BIG;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 't' )  // title ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_NORM;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 's' )  // subtitle ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_TITLE;
                    font |= Gfx::FONT_TITLE_LITTLE;
                }
                i += 3;
            }
            else if ( buffer[i+1] == 'l' )  // link ?
            {
                if ( m_bSoluce || !bInSoluce )
                {
                    font &= ~Gfx::FONT_MASK_HIGHLIGHT;
                    font |= Gfx::FONT_HIGHLIGHT_LINK;
                }
                i += 3;
            }
            else
            {
                i += 3;
            }
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \u marker name; ?
                  buffer[i+1] == 'u'  &&
                  buffer[i+2] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( iLink < EDITLINKMAX )
                {
                    m_link[iLink].name = GetNameParam(buffer+i+3, 0);
                    m_link[iLink].marker = GetNameParam(buffer+i+3, 1);
                    iLink ++;
                }
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if (// m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \m marker; ?
                  buffer[i+1] == 'm'  &&
                  buffer[i+2] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( m_markerTotal < EDITLINKMAX )
                {
                    m_marker[m_markerTotal].name = GetNameParam(buffer+i+3, 0);
                    m_marker[m_markerTotal].pos = j;
                    m_markerTotal ++;
                }
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( //m_format.size() > 0       &&
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

                strcpy(iName, GetNameParam(buffer+i+7, 0).c_str());

//?             iWidth = m_lineHeight*RetValueParam(buffer+i+7, 1);
                iWidth = static_cast<float>(GetValueParam(buffer+i+7, 1));
                iWidth *= m_engine->GetText()->GetHeight(Gfx::FONT_COLOBOT, Gfx::FONT_SIZE_SMALL);
                iLines = GetValueParam(buffer+i+7, 2);
                LoadImage(std::string(iName));

                // A part of image per line of text.
                for ( iCount=0 ; iCount<iLines ; iCount++ )
                {
                    m_image[iIndex].name = iName;
                    m_image[iIndex].offset = static_cast<float>(iCount/iLines);
                    m_image[iIndex].height = 1.0f/iLines;
                    m_image[iIndex].width = iWidth*0.75f;

                    m_text[j] = static_cast<char>(iIndex++);  // as an index into m_image
                    m_format[j] = static_cast<unsigned char>(Gfx::FONT_MASK_IMAGE);
                    j ++;
                }
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( //m_format.size() > 0       &&
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
                m_text[j] = GetValueParam(buffer+i+8, 0);
                m_format[j] = font|Gfx::FONT_BUTTON;
                j ++;
            }
            i += strchr(buffer+i, ';')-(buffer+i)+1;
        }
        else if ( //m_format.size() > 0       &&
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
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
                font |= Gfx::FONT_HIGHLIGHT_TOKEN;
            }
            i += 7;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \type; ?
                  buffer[i+1] == 't'  &&
                  buffer[i+2] == 'y'  &&
                  buffer[i+3] == 'p'  &&
                  buffer[i+4] == 'e'  &&
                  buffer[i+5] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
                font |= Gfx::FONT_HIGHLIGHT_TYPE;
            }
            i += 6;
        }
        else if ( //m_format.size() > 0       &&
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
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
                font |= Gfx::FONT_HIGHLIGHT_CONST;
            }
            i += 7;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \key; ?
                  buffer[i+1] == 'k'  &&
                  buffer[i+2] == 'e'  &&
                  buffer[i+3] == 'y'  &&
                  buffer[i+4] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
                font |= Gfx::FONT_HIGHLIGHT_KEY;
            }
            i += 5;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \tab; ?
                  buffer[i+1] == 't'  &&
                  buffer[i+2] == 'a'  &&
                  buffer[i+3] == 'b'  &&
                  buffer[i+4] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font |= Gfx::FONT_HIGHLIGHT_TABLE;
            }
            i += 5;
        }
        else if (// m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \norm; ?
                  buffer[i+1] == 'n'  &&
                  buffer[i+2] == 'o'  &&
                  buffer[i+3] == 'r'  &&
                  buffer[i+4] == 'm'  &&
                  buffer[i+5] == ';'  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                font &= ~Gfx::FONT_MASK_HIGHLIGHT;
            }
            i += 6;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \begin soluce; ?
                  buffer[i+1] == 'b'  &&
                  buffer[i+2] == 's'  &&
                  buffer[i+3] == ';'  )
        {
            bInSoluce = true;
            i += 4;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \end soluce; ?
                  buffer[i+1] == 'e'  &&
                  buffer[i+2] == 's'  &&
                  buffer[i+3] == ';'  )
        {
            bInSoluce = false;
            i += 4;
        }
        else if ( //m_format.size() > 0       &&
                  buffer[i+0] == '\\' &&  // \key name; ?
                  buffer[i+1] == 'k'  &&
                  buffer[i+2] == 'e'  &&
                  buffer[i+3] == 'y'  &&
                  buffer[i+4] == ' '  )
        {
            if ( m_bSoluce || !bInSoluce )
            {
                if ( SearchKey(buffer+i+5, slot) )
                {
                    CRobotMain* main = CRobotMain::GetInstancePointer();
                    res = main->GetInputBinding(slot).primary;
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

                            res = main->GetInputBinding(slot).secondary;
                            if ( res != 0 )
                            {
                                if ( GetResource(RES_KEY, res, iName) )
                                {
                                    GetResource(RES_TEXT, RT_KEY_OR, text);
                                    n = 0;
                                    while ( text[n] != 0 )
                                    {
                                        m_text[j] = text[n++];
                                        m_format[j] = font&~Gfx::FONT_MASK_HIGHLIGHT;
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
                //if ( m_format.size() > 0 )
                m_format[j] = font;
                j ++;
            }
            i ++;

            font &= ~Gfx::FONT_MASK_TITLE;  // reset title

            if ( (font&Gfx::FONT_MASK_HIGHLIGHT) == Gfx::FONT_HIGHLIGHT_TABLE )
            {
                font &= ~Gfx::FONT_HIGHLIGHT_TABLE;
            }
        }
    }
    m_len = j;
    m_imageTotal = iIndex;

    delete[] buffer;

    Justif();
    ColumnFix();
    return true;
}

// Writes all the text in a file.

bool CEdit::WriteText(std::string filename)
{
    FILE*       file;
    char        buffer[1000+20];
    int         i, j, k, n;
    float       iDim;

    if ( filename[0] == 0 )  return false;
    file = fopen(filename.c_str(), "wb");
    if ( file == NULL )  return false;

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

    return true;
}


// Manage the number of max characters editable.

void CEdit::SetMaxChar(int max)
{
    FreeImage();

    if (m_text != nullptr)
        delete[] m_text;

    m_maxChar = max;

    m_text = new char[m_maxChar+1];
    memset(m_text, 0, m_maxChar+1);

    m_format.clear();
    m_format.reserve(m_maxChar+1);
    for (int i = 0; i <= m_maxChar+1; i++) {
        m_format.push_back(0);
    }

    m_len = 0;
    m_cursor1 = 0;
    m_cursor2 = 0;
    Justif();
    UndoFlush();
}

int CEdit::GetMaxChar()
{
    return m_maxChar;
}


// Mode management "editable".

void CEdit::SetEditCap(bool bMode)
{
    m_bEdit = bMode;
}

bool CEdit::GetEditCap()
{
    return m_bEdit;
}

// Mode management "hilitable" (that's the franch).

void CEdit::SetHighlightCap(bool bEnable)
{
    m_bHilite = bEnable;
}

bool CEdit::GetHighlightCap()
{
    return m_bHilite;
}

// Lift in / out connection.

void CEdit::SetInsideScroll(bool bInside)
{
    m_bInsideScroll = bInside;
}

bool CEdit::GetInsideScroll()
{
    return m_bInsideScroll;
}

// Specifies whether to display the links showing the solution.

void CEdit::SetSoluceMode(bool bSoluce)
{
    m_bSoluce = bSoluce;
}

bool CEdit::GetSoluceMode()
{
    return m_bSoluce;
}

// Indicates whether the text is a defile that generic.

void CEdit::SetGenericMode(bool bGeneric)
{
    m_bGeneric = bGeneric;
}

bool CEdit::GetGenericMode()
{
    return m_bGeneric;
}


// Management of automatic indentation mode with {}.

void CEdit::SetAutoIndent(bool bMode)
{
    m_bAutoIndent = bMode;
}

bool CEdit::GetAutoIndent()
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
    m_bUndoForce = true;
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
    Scroll(rank, true);
}

// Returns the first displayed line.

int CEdit::GetFirstLine()
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

    line = GetCursorLine(cursor2);
    if ( line >= m_lineFirst+m_lineVisible )
    {
        line -= m_lineVisible-1;
        if ( line < 0 )  line = 0;
        Scroll(line, false);
    }

    line = GetCursorLine(cursor1);
    if ( line < m_lineFirst )
    {
        Scroll(line, false);
    }
}


// Management of the display mode of special characters.

void CEdit::SetDisplaySpec(bool bDisplay)
{
    m_bDisplaySpec = bDisplay;
}

bool CEdit::GetDisplaySpec()
{
    return m_bDisplaySpec;
}


// Multi-fonts mode management.

void CEdit::SetMultiFont(bool bMulti)
{
    m_format.clear();
    
    if (bMulti) {
        m_format.reserve(m_maxChar+1);
        for (int i = 0; i <= m_maxChar+1; i++) {
            m_format.push_back(0);
        }
    }
}

// TODO check if it works correctly; was checking if variable is null
bool CEdit::GetMultiFont()
{
    return ( m_format.size() > 0 );
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

    if ( m_scroll != nullptr )
    {
        value = m_scroll->GetVisibleValue();
        value *= m_lineTotal - m_lineVisible;
        Scroll(static_cast<int>(value + 0.5f), true);
    }
}

// Moves according to the visible lift.

void CEdit::Scroll(int pos, bool bAdjustCursor)
{
    int     max, line;

    m_lineFirst = pos;

    if ( m_lineFirst < 0 )  m_lineFirst = 0;

    max = m_lineTotal-m_lineVisible;
    if ( max < 0 )  max = 0;
    if ( m_lineFirst > max )  m_lineFirst = max;

    line = GetCursorLine(m_cursor1);

    if ( bAdjustCursor && m_bEdit )
    {
        // Cursor too high?
        if ( line < m_lineFirst )
        {
            MoveLine(m_lineFirst-line, false, false);
            return;
        }

        // Cursor too low?
        if ( line >= m_lineFirst+m_lineVisible )
        {
            MoveLine(m_lineFirst+m_lineVisible-line-1, false, false);
            return;
        }
    }

    Justif();
}

// Moves the cursor to the beginning of the line.

void CEdit::MoveHome(bool bWord, bool bSelect)
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

    m_bUndoForce = true;
    Justif();
    ColumnFix();
}

// Moves the cursor to the end of the line.

void CEdit::MoveEnd(bool bWord, bool bSelect)
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

    m_bUndoForce = true;
    Justif();
    ColumnFix();
}

// Moves the cursor through characters.

void CEdit::MoveChar(int move, bool bWord, bool bSelect)
{
    int     character;

    if ( move == -1 )  // back?
    {
        if ( bWord )
        {
            while ( m_cursor1 > 0 )
            {
                character = static_cast<unsigned char>(m_text[m_cursor1-1]);
                if ( !IsSpace(character) )  break;
                m_cursor1 --;
            }

            if ( m_cursor1 > 0 )
            {
                character = static_cast<unsigned char>(m_text[m_cursor1-1]);
                if ( IsSpace(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1-1]);
                        if ( !IsSpace(character) )  break;
                        m_cursor1 --;
                    }
                }
                else if ( IsWord(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1-1]);
                        if ( !IsWord(character) )  break;
                        m_cursor1 --;
                    }
                }
                else if ( IsSep(character) )
                {
                    while ( m_cursor1 > 0 )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1-1]);
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
                character = static_cast<unsigned char>(m_text[m_cursor1]);
                if ( IsSpace(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1]);
                        if ( !IsSpace(character) )  break;
                        m_cursor1 ++;
                    }
                }
                else if ( IsWord(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1]);
                        if ( !IsWord(character) )  break;
                        m_cursor1 ++;
                    }
                }
                else if ( IsSep(character) )
                {
                    while ( m_cursor1 < m_len )
                    {
                        character = static_cast<unsigned char>(m_text[m_cursor1]);
                        if ( !IsSep(character) )  break;
                        m_cursor1 ++;
                    }
                }
            }

            while ( m_cursor1 < m_len )
            {
                character = static_cast<unsigned char>(m_text[m_cursor1]);
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

    m_bUndoForce = true;
    Justif();
    ColumnFix();
}

// Moves the cursor lines.

void CEdit::MoveLine(int move, bool bWord, bool bSelect)
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

    line = GetCursorLine(m_cursor1);

    column = m_column;
    if ( m_bAutoIndent )
    {
        indentLength = m_engine->GetText()->GetCharWidth(static_cast<Gfx::UTF8Char>(' '), m_fontType, m_fontSize, 0.0f)
                        * m_engine->GetEditIndentValue();
        column -= indentLength*m_lineIndent[line];
    }

    if ( m_format.size() == 0 )
    {
        c = m_engine->GetText()->Detect(std::string(m_text+m_lineOffset[line]),
                                        m_fontType, m_fontSize,
                                        m_lineOffset[line+1]-m_lineOffset[line]);
    }
    else
    {
        c = m_engine->GetText()->Detect(std::string(m_text+m_lineOffset[line]),
                                        m_format.begin() + m_lineOffset[line],
                                        m_format.end(),
                                        m_fontSize,
                                        m_lineOffset[line+1]-m_lineOffset[line]);
    }

    m_cursor1 = m_lineOffset[line]+c;
    if ( !bSelect )  m_cursor2 = m_cursor1;

    m_bUndoForce = true;
    Justif();
}

// Sets the horizontal position.

void CEdit::ColumnFix()
{
    float   indentLength;
    int     line;

    line = GetCursorLine(m_cursor1);

    if ( m_format.size() == 0 )
    {
        m_column = m_engine->GetText()->GetStringWidth(
                                std::string(m_text+m_lineOffset[line]),
                                m_fontType, m_fontSize);
    }
    else
    {
        m_column = m_engine->GetText()->GetStringWidth(
                                std::string(m_text+m_lineOffset[line]),
                                m_format.begin() + m_lineOffset[line],
                                m_format.end(),
                                m_fontSize
                            );
    }

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->GetText()->GetCharWidth(static_cast<Gfx::UTF8Char>(' '), m_fontType, m_fontSize, 0.0f)
                        * m_engine->GetEditIndentValue();
        m_column += indentLength*m_lineIndent[line];
    }
}


// Cut the selected characters or entire line.

bool CEdit::Cut() // TODO MS Windows allocations
{
  /*  HGLOBAL hg;
    char*   text;
    char    c;
    int     c1, c2, start, len, i, j;

    if ( !m_bEdit )  return false;

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Math::Swap(c1, c2);  // always c1 <= c2

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

    if ( c1 == c2 )  return false;

    start = c1;
    len   = c2-c1;

    if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
    {
        return false;
    }
    if ( !(text = (char*)GlobalLock(hg)) )
    {
        GlobalFree(hg);
        return false;
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
        return false;
    }
    if ( !EmptyClipboard() )
    {
        GlobalFree(hg);
        return false;
    }
    if ( !SetClipboardData(CF_TEXT, hg) )
    {
        GlobalFree(hg);
        return false;
    }
    CloseClipboard();

    UndoMemorize(OPERUNDO_SPEC);
    m_cursor1 = c1;
    m_cursor2 = c2;
    DeleteOne(0);  // deletes the selected characters
    Justif();
    ColumnFix();
    SendModifEvent();*/
    return true;
}

// Copy the selected characters or entire line.

bool CEdit::Copy() // TODO
{
 /*   HGLOBAL hg;
    char*   text;
    char    c;
    int     c1, c2, start, len, i, j;

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Math::Swap(c1, c2);  // always c1 <= c2

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

    if ( c1 == c2 )  return false;

    start = c1;
    len   = c2-c1;

    if ( !(hg = GlobalAlloc(GMEM_DDESHARE, len*2+1)) )
    {
        return false;
    }
    if ( !(text = (char*)GlobalLock(hg)) )
    {
        GlobalFree(hg);
        return false;
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
        return false;
    }
    if ( !EmptyClipboard() )
    {
        GlobalFree(hg);
        return false;
    }
    if ( !SetClipboardData(CF_TEXT, hg) )
    {
        GlobalFree(hg);
        return false;
    }
    CloseClipboard();
*/
    return true;
}

// Paste the contents of the notebook.

bool CEdit::Paste() // TODO
{
    /*HANDLE  h;
    char    c;
    char*   p;

    if ( !m_bEdit )  return false;

    if ( !OpenClipboard(NULL) )
    {
        return false;
    }

    if ( !(h = GetClipboardData(CF_TEXT)) )
    {
        CloseClipboard();
        return false;
    }

    if ( !(p = (char*)GlobalLock(h)) )
    {
        CloseClipboard();
        return false;
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
    SendModifEvent();*/
    return true;
}


// Cancels the last action.

bool CEdit::Undo()
{
    if ( !m_bEdit )  return false;

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
            MoveChar(-1, false, false);
            MoveChar(-1, false, false);
        }
#if 0
        else if ( character == '(' )
        {
            InsertOne(character);
            InsertOne(')');
            MoveChar(-1, false, false);
        }
        else if ( character == '[' )
        {
            InsertOne(character);
            InsertOne(']');
            MoveChar(-1, false, false);
        }
#endif
        else if ( character == '\t' )
        {
            for ( i=0 ; i<m_engine->GetEditIndentValue() ; i++ )
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

        if ( m_format.size() > static_cast<unsigned int>(i) )
        {
            m_format[i] = m_format[i-1];  // shoot
        }
    }

    m_len ++;

    m_text[m_cursor1] = character;

    if ( m_format.size() > static_cast<unsigned int>(m_cursor1) )
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

    if ( m_cursor1 > m_cursor2 )  Math::Swap(m_cursor1, m_cursor2);
    hole = m_cursor2-m_cursor1;
    end = m_len-hole;
    for ( i=m_cursor1 ; i<end ; i++ )
    {
        m_text[i] = m_text[i+hole];

        if ( m_format.size() > static_cast<unsigned int>(i + hole) )
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

bool CEdit::Shift(bool bLeft)
{
    bool    bInvert = false;
    int     c1, c2, i;

    if ( m_cursor1 == m_cursor2 )  return false;

    UndoMemorize(OPERUNDO_SPEC);

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )
    {
        Math::Swap(c1, c2);  // always c1 <= c2
        bInvert = true;
    }

    if ( c1 > 0 )
    {
        if ( m_text[c1-1] != '\n' )  return false;
    }
    if ( c2 < m_len )
    {
        if ( m_text[c2-1] != '\n' )  return false;
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

    if ( bInvert )  Math::Swap(c1, c2);
    m_cursor1 = c1;
    m_cursor2 = c2;

    Justif();
    ColumnFix();
    SendModifEvent();
    return true;
}

// Math::Min conversion <-> shift the selection.

bool CEdit::MinMaj(bool bMaj)
{
    int     c1, c2, i, character;

    if ( m_cursor1 == m_cursor2 )  return false;

    UndoMemorize(OPERUNDO_SPEC);

    c1 = m_cursor1;
    c2 = m_cursor2;
    if ( c1 > c2 )  Math::Swap(c1, c2);  // alwyas c1 <= c2

    for ( i=c1 ; i<c2 ; i++ )
    {
        character = static_cast<unsigned char>(m_text[i]);
        if ( bMaj )  character = GetToUpper(character);
        else         character = GetToLower(character);
        m_text[i] = character;
    }

    Justif();
    ColumnFix();
    SendModifEvent();
    return true;
}


// Cut all text lines.

void CEdit::Justif()
{
    float   width, size, indentLength;
    int     i, j, line, indent;
    bool    bDual, bString, bRem;

    indent = 0;
    m_lineTotal = 0;
    m_lineOffset[m_lineTotal] = 0;
    m_lineIndent[m_lineTotal] = indent;
    m_lineTotal ++;

    if ( m_bAutoIndent )
    {
        indentLength = m_engine->GetText()->GetCharWidth(static_cast<Gfx::UTF8Char>(' '), m_fontType, m_fontSize, 0.0f)
                        * m_engine->GetEditIndentValue();
    }

    bString = bRem = false;
    i = 0;
    while ( true )
    {
        bDual = false;

        width = m_dim.x-(10.0f/640.0f)*2.0f-(m_bMulti?MARGX*2.0f+SCROLL_WIDTH:0.0f);
        if ( m_bAutoIndent )
        {
            width -= indentLength*m_lineIndent[m_lineTotal-1];
        }

        if ( m_format.size() == 0 )
        {
            // TODO check if good

            i += m_engine->GetText()->Justify(m_text+i, m_fontType,
                                              m_fontSize, width);
        }
        else
        {
            size = m_fontSize;

            if ( m_format.size() > static_cast<unsigned int>(i) && (m_format[i]&Gfx::FONT_MASK_TITLE) == Gfx::FONT_TITLE_BIG )  // headline?
            {
                size *= BIG_FONT;
                bDual = true;
            }

            if ( m_format.size() > static_cast<unsigned int>(i) && (m_format[i]&Gfx::FONT_MASK_IMAGE) != 0 )  // image part?
            {
                i ++;  // jumps just a character (index in m_image)
            }
            else
            {
                // TODO check if good
                i += m_engine->GetText()->Justify(std::string(m_text+i),
                                                  m_format.begin() + i,
                                                  m_format.end(),
                                                  size,
                                                  width);
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
                     m_text[j+1] == '/' )  bRem = true;
                if ( m_text[j] == '\n' )  bString = bRem = false;
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
            line = GetCursorLine(m_cursor1);
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

    UpdateScroll();

    m_timeBlink = 0.0f;  // lights the cursor immediately
}

// Returns the rank of the line where the cursor is located.

int CEdit::GetCursorLine(int cursor)
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
        m_undo[i].text = nullptr;
    }

    m_bUndoForce = true;
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

    m_bUndoForce = false;
    m_undoOper = oper;

    delete m_undo[EDITUNDOMAX-1].text;
    m_undo[EDITUNDOMAX-1].text = nullptr;

    for ( i=EDITUNDOMAX-1 ; i>=1 ; i-- )
    {
        m_undo[i] = m_undo[i-1];
    }

    len = m_len;
    if ( len == 0 )  len ++;
    m_undo[0].text = new char[len+1];
    memcpy(m_undo[0].text, m_text, m_len);
    m_undo[0].len = m_len;

    m_undo[0].cursor1 = m_cursor1;
    m_undo[0].cursor2 = m_cursor2;
    m_undo[0].lineFirst = m_lineFirst;
}

// Back to previous state.

bool CEdit::UndoRecall()
{
    int     i;

    if ( m_undo[0].text == nullptr )  return false;

    m_len = m_undo[0].len;
    memcpy(m_text, m_undo[0].text, m_len);

    m_cursor1 = m_undo[0].cursor1;
    m_cursor2 = m_undo[0].cursor2;
    m_lineFirst = m_undo[0].lineFirst;

    for ( i=0 ; i<EDITUNDOMAX-1 ; i++ )
    {
        m_undo[i] = m_undo[i+1];
    }
    m_undo[EDITUNDOMAX-1].text = nullptr;

    m_bUndoForce = true;
    Justif();
    ColumnFix();
    SendModifEvent();
    return true;
}


// Clears the format of all characters.

bool CEdit::ClearFormat()
{
    if ( m_format.size() == 0 )
    {
        SetMultiFont(true);
    }
    m_format.clear();

    return true;
}

// Changes the format of a sequence of characters.

bool CEdit::SetFormat(int cursor1, int cursor2, int format)
{
    int     i;

    if ( m_format.size() < static_cast<unsigned int>(cursor2) )
        SetMultiFont(true);

    for ( i=cursor1 ; i<cursor2 ; i++ )
    {
        m_format.at(i) |= format;
    }

    return true;
}

void CEdit::UpdateScroll()
{
    float value;
    
    if ( m_scroll != nullptr )
    {
        if ( m_lineTotal <= m_lineVisible )
        {
            m_scroll->SetVisibleRatio(1.0f);
            m_scroll->SetVisibleValue(0.0f);
            m_scroll->SetArrowStep(0.0f);
        }
        else
        {
            value = static_cast<float>(m_lineVisible) / m_lineTotal;
            m_scroll->SetVisibleRatio(value);

            value = static_cast<float>(m_lineFirst) / (m_lineTotal - m_lineVisible);
            m_scroll->SetVisibleValue(value);

            value = 1.0f / (m_lineTotal - m_lineVisible);
            m_scroll->SetArrowStep(value);
        }
    }
}

}
