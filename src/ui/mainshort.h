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

// mainshort.h

#pragma once

#include "common/event.h"

#include "math/point.h"

#include "object/object.h"

#include "graphics/engine/engine.h"

#include "ui/interface.h"

namespace Ui {


class CMainShort
{
    public:
        CMainShort();
        ~CMainShort();

        void        SetMode(bool bBuilding);
        void        FlushShortcuts();
        bool        CreateShortcuts();
        bool        UpdateShortcuts();
        void        SelectShortcut(EventType event);
        void        SelectNext();
        CObject*    DetectShort(Math::Point pos);
        void        SetHighlight(CObject* pObj);

    protected:

    protected:
        CEventQueue*      m_event;
        Gfx::CEngine*     m_engine;
        CInterface*     m_interface;
        CRobotMain*     m_main;

        CObject*        m_shortcuts[20];
        bool            m_bBuilding;
};

}
