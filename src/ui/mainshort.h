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

// mainshort.h

#pragma once


#include "common/event.h"
#include "math/point.h"


class CInstanceManager;
class CD3DEngine;
class CInterface;
class CRobotMain;
class CObject;



class CMainShort
{
public:
    CMainShort(CInstanceManager* iMan);
    ~CMainShort();

    void        SetMode(bool bBuilding);
    void        FlushShortcuts();
    bool        CreateShortcuts();
    bool        UpdateShortcuts();
    void        SelectShortcut(EventMsg event);
    void        SelectNext();
    CObject*    DetectShort(Math::Point pos);
    void        SetHilite(CObject* pObj);

protected:

protected:
    CInstanceManager* m_iMan;
    CEvent*         m_event;
    CD3DEngine*     m_engine;
    CInterface*     m_interface;
    CRobotMain*     m_main;

    CObject*        m_shortcuts[20];
    bool            m_bBuilding;
};


