/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <string>
#include <filesystem>

#include <glm/glm.hpp>

class CRobotMain;
class CApplication;
class CEventQueue;
class CSoundInterface;
struct Event;

namespace Gfx
{
class CEngine;
}

namespace Ui
{

class CInterface;

class CScreen
{
public:
    CScreen();
    virtual ~CScreen();

    virtual void CreateInterface() = 0;
    virtual bool EventProcess(const Event &event) = 0;

protected:
    void CreateVersionDisplay();
    void SetBackground(const std::filesystem::path& filename, bool scaled = false);

protected:
    CRobotMain* m_main;
    CInterface* m_interface;
    CApplication* m_app;
    CEventQueue* m_eventQueue;
    Gfx::CEngine* m_engine;
    CSoundInterface* m_sound;

    const glm::vec2 dim = { 32.0f / 640.0f, 32.0f / 480.0f };
    const float ox = 3.0f/640.0f,         oy = 3.0f/480.0f;
    const float sx = (32.0f+2.0f)/640.0f, sy = (32.0f+2.0f)/480.0f;
};

} // namespace Ui
