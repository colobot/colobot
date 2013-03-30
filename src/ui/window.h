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

// window.h

#pragma once

#include "common/event.h"
#include "common/misc.h"
#include "common/restext.h"

#include "graphics/engine/text.h"

#include "ui/button.h"
#include "ui/color.h"
#include "ui/check.h"
#include "ui/key.h"
#include "ui/group.h"
#include "ui/image.h"
#include "ui/label.h"
#include "ui/edit.h"
#include "ui/editvalue.h"
#include "ui/scroll.h"
#include "ui/slider.h"
#include "ui/list.h"
#include "ui/shortcut.h"
#include "ui/map.h"
#include "ui/gauge.h"
#include "ui/compass.h"
#include "ui/target.h"
#include "ui/control.h"

#include <string>

namespace Ui {

const int MAXWINDOW = 100;


class CWindow : public CControl
{
public:
    CWindow();
    ~CWindow();

    void        Flush();
    bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
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
    CList*      CreateList(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand=1.2f);
    CShortcut*  CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CMap*       CreateMap(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CGauge*     CreateGauge(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CCompass*   CreateCompass(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    CTarget*    CreateTarget(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    bool        DeleteControl(EventType eventMsg);
    CControl*   SearchControl(EventType eventMsg);

    EventType    GetEventTypeReduce();
    EventType    GetEventTypeFull();
    EventType    GetEventTypeClose();

    void        SetName(std::string name);

    void        SetTrashEvent(bool bTrash);
    bool        GetTrashEvent();

    void        SetPos(Math::Point pos);
    void        SetDim(Math::Point dim);

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

    bool        GetTooltip(Math::Point pos, std::string &name);

    bool        EventProcess(const Event &event);

    void        Draw();

protected:
    int         BorderDetect(Math::Point pos);
    void        AdjustButtons();
    void        MoveAdjust();
    void        DrawVertex(Math::Point pos, Math::Point dim, int icon);
    void        DrawHach(Math::Point pos, Math::Point dim);

protected:
    CControl*   m_table[MAXWINDOW];

    bool        m_bTrashEvent;
    bool        m_bMaximized;
    bool        m_bMinimized;
    bool        m_bFixed;

    Math::Point     m_minDim;
    Math::Point     m_maxDim;

    CButton*    m_buttonReduce;
    CButton*    m_buttonFull;
    CButton*    m_buttonClose;

    bool        m_bMovable;
    bool        m_bRedim;
    bool        m_bClosable;
    bool        m_bCapture;
    Math::Point     m_pressPos;
    int         m_pressFlags;
    Gfx::EngineMouseType    m_pressMouse;
};


}
