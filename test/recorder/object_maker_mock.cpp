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


#include "test/recorder/object_maker_mock.h"

#include "common/make_unique.h"

#include "test/recorder/controller_mock.h"
#include "test/recorder/event_queue_mock.h"
#include "test/recorder/input_mock.h"


CObjectMakerRecord::CObjectMakerRecord(CRecord* record)
{
    m_record = record;
}

std::unique_ptr<CEventQueue> CObjectMakerRecord::MakeEventQueue()
{
    return MakeUnique<CEventQueueRecord>(m_record);
}

std::unique_ptr<CController> CObjectMakerRecord::MakeController()
{
    return MakeUnique<CControllerRecord>(m_record);
}


    
CObjectMakerReplay::CObjectMakerReplay(CRecord* record)
{
    m_record = record;
}

std::unique_ptr<CInput> CObjectMakerReplay::MakeInput()
{
    return MakeUnique<CInputMock>();
}

std::unique_ptr<CEventQueue> CObjectMakerReplay::MakeEventQueue()
{
    return MakeUnique<CEventQueueReplay>(m_record);
}

std::unique_ptr<CController> CObjectMakerReplay::MakeController()
{
	return MakeUnique<CControllerReplay>();
}
