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

// target.h

#pragma once

#include <string>

#include <ui/control.h>

#include <common/event.h>

namespace Ui {

class CTarget : public CControl
{
    public:
        CTarget();
        ~CTarget();

        bool        Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);

        bool        EventProcess(const Event &event);
        void        Draw();
        bool        GetTooltip(Math::Point pos, std::string &name);

    protected:
        CObject*    DetectFriendObject(Math::Point pos);
};


}
