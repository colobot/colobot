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


#include "common/event.h"

#include "common/logger.h"

static EventType g_uniqueEventType = EVENT_USER;



EventType GetUniqueEventType()
{
    int i = static_cast<int>(g_uniqueEventType+1);
    g_uniqueEventType = static_cast<EventType>(i);
    return g_uniqueEventType;
}



CEventQueue::CEventQueue()
{
    Flush();
}

CEventQueue::~CEventQueue()
{
}

void CEventQueue::Flush()
{
    m_head = 0;
    m_tail = 0;
    m_total = 0;
}

/** If the maximum size of queue has been reached, returns \c false.
    Else, adds the event to the queue and returns \c true. */
bool CEventQueue::AddEvent(const Event &event)
{
    if ( m_total >= MAX_EVENT_QUEUE )
    {
        GetLogger()->Warn("Event queue flood!\n");
        return false;
    }

    m_fifo[m_head++] = event;
    if ( m_head >= MAX_EVENT_QUEUE )  m_head = 0;
    m_total ++;

    return true;
}

/** If the queue is empty, returns \c false.
    Else, gets the event from the front, puts it into \a event and returns \c true. */
bool CEventQueue::GetEvent(Event &event)
{
    if ( m_head == m_tail )  return false;

    event = m_fifo[m_tail++];
    if ( m_tail >= MAX_EVENT_QUEUE )  m_tail = 0;
    m_total --;

    return true;
}

