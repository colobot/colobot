/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "test/recorder/input_mock.h"

#include "common/event.h"

void CInputMock::EventProcess(Event& event)
{
        Event e = event.Clone();
        
        CInput::EventProcess(event);

        // CInput::EventProcess() changes event, so we must restore part of variables
        event.mousePos = e.mousePos;
        event.mouseButtonsState = e.mouseButtonsState;
        event.kmodState = e.kmodState;
}
