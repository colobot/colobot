/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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


template<> CPauseManager* CSingleton<CPauseManager>::m_instance = nullptr;


CPauseManager::CPauseManager()
{}

CPauseManager::~CPauseManager()
{}

ActivePause* CPauseManager::ActivatePause(PauseType type)
{
    assert(type != PAUSE_NONE);
    GetLogger()->Debug("Activated pause mode - %s\n", GetPauseName(type).c_str());
    auto pause = std::unique_ptr<ActivePause>(new ActivePause(type)); // TODO: Can't use MakeUnique here because the constructor is private
    ActivePause* ptr = pause.get();
    m_activePause.push_back(std::move(pause));
    UpdatePause();
    return ptr;
}

void CPauseManager::DeactivatePause(ActivePause* pause)
{
    if (pause == nullptr) return;
    GetLogger()->Debug("Deactivated pause mode - %s\n", GetPauseName(pause->type).c_str());
    m_activePause.erase(std::remove_if(
        m_activePause.begin(), m_activePause.end(),
        [&](const std::unique_ptr<ActivePause>& x) { return x.get() == pause; })
    );
    UpdatePause();
}

void CPauseManager::FlushPause()
{
    m_activePause.clear();
}

bool CPauseManager::IsPause()
{
    return m_activePause.size() > 0;
}

PauseType CPauseManager::GetPauseType()
{
    if (m_activePause.size() > 0)
        return m_activePause[m_activePause.size()-1]->type;
    return PAUSE_NONE;
}

void CPauseManager::UpdatePause()
{
    PauseType type = GetPauseType();

    if (type != PAUSE_NONE)
    {
        GetLogger()->Info("Game paused - %s\n", GetPauseName(type).c_str());
    }
    else
    {
        GetLogger()->Info("Game resumed\n");
    }

    CRobotMain::GetInstancePointer()->UpdatePause(type);
}

std::string CPauseManager::GetPauseName(PauseType pause)
{
    switch(pause)
    {
        case PAUSE_NONE:        return "None";
        case PAUSE_USER:        return "User";
        case PAUSE_SATCOM:      return "SatCom";
        case PAUSE_SATCOMMOVIE: return "SatCom opening animation";
        case PAUSE_DIALOG:      return "Dialog";
        case PAUSE_EDITOR:      return "CBot editor";
        case PAUSE_VISIT:       return "Visit";
        case PAUSE_CHEAT:       return "Cheat console";
        case PAUSE_PHOTO:       return "Photo mode";
        case PAUSE_CODE_BATTLE_LOCK: return "Code battle lock";
        default: assert(false); // Should never happen
    }
    return "?";
}
