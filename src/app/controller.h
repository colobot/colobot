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
 * \file app/controller.h
 * \brief CController class
 */

#pragma once

#include "level/level_category.h"

#include <memory>
#include <string>

class CRobotMain;
struct Event;

/**
 * \class CController
 * \brief Entry point into CRobotMain
 */
class CController
{
public:
    CController();
    ~CController();

    //! Return CRobotMain instance
    CRobotMain*      GetRobotMain();

    //! Event processing
    void ProcessEvent(Event &event);

    //! Start the application
    void StartApp();
    //! Starts the simulation, loading the given scene
    void StartGame(LevelCategory cat, int chap, int lvl);

private:
    std::unique_ptr<CRobotMain> m_main;
};
