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
 * \file app/pausemanager.h
 * \brief Management of pause modes
 */
#pragma once

#include "common/make_unique.h"

#include <string>
#include <vector>
#include <memory>

class CRobotMain;


enum PauseType
{
    PAUSE_NONE = 0,
    PAUSE_ENGINE = (1<<0), //!< pause all the CEngine classes
    PAUSE_HIDE_SHORTCUTS = (1<<1), //!< hide the shortcuts
    PAUSE_PHOTO = (1<<2), //!< photo mode, TODO: remove
    PAUSE_OBJECT_UPDATES = (1<<3), //!< do not send events to objects
    PAUSE_MUTE_SOUND = (1<<4), //!< mute sound
    PAUSE_CAMERA = (1<<5), //!< freeze camera
};
inline PauseType& operator|=(PauseType& a, const PauseType& b)
{
    return a = static_cast<PauseType>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}
inline PauseType operator|(PauseType a, const PauseType& b)
{
    return a |= b;
}
inline PauseType& operator&=(PauseType& a, const PauseType& b)
{
    return a = static_cast<PauseType>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
}
inline PauseType operator&(PauseType a, const PauseType& b)
{
    return a &= b;
}

enum PauseMusic
{
    PAUSE_MUSIC_NONE = 0,
    PAUSE_MUSIC_EDITOR = 1,
    PAUSE_MUSIC_SATCOM = 2,
};

struct ActivePause;

class CPauseManager
{
public:
    CPauseManager();
    ~CPauseManager();

    ActivePause* ActivatePause(PauseType type, PauseMusic music = PAUSE_MUSIC_NONE);
    void DeactivatePause(ActivePause* pause);

    void FlushPause();

    PauseType GetPause();
    bool IsPauseType(PauseType type);

private:
    //static std::string GetPauseName(PauseType pause);
    void Update();

private:
    CRobotMain* m_main;

    std::vector<std::unique_ptr<ActivePause>> m_activePause;
    PauseMusic m_lastPauseMusic = PAUSE_MUSIC_NONE;
};
