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

// list.h

#pragma once


#include "ui/control.h"
#include "ui/button.h"
#include "ui/scroll.h"

#include "common/event.h"
#include "common/misc.h"

#include "graphics/engine/text.h"


namespace Ui {

const int LISTMAXDISPLAY = 20;  // maximum number of visible lines
const int LISTMAXTOTAL   = 100; // maximum total number of lines



class CList : public CControl
{
    public:
        CList();
        ~CList();

        bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand);

        void        SetPos(Math::Point pos);
        void        SetDim(Math::Point dim);

        bool        SetState(int state, bool bState);
        bool        SetState(int state);
        bool        ClearState(int state);

        bool        EventProcess(const Event &event);
        void        Draw();

        void        Flush();

        void        SetTotal(int i);
        int         GetTotal();

        void        SetSelect(int i);
        int         GetSelect();

        void        SetSelectCap(bool bEnable);
        bool        GetSelectCap();

        void        SetBlink(bool bEnable);
        bool        GetBlink();

        void        SetItemName(int i, const char* name);
        char*       GetItemName(int i);

        void        SetCheck(int i, bool bMode);
        bool        GetCheck(int i);

        void        SetEnable(int i, bool bEnable);
        bool        GetEnable(int i);

        void        SetTabs(int i, float pos, Gfx::TextAlign justif=Gfx::TEXT_ALIGN_LEFT);
        float       GetTabs(int i);

        void        ShowSelect(bool bFixed);

        EventType    GetEventMsgButton(int i);
        EventType    GetEventMsgScroll();

    protected:
        bool        MoveAdjust();
        void        UpdateButton();
        void        UpdateScroll();
        void        MoveScroll();
        void        DrawCase(char *text, Math::Point pos, float width, Gfx::TextAlign justif);

    private:
        // Overridden to avoid warning about hiding the virtual function
        virtual bool Create(Math::Point pos, Math::Point dim, int icon, EventType eventType) override;

    protected:
        CButton*    m_button[LISTMAXDISPLAY];
        CScroll*    m_scroll;

        EventType    m_eventButton[LISTMAXDISPLAY];
        EventType    m_eventScroll;

        float       m_expand;
        int         m_totalLine;    // total number of lines
        int         m_displayLine;  // number of visible lines
        int         m_selectLine;   // selected line
        int         m_firstLine;    // first visible line
        bool        m_bBlink;
        bool        m_bSelectCap;
        float       m_blinkTime;
        float       m_tabs[10];
        Gfx::TextAlign m_justifs[10];

        char        m_text[LISTMAXTOTAL][100];
        char        m_check[LISTMAXTOTAL];
        char        m_enable[LISTMAXTOTAL];
};


} // namespace Ui

