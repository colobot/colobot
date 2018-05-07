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
 * \file common/system/system_windows.h
 * \brief Windows-specific implementation of system functions
 */

#include "common/system/system.h"

//@colobot-lint-exclude UndefinedFunctionRule

struct SystemTimeStamp
{
    long long counterValue = 0;
};

class CSystemUtilsWindows : public CSystemUtils
{
public:
    void Init() override;

    SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message) override;

    void GetCurrentTimeStamp(SystemTimeStamp *stamp) override;
    long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after) override;

    std::string GetSaveDir() override;

    void Usleep(int usec) override;

public:
    static std::string UTF8_Encode(const std::wstring &wstr);
    static std::wstring UTF8_Decode(const std::string &str);

protected:
    long long m_counterFrequency = 0;
};

//@end-colobot-lint-exclude
