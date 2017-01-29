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

#pragma once

#include "app/app.h"

#include "common/event.h"

#include "object/object_manager.h"

#include "test/recorder/controller_mock.h"

#include <boost/function.hpp>


class CApplicationMock : public CApplication
{
public:
    CApplicationMock(CSystemUtils* systemUtils);
    
    //! Level will start immediately at the beginning of game instead of main menu
    void 			SetInitialLevel(LevelCategory category, int chapter, int level);
    //! Set function that tests correctness of code, invoked while every EVENT_FRAME
    void 			SetAssertion(AssertionFunc assertion);

    CObjectManager* GetObjectManager();

    bool Create() override;
    
protected:
    LevelCategory 	m_initialCategory = LevelCategory::Max; //!< If different from LevelCategory::Max, specified level will start instead of main menu
    int 			m_initialChapter = 0;
    int 			m_initialLevel = 0;
};

class CApplicationRecord : public CApplicationMock
{
public:
    CApplicationRecord(CSystemUtils* systemUtils)
		: CApplicationMock(systemUtils)
	{ }

protected:
    Event ProcessSystemEvent() override;
};


