/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file ui/controls/key.h
 * \brief Key slot control
 */

#pragma once

#include "ui/controls/control.h"

#include "app/input.h"

#include "common/event.h"
#include "common/key.h"
#include "common/restext.h"


namespace Ui
{

class CKey : public CControl
{
public:
    CKey();
    virtual ~CKey();

    //! Creates a new key slot button
    bool    Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg) override;
    //! Management of an event
    bool    EventProcess(const Event &event) override;

    //! Draws button
    void    Draw() override;

    //! Management of binding
    //@{
    void    SetBinding(InputBinding b);
    InputBinding GetBinding();
    //@}

protected:
    //! Checks if a key is already used
    bool    TestKey(unsigned int key);

protected:
    CInput*      m_input;

    InputBinding m_binding;
    bool         m_catch;
};


} // namespace Ui
