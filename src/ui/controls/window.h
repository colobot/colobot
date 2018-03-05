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

#include "graphics/engine/engine.h" // TODO: only needed for EngineMouseType

#include <memory>
#include <string>
#include <vector>

namespace Ui
{

class CButton;
class CColor;
class CCheck;
class CKey;
class CGroup;
class CImage;
class CLabel;
class CEdit;
class CEditValue;
class CScroll;
class CSlider;
class CEnumSlider;
class CList;
class CShortcut;
class CMap;
class CGauge;
class CTarget;

class CWindow : public CControl
{
public:
    CWindow();
    ~CWindow();

    void        Flush();
    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg) override;
    CButton*    CreateButton(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CColor*     CreateColor(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CCheck*     CreateCheck(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CKey*       CreateKey(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CGroup*     CreateGroup(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CImage*     CreateImage(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CLabel*     CreateLabel(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, std::string name);
    CEdit*      CreateEdit(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CEditValue* CreateEditValue(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CScroll*    CreateScroll(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CSlider*    CreateSlider(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CEnumSlider* CreateEnumSlider(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CList*      CreateList(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand=1.2f);
    CShortcut*  CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CMap*       CreateMap(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CGauge*     CreateGauge(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CTarget*    CreateTarget(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    bool        DeleteControl(EventType eventMsg);
    CControl*   SearchControl(EventType eventMsg);

    EventType    GetEventTypeReduce();
    EventType    GetEventTypeFull();
    EventType    GetEventTypeClose();

    void SetName(std::string name, const bool tooltip = true) override;

    void        SetTrashEvent(bool bTrash);
    bool        GetTrashEvent();

    void        SetPos(Math::Point pos) override;
    void        SetDim(Math::Point dim) override;

    void        SetMinDim(Math::Point dim);
    void        SetMaxDim(Math::Point dim);
    Math::Point     GetMinDim();
    Math::Point     GetMaxDim();

    void        SetMovable(bool bMode);
    bool        GetMovable();

    void        SetRedim(bool bMode);
    bool        GetRedim();

    void        SetClosable(bool bMode);
    bool        GetClosable();

    void        SetMaximized(bool bMaxi);
    bool        GetMaximized();
    void        SetMinimized(bool bMini);
    bool        GetMinimized();
    void        SetFixed(bool bFix);
    bool        GetFixed();

    bool        GetTooltip(Math::Point pos, std::string &name)const override;

    bool        EventProcess(const Event &event) override;

    void        Draw() override;

    void        SetFocus(CControl* focusControl) override;

protected:
    int         BorderDetect(Math::Point pos);
    void        AdjustButtons();
    void        MoveAdjust();
    void        DrawVertex(Math::Point pos, Math::Point dim, int icon);
    void        DrawHach(Math::Point pos, Math::Point dim);
    template<typename ControlClass>
    ControlClass* CreateControl(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);

protected:
    std::vector<std::unique_ptr<CControl>> m_controls;

    bool        m_bTrashEvent;
    bool        m_bMaximized;
    bool        m_bMinimized;
    bool        m_bFixed;

    Math::Point     m_minDim;
    Math::Point     m_maxDim;

    std::unique_ptr<CButton> m_buttonReduce;
    std::unique_ptr<CButton> m_buttonFull;
    std::unique_ptr<CButton> m_buttonClose;

    bool        m_bMovable;
    bool        m_bRedim;
    bool        m_bClosable;
    bool        m_bCapture;
    Math::Point     m_pressPos;
    int         m_pressFlags;
    Gfx::EngineMouseType    m_pressMouse;
};


} // namespace Ui
