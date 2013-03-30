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

/**
 * \file ui/key.h
 * \brief Key slot control
 */

#pragma once

#include "ui/control.h"

#include "common/event.h"
#include "common/restext.h"
#include "common/key.h"


namespace Ui {

class CKey : public CControl
{
public:
    CKey();
    virtual ~CKey();

    //! Creates a new key slot button
    bool    Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg);
    //! Management of an event
    bool    EventProcess(const Event &event);

    //! Draws button
    void    Draw();

    //! Management of binding
    //@{
    void    SetBinding(InputBinding b);
    InputBinding GetBinding();
    //@}

protected:
    //! Checks if a key is already used
    bool    TestKey(unsigned int key);

protected:
    CRobotMain* m_robotMain;

    InputBinding m_binding;
    bool         m_catch;
};


} // namespace Ui

