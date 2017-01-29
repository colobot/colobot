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


#include "test/recorder/event_queue_mock.h"


bool CEventQueueRecord::AddEvent(Event&& event)
{
    if (event.type == EVENT_SYS_QUIT || event.type == EVENT_QUIT || event.type == EVENT_RESOLUTION_CHANGED)
    {
        Event cloneEvent = event.Clone();
        RecordedEvent recordedEvent(std::move(cloneEvent), m_app->GetAbsTime());
        
        m_record->WriteEvent(recordedEvent);
    }
    
    return CEventQueue::AddEvent(std::move(event));
}

bool CEventQueueReplay::AddEvent(Event&& event)
{
    return true;
}

Event CEventQueueReplay::GetEvent()
{
    if (m_record->CanReadEvent(m_app->GetAbsTime()))
    {
        Event e = m_record->ReadEvent();
        
        return e;
    }

    return Event();
}

bool CEventQueueReplay::IsEmpty()
{
    return !m_record->CanReadEvent(m_app->GetAbsTime());
}
