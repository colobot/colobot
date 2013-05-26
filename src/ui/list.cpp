// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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


#include "ui/list.h"

#include <string.h>

namespace Ui {

const float MARGING = 4.0f;


// Object's constructor.

CList::CList() : CControl()
{
    for (int i = 0; i < LISTMAXDISPLAY; i++)
        m_button[i] = nullptr;

    m_scroll = nullptr;
    for (int i = 0; i < LISTMAXTOTAL; i++)
    {
        m_text[i][0] = 0;
        m_check[i] = false;
        m_enable[i] = true;
    }

    for (int i = 0; i < 10; i++)
    {
        m_tabs[i] = 0.0f;
        m_justifs[i] = Gfx::TEXT_ALIGN_LEFT;
    }

    m_totalLine = 0;
    m_displayLine = 0;
    m_selectLine = -1;
    m_firstLine = 0;
    m_bBlink = false;
    m_bSelectCap = true;
    m_blinkTime = 0.0f;
}

// Object's destructor.

CList::~CList()
{
    for (int i = 0; i < LISTMAXDISPLAY; i++)
    {
        if (m_button[i] != nullptr)
            delete m_button[i];
    }

    if (m_scroll != nullptr)
        delete m_scroll;
}


// Creates a new list.
// if expand is less then zero, then the list would try to use expand's absolute value,
// and try to scale items to some size, so that dim of the list would not change after
// adjusting

bool CList::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand)
{
    m_expand = expand;

    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);

    m_scroll = new CScroll();
    m_scroll->Create(pos, dim, 0, EVENT_NULL);
    m_eventScroll = m_scroll->GetEventType();

    return MoveAdjust();
}

// Should never be called
bool CList::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    assert(false);
    return false;
}


// Adjusted after a change of dimensions.

bool CList::MoveAdjust()
{
    Math::Point ipos, idim, ppos, ddim;
    float marging, h;

    for (int i = 0; i < LISTMAXDISPLAY; i++)
    {
        if (m_button[i] != nullptr)
        {
            delete m_button[i];
            m_button[i] = nullptr;
        }
    }

    if (m_icon == 0)
        marging = MARGING;
    else
        marging = 0.0f;

    ipos.x = m_pos.x + marging / 640.f;
    ipos.y = m_pos.y + marging / 480.f;
    idim.x = m_dim.x - marging * 2.0f / 640.f;
    idim.y = m_dim.y - marging * 2.0f / 480.f;

    //If m_expand is less then zero, then try to apply it's absolute value
    h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize) * ((m_expand < 0) ? -m_expand : m_expand);
    m_displayLine = static_cast<int>(idim.y / h);

    if (m_displayLine == 0)
        return false;
    if (m_displayLine > LISTMAXDISPLAY)
        m_displayLine = LISTMAXDISPLAY;

    // Stretch lines to fill whole area of a list, if needed
    if (m_expand < 0 && (idim.y - (h * m_displayLine) < h))
    {
        h = idim.y / m_displayLine;
    }

    idim.y = h * m_displayLine; //Here cuts list size if height of shown elements is less then designed height
    m_dim.y = idim.y + marging * 2.0f / 480.f;

    ppos.x = ipos.x;
    ppos.y = ipos.y + idim.y - h;
    ddim.x = idim.x - SCROLL_WIDTH;
    ddim.y = h;
    for (int i = 0; i < m_displayLine; i++)
    {
        m_button[i] = new CButton();
        m_button[i]->Create(ppos, ddim, -1, EVENT_NULL);
        m_button[i]->SetTextAlign(Gfx::TEXT_ALIGN_LEFT);
        m_button[i]->SetState(STATE_SIMPLY);
        m_button[i]->SetFontType(m_fontType);
        m_button[i]->SetFontSize(m_fontSize);
        ppos.y -= h;

        m_eventButton[i] = m_button[i]->GetEventType();
    }

    if ( m_scroll != nullptr )
    {
        ppos.x = ipos.x + idim.x - SCROLL_WIDTH;
        ppos.y = ipos.y;
        ddim.x = SCROLL_WIDTH;
        ddim.y = idim.y;
        m_scroll->SetPos(ppos);
        m_scroll->SetDim(ddim);
    }

    UpdateScroll();
    UpdateButton();
    return true;
}


