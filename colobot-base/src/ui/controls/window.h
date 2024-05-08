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

#include <memory>
#include <string>
#include <vector>

namespace Gfx
{
enum EngineMouseType : unsigned char;
}

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
    bool        Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg) override;
    CButton*    CreateButton(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CColor*     CreateColor(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CCheck*     CreateCheck(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CKey*       CreateKey(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CGroup*     CreateGroup(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CImage*     CreateImage(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CLabel*     CreateLabel(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, std::string name);
    CLabel*     CreateLabelRaw(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, std::string name);
    CEdit*      CreateEdit(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CEditValue* CreateEditValue(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CScroll*    CreateScroll(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CSlider*    CreateSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CEnumSlider* CreateEnumSlider(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CList*      CreateList(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg, float expand=1.2f);
    CShortcut*  CreateShortcut(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CMap*       CreateMap(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CGauge*     CreateGauge(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    CTarget*    CreateTarget(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);
    bool        DeleteControl(EventType eventMsg);
    CControl*   SearchControl(EventType eventMsg);

    EventType    GetEventTypeReduce();
    EventType    GetEventTypeFull();
    EventType    GetEventTypeClose();

    void SetName(std::string name, bool tooltip = true) override;

    void        SetTrashEvent(bool bTrash);
    bool        GetTrashEvent();

    void        SetPos(const glm::vec2& pos) override;
    void        SetDim(const glm::vec2& dim) override;

    void        SetMinDim(const glm::vec2& dim);
    void        SetMaxDim(const glm::vec2& dim);
    glm::vec2   GetMinDim();
    glm::vec2   GetMaxDim();

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

    bool        GetTooltip(const glm::vec2& pos, std::string &name) override;

    bool        EventProcess(const Event &event) override;

    void        Draw() override;

    void        SetFocus(CControl* focusControl) override;

protected:
    int         BorderDetect(const glm::vec2& pos);
    void        AdjustButtons();
    void        MoveAdjust();
    void        DrawVertex(const glm::vec2& pos, const glm::vec2& dim, int icon);
    void        DrawHach(const glm::vec2& pos, const glm::vec2& dim);
    template<typename ControlClass>
    ControlClass* CreateControl(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg);

protected:
    std::vector<std::unique_ptr<CControl>> m_controls;

    bool        m_bTrashEvent;
    bool        m_bMaximized;
    bool        m_bMinimized;
    bool        m_bFixed;

    glm::vec2   m_minDim;
    glm::vec2   m_maxDim;

    std::unique_ptr<CButton> m_buttonReduce;
    std::unique_ptr<CButton> m_buttonFull;
    std::unique_ptr<CButton> m_buttonClose;

    bool        m_bMovable;
    bool        m_bRedim;
    bool        m_bClosable;
    bool        m_bCapture;
    glm::vec2   m_pressPos;
    int         m_pressFlags;
    Gfx::EngineMouseType m_pressMouse;
};


} // namespace Ui
