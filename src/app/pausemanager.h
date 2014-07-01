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

/**
 * \file app/pausemanager.h
 * \brief Management of pause modes
 */
#pragma once

#include "common/singleton.h"
#include "sound/sound.h"

#include <string>


enum PauseType {
    PAUSE_NONE = 0,
    PAUSE_USER,
    PAUSE_SATCOM,
    PAUSE_SATCOMMOVIE,
    PAUSE_DIALOG,
    PAUSE_EDITOR,
    PAUSE_VISIT,
    PAUSE_CHEAT,
    PAUSE_PHOTO
};

class CPauseManager : public CSingleton<CPauseManager>
{
public:
    CPauseManager();
    ~CPauseManager();
    
    void SetPause(PauseType pause);
    void ClearPause();
    bool GetPause();
    bool GetPause(PauseType pause);
    PauseType GetPauseType();
    
private:
    std::string GetPauseName(PauseType pause);
    
private:
    CSoundInterface* m_sound;
    
    PauseType m_pause;
};