// Returns the message of a button.

EventType CList::GetEventMsgButton(int i)
{
    if (i < 0 || i >= m_displayLine)
        return EVENT_NULL;
    if (m_button[i] == nullptr)
        return EVENT_NULL;
    return m_button[i]->GetEventType();
}

// Returns the message from the elevator.

EventType CList::GetEventMsgScroll()
{
    if (m_scroll == nullptr)
        return EVENT_NULL;
    return m_scroll->GetEventType();
}


void CList::SetPos(Math::Point pos)
{
    CControl::SetPos(pos);
}


void CList::SetDim(Math::Point dim)
{
    m_dim = dim;
    MoveAdjust();
    CControl::SetDim(dim);
}


bool CList::SetState(int state, bool bState)
{
    if (state & STATE_ENABLE)
    {
        for (int i = 0; i < m_displayLine; i++)
        {
            if (m_button[i] != nullptr)
                m_button[i]->SetState(state, bState);
        }
        if (m_scroll != nullptr)
            m_scroll->SetState(state, bState);
    }

    return CControl::SetState(state, bState);
}


bool CList::SetState(int state)
{
    if (state & STATE_ENABLE)
    {
        for (int i = 0; i < m_displayLine; i++)
        {
            if (m_button[i] != nullptr)
                m_button[i]->SetState(state);
        }
        if (m_scroll != nullptr)
            m_scroll->SetState(state);
    }

    return CControl::SetState(state);
}


bool CList::ClearState(int state)
{
    if (state & STATE_ENABLE)
    {
        for (int i = 0; i < m_displayLine; i++)
        {
            if (m_button[i] != nullptr)
                m_button[i]->ClearState(state);
        }
        if (m_scroll != nullptr)
            m_scroll->ClearState(state);
    }

    return CControl::ClearState(state);
}


// Management of an event.

