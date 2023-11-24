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

#include "app/pausemanager.h"

#include "app/app.h"

#include "common/logger.h"

#include "level/robotmain.h"

#include <algorithm>
#include <numeric>

struct ActivePause
{
    explicit ActivePause(PauseType type, PauseMusic music = PAUSE_MUSIC_NONE)
        : type(type),
          music(music)
    {}

    ActivePause(const ActivePause&) = delete;
    ActivePause& operator=(const ActivePause&) = delete;

    PauseType type;
    PauseMusic music;
};

static std::string GetPauseName(PauseType type)
{
    std::string result;

    auto append = [&](std::string_view text)
    {
        if (!result.empty()) result.push_back('|');

        result.append(text);
    };

    if ((type & PAUSE_ENGINE) != 0) append("engine");
    if ((type & PAUSE_HIDE_SHORTCUTS) != 0) append("hide_shortcuts");
    if ((type & PAUSE_PHOTO) != 0) append("photo");
    if ((type & PAUSE_OBJECT_UPDATES) != 0) append("object_updates");
    if ((type & PAUSE_MUTE_SOUND) != 0) append("mute_sound");
    if ((type & PAUSE_CAMERA) != 0) append("camera");

    return result;
}

CPauseManager::CPauseManager()
{
    m_main = CRobotMain::GetInstancePointer();
}

CPauseManager::~CPauseManager()
{}

ActivePause* CPauseManager::ActivatePause(PauseType type, PauseMusic music)
{
    GetLogger()->Debug("Activated pause mode - %%", GetPauseName(type));
    auto pause = std::make_unique<ActivePause>(type, music);
    ActivePause* ptr = pause.get();
    m_activePause.push_back(std::move(pause));
    Update();
    return ptr;
}

void CPauseManager::DeactivatePause(ActivePause* pause)
{
    if (pause == nullptr) return;
    GetLogger()->Debug("Deactivated pause mode - %%", GetPauseName(pause->type));
    auto it = std::remove_if(
        m_activePause.begin(), m_activePause.end(),
        [&](const std::unique_ptr<ActivePause>& x) { return x.get() == pause; }
    );
    if (it == m_activePause.end())
    {
        GetLogger()->Warn("Releasing previously not deactivated pause now!");
        std::free(pause);
    }
    m_activePause.erase(it);
    Update();
}

void CPauseManager::FlushPause()
{
    for (auto& pause : m_activePause)
    {
        GetLogger()->Warn("Pause not deactivated before phase change!");
        pause.release();
    }
    m_activePause.clear();
}

PauseType CPauseManager::GetPause()
{
    PauseType current = PAUSE_NONE;
    for(auto& pause : m_activePause)
    {
        current |= pause->type;
    }
    return current;
}

bool CPauseManager::IsPauseType(PauseType type)
{
    PauseType current = GetPause();
    return (current & type) == type;
}

void CPauseManager::Update()
{
    m_main->UpdatePause(GetPause()); //TODO

    PauseMusic music = PAUSE_MUSIC_NONE;
    for(int i = m_activePause.size()-1; i >= 0; i--)
    {
        if (m_activePause[i]->music != PAUSE_MUSIC_NONE)
        {
            music = m_activePause[i]->music;
            break;
        }
    }
    if (music != m_lastPauseMusic)
    {
        m_main->UpdatePauseMusic(music);
        m_lastPauseMusic = music;
    }
}
