/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "ui/controls/control.h"

#include "common/event.h"

#include "graphics/engine/text.h"

#include "ui/controls/button.h"
#include "ui/controls/scroll.h"

#include <array>
#include <memory>

namespace Ui
{

const short LISTMAXDISPLAY = 20;  // maximum number of visible lines



class CList : public CControl
{
public:
    CList();
    ~CList();

    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand);

    void        SetPos(Math::Point pos) override;
    void        SetDim(Math::Point dim) override;

    bool        SetState(int state, bool bState) override;
    bool        SetState(int state) override;
    bool        ClearState(int state) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void        Flush();

    void        SetTotal(const short i);
    short         GetTotal()const;

    void        SetSelect(const short i);
    short         GetSelect()const;

    void        SetSelectCap(const bool bEnable);
    bool        GetSelectCap()const;

    void        SetBlink(const bool bEnable);
    bool        GetBlink()const;

    void        SetKeyCtrl(const bool bEnable)  {m_bKeyCtrl=bEnable;}
    bool        GetKeyCtrl()const               {return m_bKeyCtrl;}

    void        SetItemName(const short i, const std::string& name);
    const std::string& GetItemName(const short i)const;

    void        SetCheck(const short i, const bool bMode);
    bool        GetCheck(const short i)const;

    void        SetEnable(const short i, const bool enable);
    bool        GetEnable(const short i)const;

    void        SetTabs(const short i, const float pos, const Gfx::TextAlign justif=Gfx::TEXT_ALIGN_LEFT);
    float       GetTabs(const short i)const;

    void        ShowSelect(const bool bFixed);

    EventType    GetEventMsgButton(const short i)const;
    EventType    GetEventMsgScroll()const;

protected:
    bool        MoveAdjust();
    void        UpdateButton();
    void        UpdateScroll();
    void        MoveScroll();
    void        DrawCase(const char* text, Math::Point pos, float width, Gfx::TextAlign justif);

private:
    // Overridden to avoid warning about hiding the virtual function
    bool Create(Math::Point pos, Math::Point dim, int icon, EventType eventType) override;

protected:
    std::array<std::unique_ptr<CButton>, LISTMAXDISPLAY> m_buttons;
    std::unique_ptr<CScroll> m_scroll;

    float       m_expand;
    short         m_totalLine;  // total number of lines
    short         m_displayLine;// number of visible lines
    short         m_selectLine; // selected line
    short         m_firstLine;  // first visible line
    bool        m_bBlink;       // selected blink into 4 1st elts (FIXME?)
    bool        m_bSelectCap;   //capability has a select box
    bool        m_bKeyCtrl;     //manage arrows & pg-up/pg-down & beg/end
                                // Nota : better one max per windows!
    float       m_blinkTime;    // period used by m_bBlink
    float       m_tabs[10];
    Gfx::TextAlign m_justifs[10];

    struct Item
    {
        std::string text = "";
        bool check = false;
        bool enable = true;
    };
    std::vector<Item> m_items;
};


} // namespace Ui
