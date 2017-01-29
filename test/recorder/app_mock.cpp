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

#include "test/recorder/app_mock.h"

#include "level/level_category.h"
#include "level/robotmain.h"

#include "test/recorder/event_queue_mock.h"
#include "test/recorder/controller_mock.h"
#include "test/recorder/event_queue_mock.h"

#include <memory>


CApplicationMock::CApplicationMock(CSystemUtils* systemUtils)
    : CApplication(systemUtils)
{
    
}

void CApplicationMock::SetInitialLevel(LevelCategory category, int chapter, int level)
{
    m_initialCategory = category;
    m_initialChapter = chapter;
    m_initialLevel = level;
}

void CApplicationMock::SetAssertion(AssertionFunc assertion)
{
    dynamic_cast<CControllerReplay*>(m_controller.get())->SetAssertion(assertion);
}

CObjectManager* CApplicationMock::GetObjectManager()
{
	return m_controller->GetRobotMain()->GetObjectManager();
}

bool CApplicationMock::Create()
{
    bool isSuccess = CApplication::Create();
    
    if (isSuccess)
    {
        if (m_initialCategory != LevelCategory::Max)
        {
            m_controller->StartGame(m_initialCategory, m_initialChapter, m_initialLevel);
        }
        
        if (dynamic_cast<CEventQueueMock*>(m_eventQueue.get()) != nullptr)
        {
            dynamic_cast<CEventQueueMock*>(m_eventQueue.get())->Initialize(this);
        }
        
        if (dynamic_cast<CControllerMock*>(m_controller.get()) != nullptr)
        {
            dynamic_cast<CControllerMock*>(m_controller.get())->Initialize(this);
        }
    }
    
    return isSuccess;
}

Event CApplicationRecord::ProcessSystemEvent()
{
	Event event = CApplication::ProcessSystemEvent();

	if (event.type == EVENT_SYS_QUIT)
	{
		Event clone = event.Clone();

		dynamic_cast<CEventQueueMock*>(m_eventQueue.get())->AddEvent(std::move(clone));
	}

	return event;
}
