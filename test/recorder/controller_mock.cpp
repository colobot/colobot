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


#include "test/recorder/controller_mock.h"

#include "test/recorder/recorder.h"


CControllerRecord::CControllerRecord(CRecord* record)
    : m_record(record)
{
    
}

void CControllerRecord::ProcessEvent(Event& event)
{
	if (event.type != EVENT_FRAME) // don't record EVENT_FRAME, number of FPS can differ
	{
		Event cloneEvent = event.Clone();
		RecordedEvent recordEvent(std::move(cloneEvent), m_app->GetAbsTime());

		m_record->WriteEvent(recordEvent);
	}

    CController::ProcessEvent(event);
}

void CControllerReplay::ProcessEvent(Event& event)
{
	if (event.type == EVENT_FRAME && !m_assertion.empty())
	{
		m_assertion(dynamic_cast<CApplicationMock*>(m_app));
	}

	CController::ProcessEvent(event);
}
