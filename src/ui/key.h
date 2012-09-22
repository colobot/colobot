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

// key.h

#pragma once

#include <cctype>
#include <string>

#include "ui/control.h"

#include "common/iman.h"
#include "common/event.h"
#include "common/restext.h"
#include "common/key.h"

#include "app/app.h"

namespace Ui {

class CKey : public CControl
{
    public:
        CKey();
        virtual ~CKey();

        bool    Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
        bool    EventProcess(const Event &event);

        void    Draw();

        void    SetKey(int option, int key);
        int     GetKey(int option);

    protected:
        bool    TestKey(int key);

        unsigned int m_key[2];
        bool    m_bCatch;

        CApplication *m_app;
};


}

