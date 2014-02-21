// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#include "app/app.h"
#include "app/pausemanager.h"

#include "common/logger.h"

#include "object/robotmain.h"


template<> CPauseManager* CSingleton<CPauseManager>::m_instance = nullptr;


CPauseManager::CPauseManager()
{
    m_sound = CApplication::GetInstancePointer()->GetSound();
    
    m_pause = PAUSE_NONE;
}

CPauseManager::~CPauseManager()
{
    m_sound = nullptr;
}

void CPauseManager::SetPause(PauseType pause)
{
    if(pause != PAUSE_NONE) {
        if(m_pause != pause) {
            CLogger::GetInstancePointer()->Info("Game paused - %s\n", GetPauseName(pause).c_str());
            CRobotMain::GetInstancePointer()->StartPauseMusic(pause);
        }
        
        m_pause = pause;
    } else
        ClearPause();
}

void CPauseManager::ClearPause()
{
    if(m_pause != PAUSE_NONE) {
        CLogger::GetInstancePointer()->Info("Game resumed\n");
        m_sound->StopPauseMusic();
    }
    
    m_pause = PAUSE_NONE;
}

bool CPauseManager::GetPause()
{
    return m_pause != PAUSE_NONE;
}

bool CPauseManager::GetPause(PauseType pause)
{
    return m_pause == pause;
}

PauseType CPauseManager::GetPauseType()
{
    return m_pause;
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
        default: assert(false); // Should never happen
    }
}
