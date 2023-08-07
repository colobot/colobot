/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

const int LISTMAXDISPLAY = 20;  // maximum number of visible lines



class CList : public CControl
{
public:
    CList();
    ~CList();

    bool        Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, float expand);

    void        SetPos(const glm::vec2& pos) override;
    void        SetDim(const glm::vec2& dim) override;

    bool        SetState(int state, bool bState) override;
    bool        SetState(int state) override;
    bool        ClearState(int state) override;

    bool        EventProcess(const Event &event) override;
    void        Draw() override;

    void        Flush();

    void        SetTotal(int i);
    int         GetTotal();

    void        SetSelect(int i);
    int         GetSelect();

    void        SetSelectCap(bool bEnable);
    bool        GetSelectCap();

    void        SetBlink(bool bEnable);
    bool        GetBlink();

    void        SetItemName(int i, const std::string& name);
    const std::string& GetItemName(int i);

    void        SetCheck(int i, bool bMode);
    bool        GetCheck(int i);

    void        SetEnable(int i, bool enable);
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
    void        DrawCase(const char* text, const glm::vec2& pos, float width, Gfx::TextAlign justif);

private:
    // Overridden to avoid warning about hiding the virtual function
    bool        Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventType) override;

protected:
    std::array<std::unique_ptr<CButton>, LISTMAXDISPLAY> m_buttons;
    std::unique_ptr<CScroll> m_scroll;

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

    struct Item
    {
        std::string text = "";
        bool check = false;
        bool enable = true;
    };
    std::vector<Item> m_items;
};


} // namespace Ui
