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

/**
 * \file app/pausemanager.h
 * \brief Management of pause modes
 */
#pragma once

#include <string>
#include <vector>
#include <memory>


enum PauseType
{
    PAUSE_NONE = 0,
    PAUSE_USER,
    PAUSE_SATCOM,
    PAUSE_SATCOMMOVIE,
    PAUSE_DIALOG,
    PAUSE_EDITOR,
    PAUSE_VISIT,
    PAUSE_CHEAT,
    PAUSE_PHOTO,
    PAUSE_CODE_BATTLE_LOCK
};

struct ActivePause
{
private:
    friend class CPauseManager;

    explicit ActivePause(PauseType type)
    : type(type)
    {}

    ActivePause(const ActivePause&) = delete;
    ActivePause& operator=(const ActivePause&) = delete;

    PauseType type;
};

class CPauseManager
{
public:
    CPauseManager();
    ~CPauseManager();

    ActivePause* ActivatePause(PauseType type);
    void DeactivatePause(ActivePause* pause);

    void FlushPause();

    bool IsPause();
    PauseType GetPauseType();

private:
    void UpdatePause();

    static std::string GetPauseName(PauseType pause);

private:
    std::vector<std::unique_ptr<ActivePause>> m_activePause;
};
