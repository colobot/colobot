/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
 * \file app/system_linux.h
 * \brief Linux-specific implementation of system functions
 */

#include "app/system.h"

#include <sys/time.h>


struct SystemTimeStamp
{
    timespec clockTime;

    SystemTimeStamp()
    {
        clockTime.tv_sec = clockTime.tv_nsec = 0;
    }
};

class CSystemUtilsLinux : public CSystemUtils
{
public:
    virtual void Init() override;

    virtual SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message) override;

    virtual void GetCurrentTimeStamp(SystemTimeStamp *stamp) override;
    virtual long long GetTimeStampExactResolution() override;
    virtual long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after) override;

    virtual std::string GetSaveDir() override;

private:
    bool m_zenityAvailable;
};

