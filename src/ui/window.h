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

// window.h

#pragma once


#include "ui/control.h"


class CD3DEngine;
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
class CList;
class CShortcut;
class CMap;
class CGauge;
class CCompass;
class CTarget;


const int MAXWINDOW = 100;


class CWindow : public CControl
{
public:
    CWindow(CInstanceManager* iMan);
    ~CWindow();

    void        Flush();
    bool        Create(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CButton*    CreateButton(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CColor*     CreateColor(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CCheck*     CreateCheck(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CKey*       CreateKey(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CGroup*     CreateGroup(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CImage*     CreateImage(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CLabel*     CreateLabel(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg, char *name);
    CEdit*      CreateEdit(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CEditValue* CreateEditValue(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CScroll*    CreateScroll(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CSlider*    CreateSlider(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CList*      CreateList(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg, float expand=1.2f);
    CShortcut*  CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CMap*       CreateMap(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CGauge*     CreateGauge(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CCompass*   CreateCompass(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    CTarget*    CreateTarget(Math::Point pos, Math::Point dim, int icon, EventMsg eventMsg);
    bool        DeleteControl(EventMsg eventMsg);
    CControl*   SearchControl(EventMsg eventMsg);

    EventMsg    RetEventMsgReduce();
    EventMsg    RetEventMsgFull();
    EventMsg    RetEventMsgClose();

    void        SetName(char* name);

    void        SetTrashEvent(bool bTrash);
    bool        RetTrashEvent();

    void        SetPos(Math::Point pos);
    void        SetDim(Math::Point dim);

    void        SetMinDim(Math::Point dim);
    void        SetMaxDim(Math::Point dim);
    Math::Point     RetMinDim();
    Math::Point     RetMaxDim();

    void        SetMovable(bool bMode);
    bool        RetMovable();

    void        SetRedim(bool bMode);
    bool        RetRedim();

    void        SetClosable(bool bMode);
    bool        RetClosable();

    void        SetMaximized(bool bMaxi);
    bool        RetMaximized();
    void        SetMinimized(bool bMini);
    bool        RetMinimized();
    void        SetFixed(bool bFix);
    bool        RetFixed();

    bool        GetTooltip(Math::Point pos, char* name);

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
    D3DMouse    m_pressMouse;
};