bool CList::EventProcess(const Event &event)
{
    int i;
    if (m_bBlink && event.type == EVENT_FRAME)
    {
        i = m_selectLine-m_firstLine;

        if (i >= 0 && i < 4  && m_button[i] != nullptr)
        {
            m_blinkTime += event.rTime;
            if (Math::Mod(m_blinkTime, 0.7f) < 0.3f)
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

    if ((m_state & STATE_VISIBLE) == 0)
        return true;
    if ((m_state & STATE_ENABLE) == 0)
        return true;

    if (event.type == EVENT_MOUSE_WHEEL && event.mouseWheel.dir == WHEEL_UP && Detect(event.mousePos))
    {
        if (m_firstLine > 0)
            m_firstLine--;
        UpdateScroll();
        UpdateButton();
        return true;
    }

    if (event.type == EVENT_MOUSE_WHEEL && event.mouseWheel.dir == WHEEL_DOWN && Detect(event.mousePos))
    {
        if (m_firstLine < m_totalLine - m_displayLine)
            m_firstLine++;
        UpdateScroll();
        UpdateButton();
        return true;
    }

    CControl::EventProcess(event);

    if (event.type == EVENT_MOUSE_MOVE && Detect(event.mousePos))
    {
        m_engine->SetMouseType(Gfx::ENG_MOUSE_NORM);
        for (i = 0; i < m_displayLine; i++)
        {
            if (i + m_firstLine >= m_totalLine)
                break;
            if (m_button[i] != nullptr)
                m_button[i]->EventProcess(event);
        }
    }

    if (m_bSelectCap)
    {
        for (i = 0; i < m_displayLine; i++)
        {
            if (i + m_firstLine >= m_totalLine)
                break;

            if (m_button[i] != nullptr)
            {
                if (!m_button[i]->EventProcess(event))
                    return false;

                if (event.type == m_eventButton[i])
                {
                    SetSelect(m_firstLine + i);

                    Event newEvent = event;
                    newEvent.type = m_eventType;
                    m_event->AddEvent(newEvent);  // selected line changes
                }
            }
        }
    }

    if (m_scroll != nullptr)
    {
        if (!m_scroll->EventProcess(event))
            return false;

        if (event.type == m_eventScroll)
        {
            MoveScroll();
            UpdateButton();
        }
    }

    return true;
}


// Draws the list.

void CList::Draw()
{
    Math::Point uv1, uv2, corner, pos, dim, ppos, ddim;
    float   dp;
    int     i;
    char    text[100];
    char    *pb, *pe;

    if ((m_state & STATE_VISIBLE) == 0)
        return;

    if (m_state & STATE_SHADOW)
        DrawShadow(m_pos, m_dim);

    dp = 0.5f / 256.0f;

    if (m_icon != -1)
    {
        dim = m_dim;

        if (m_icon == 0)
        {
            m_engine->SetTexture("button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

            uv1.x = 128.0f / 256.0f;
            uv1.y =  64.0f / 256.0f;  // u-v texture
            uv2.x = 160.0f / 256.0f;
            uv2.y =  96.0f / 256.0f;
        }
        else
        {
            m_engine->SetTexture("button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

            uv1.x = 132.0f / 256.0f;
            uv1.y =  68.0f / 256.0f;  // u-v texture
            uv2.x = 156.0f / 256.0f;
            uv2.y =  92.0f / 256.0f;

            if (m_button[0] != nullptr)
            {
                dim = m_button[0]->GetDim();
                dim.y *= m_displayLine;  // background sounds spot behind
            }
        }

        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        corner.x = 10.0f / 640.0f;
        corner.y = 10.0f / 480.0f;
        DrawIcon(m_pos, dim, uv1, uv2, corner, 8.0f / 256.0f);
    }

    if ( m_totalLine < m_displayLine ) // no buttons to the bottom?
    {
        i = m_totalLine;
        if ( m_button[i] != 0 )
        {
            pos = m_button[i]->GetPos();
            dim = m_button[i]->GetDim();
            pos.y += dim.y * 1.1f;
            dim.y *= 0.4f;
            pos.y -= dim.y;

            m_engine->SetTexture("button2.png");
            m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE); // was D3DSTATETTw
            uv1.x = 120.0f / 256.0f;
            uv1.y =  64.0f / 256.0f;
            uv2.x = 128.0f / 256.0f;
            uv2.y =  48.0f / 256.0f;
            uv1.x += dp;
            uv1.y -= dp;
            uv2.x -= dp;
            uv2.y += dp;
            DrawIcon(pos, dim, uv1, uv2);  // ch'tite shadow cute  (?)
        }
    }

    for (i = 0; i < m_displayLine; i++)
    {
        if ( i + m_firstLine >= m_totalLine )
            break;

        if ( m_button[i] != nullptr )
        {
            if ( !m_bBlink && i + m_firstLine < m_totalLine )
                m_button[i]->SetState(STATE_ENABLE, m_enable[i+m_firstLine] && (m_state & STATE_ENABLE) );

            m_button[i]->Draw();  // draws a box without text

            // draws text in the box
            pos = m_button[i]->GetPos();
            dim = m_button[i]->GetDim();
            if ( m_tabs[0] == 0.0f )
            {
                ppos.x = pos.x + dim.y * 0.5f;
                ppos.y = pos.y + dim.y * 0.5f;
                ppos.y -= m_engine->GetText()->GetHeight(m_fontType, m_fontSize) / 2.0f;
                ddim.x = dim.x-dim.y;
                DrawCase(m_text[i + m_firstLine], ppos, ddim.x, Gfx::TEXT_ALIGN_LEFT);
            }
            else
            {
                ppos.x = pos.x + dim.y * 0.5f;
                ppos.y = pos.y + dim.y * 0.5f;
                ppos.y -= m_engine->GetText()->GetHeight(m_fontType, m_fontSize) / 2.0f;
                pb = m_text[i + m_firstLine];
                for (int j = 0; j < 10; j++)
                {
                    pe = strchr(pb, '\t');
                    if ( pe == 0 )
                        strcpy(text, pb);
                    else
                    {
                        strncpy(text, pb, pe - pb);
                        text[pe - pb] = 0;
                    }
                    DrawCase(text, ppos, m_tabs[j], m_justifs[j]);

                    if ( pe == 0 )
                        break;
                    ppos.x += m_tabs[j];
                    pb = pe + 1;
                }
            }

            if ( (m_state & STATE_EXTEND) && i < m_totalLine)
            {
                pos = m_button[i]->GetPos();
                dim = m_button[i]->GetDim();
                pos.x += dim.x - dim.y * 0.75f;
                dim.x = dim.y * 0.75f;
                pos.x += 2.0f / 640.0f;
                pos.y += 2.0f / 480.0f;
                dim.x -= 4.0f / 640.0f;
                dim.y -= 4.0f / 480.0f;

                if ( m_check[i + m_firstLine] )
                {
                    m_engine->SetTexture("button1.png");
                    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
                    uv1.x = 64.0f / 256.0f;
                    uv1.y =  0.0f / 256.0f;
                    uv2.x = 96.0f / 256.0f;
                    uv2.y = 32.0f / 256.0f;
                    uv1.x += dp;
                    uv1.y += dp;
                    uv2.x -= dp;
                    uv2.y -= dp;
                    DrawIcon(pos, dim, uv1, uv2);  // square shape

                    m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE); // was D3DSTATETTw
                    uv1.x =  0.0f / 256.0f;  // v
                    uv1.y = 64.0f / 256.0f;
                    uv2.x = 32.0f / 256.0f;
                    uv2.y = 96.0f / 256.0f;
                    uv1.x += dp;
                    uv1.y += dp;
                    uv2.x -= dp;
                    uv2.y -= dp;
                    DrawIcon(pos, dim, uv1, uv2);  // draws v
                }
                else
                {
                    m_engine->SetTexture("button1.png");
                    m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE); // was D3DSTATETTw
                    if ( i + m_firstLine == m_selectLine )
                    {
                        uv1.x =224.0f / 256.0f;  // <
                        uv1.y =192.0f / 256.0f;
                        uv2.x =256.0f / 256.0f;
                        uv2.y =224.0f / 256.0f;
                    }
                    else
                    {
                        uv1.x = 96.0f / 256.0f;  // x
                        uv1.y = 32.0f / 256.0f;
                        uv2.x =128.0f / 256.0f;
                        uv2.y = 64.0f / 256.0f;
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
        m_scroll->Draw();  // draws the lift
}

// Displays text in a box.

void CList::DrawCase(char *text, Math::Point pos, float width, Gfx::TextAlign justif)
{
    if (justif == Gfx::TEXT_ALIGN_CENTER)
        pos.x += width / 2.0f;
    else if (justif == Gfx::TEXT_ALIGN_RIGHT)
        pos.x += width;
    m_engine->GetText()->DrawText(std::string(text), m_fontType, m_fontSize, pos, width, justif, 0);
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

int CList::GetTotal()
{
    return m_totalLine;
}


// Selects a line.

void CList::SetSelect(int i)
{
    if ( m_bSelectCap )
        m_selectLine = i;
    else
    {
        m_firstLine = i;
        UpdateScroll();
    }

    UpdateButton();
}

// Returns the selected line.

int CList::GetSelect()
{
    if ( m_bSelectCap )
        return m_selectLine;
    else
        return m_firstLine;
}


// Management of capability has a select box.

void CList::SetSelectCap(bool bEnable)
{
    m_bSelectCap = bEnable;
}

bool CList::GetSelectCap()
{
    return m_bSelectCap;
}


// Blink a line.

void CList::SetBlink(bool bEnable)
{
    int i;

    m_bBlink = bEnable;
    m_blinkTime = 0.0f;

    i = m_selectLine-m_firstLine;

    if (i >= 0 && i < 4  &&  m_button[i] != nullptr)
    {
        if ( !bEnable )
        {
            m_button[i]->SetState(STATE_CHECK);
            m_button[i]->ClearState(STATE_ENABLE);
        }
    }
}

bool CList::GetBlink()
{
    return m_bBlink;
}


// Specifies the text of a line.

void CList::SetItemName(int i, const char* name)
{
    if ( i < 0 || i >= LISTMAXTOTAL )
        return;

    if ( i >= m_totalLine )
        m_totalLine = i+1;  // expands the list

    if ( name[0] == 0 )
        strcpy(m_text[i], " ");
    else
        strcpy(m_text[i], name);

    UpdateButton();
    UpdateScroll();
}

// Returns the text of a line.

char* CList::GetItemName(int i)
{
    if ( i < 0 || i >= m_totalLine )
        return 0;

    return m_text[i];
}


// Specifies the bit "check" for a box.

void CList::SetCheck(int i, bool bMode)
{
    if ( i < 0 || i >= m_totalLine )
        return;

    m_check[i] = bMode;
}

// Returns the bit "check" for a box.

bool CList::GetCheck(int i)
{
    if ( i < 0 || i >= m_totalLine )
        return false;

    return m_check[i];
}


// Specifies the bit "enable" for a box.

void CList::SetEnable(int i, bool bMode)
{
    if ( i < 0 || i >= m_totalLine )
        return;

    m_enable[i] = bMode;
}

// Returns the bit "enable" for a box.

bool CList::GetEnable(int i)
{
    if ( i < 0 || i >= m_totalLine )
        return false;

    return m_enable[i];
}


// Management of the position of the tabs.

void CList::SetTabs(int i, float pos, Gfx::TextAlign justif)
{
    if ( i < 0 || i >= 10 )
        return;
    m_tabs[i] = pos;
    m_justifs[i] = justif;
}

float  CList::GetTabs(int i)
{
    if ( i < 0 || i >= 10 )
        return 0.0f;
    return m_tabs[i];
}


// Moves the lift to see the list of the selected line.

void CList::ShowSelect(bool bFixed)
{
    int     sel;

    if ( bFixed && m_selectLine >= m_firstLine && m_selectLine <  m_firstLine+m_displayLine )
        return;  // all good

    sel = m_selectLine;

    // Down from 1/2 * h.
    sel += m_displayLine / 2;
    if ( sel > m_totalLine - 1 )
        sel = m_totalLine - 1;

    // Back to h-1.
    sel -= m_displayLine - 1;
    if ( sel < 0 )
        sel = 0;

    m_firstLine = sel;

    UpdateButton();
    UpdateScroll();
}


// Updates all button names.

void CList::UpdateButton()
{
    int state, i, j;

    state = CControl::GetState();

    j = m_firstLine;
    for (i = 0; i < m_displayLine; i++)
    {
        if (m_button[i] == nullptr)
            continue;

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
    float ratio, value, step;

    if (m_scroll == nullptr)
        return;

    if (m_totalLine <= m_displayLine)
    {
        ratio = 1.0f;
        value = 0.0f;
        step = 0.0f;
    }
    else
    {
        ratio = static_cast<float>(m_displayLine) / m_totalLine;
        if ( ratio > 1.0f )  ratio = 1.0f;

        value = static_cast<float>(m_firstLine) / (m_totalLine - m_displayLine);
        if ( value < 0.0f )
            value = 0.0f;
        if ( value > 1.0f )
            value = 1.0f;

        step = static_cast<float>(1.0f)/ (m_totalLine - m_displayLine);
        if ( step < 0.0f )
            step = 0.0f;
    }

    m_scroll->SetVisibleRatio(ratio);
    m_scroll->SetVisibleValue(value);
    m_scroll->SetArrowStep(step);
}

// Update when the lift is moving.

void CList::MoveScroll()
{
    float pos;
    int n;

    if ( m_scroll == 0 )
        return;

    n = m_totalLine - m_displayLine;
    pos = m_scroll->GetVisibleValue();
    pos += m_scroll->GetArrowStep() / 2.0f;  // it's magic!
    m_firstLine = static_cast<int>(pos * n);
    if ( m_firstLine < 0 )
        m_firstLine = 0;
    if ( m_firstLine > n )
        m_firstLine = n;
}


} // namespace Ui

