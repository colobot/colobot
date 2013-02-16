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

// interface.h

#pragma once

#include "common/event.h"
#include "common/misc.h"

#include "math/point.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"

#include "ui/control.h"
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
#include "ui/compass.h"
#include "ui/target.h"
#include "ui/map.h"
#include "ui/window.h"

#include <string>

namespace Ui {

const int MAXCONTROL = 100;


class CInterface
{
    public:
        CInterface();
        ~CInterface();

        bool        EventProcess(const Event &event);
        bool        GetTooltip(Math::Point pos, std::string &name);

        void        Flush();
        CButton*    CreateButton(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CColor*     CreateColor(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CCheck*     CreateCheck(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CKey*       CreateKey(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CGroup*     CreateGroup(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CImage*     CreateImage(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CEdit*      CreateEdit(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CEditValue* CreateEditValue(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CScroll*    CreateScroll(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CSlider*    CreateSlider(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CShortcut*  CreateShortcut(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CCompass*   CreateCompass(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CTarget*    CreateTarget(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CMap*       CreateMap(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);

        CWindow*    CreateWindows(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        CList*      CreateList(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, float expand=1.2f);
        CLabel*     CreateLabel(Math::Point pos, Math::Point dim, int icon, EventType eventMsg, std::string name);

        bool        DeleteControl(EventType eventMsg);
        CControl*   SearchControl(EventType eventMsg);

        void        Draw();

    protected:
        int GetNextFreeControl();
        template <typename T> inline T* CreateControl(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);

        CEventQueue* m_event;
        Gfx::CEngine* m_engine;
        Gfx::CCamera* m_camera;

        CControl* m_table[MAXCONTROL];
};


}
