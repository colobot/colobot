/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "common/system/system.h"

#include "common/make_unique.h"

#if defined(PLATFORM_WINDOWS)
    #include "common/system/system_windows.h"
#elif defined(PLATFORM_LINUX)
    #include "common/system/system_linux.h"
#elif defined(PLATFORM_ANDROID)
    #include "common/system/system_android.h"
#elif defined(PLATFORM_MACOSX)
    #include "common/system/system_macosx.h"
#else
    #include "common/system/system_other.h"
#endif

#include <cassert>
#include <iostream>
#include <algorithm>

#include <SDL2/SDL.h>

std::unique_ptr<CSystemUtils> CSystemUtils::Create()
{
    std::unique_ptr<CSystemUtils> instance;
#if defined(PLATFORM_WINDOWS)
    instance = MakeUnique<CSystemUtilsWindows>();
#elif defined(PLATFORM_LINUX)
    instance = MakeUnique<CSystemUtilsLinux>();
#elif defined(PLATFORM_ANDROID)
    instance = MakeUnique<CSystemUtilsAndroid>();
#elif defined(PLATFORM_MACOSX)
    instance = MakeUnique<CSystemUtilsMacOSX>();
#else
    instance = MakeUnique<CSystemUtilsOther>();
#endif
    return instance;
}

CSystemUtils::~CSystemUtils()
{}

SystemDialogResult CSystemUtils::ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    switch (type)
    {
        case SDT_INFO:
            std::cout << "INFO: ";
            break;
        case SDT_WARNING:
            std::cout << "WARNING:";
            break;
        case SDT_ERROR:
            std::cout << "ERROR: ";
            break;
        case SDT_YES_NO:
        case SDT_OK_CANCEL:
            std::cout << "QUESTION: ";
            break;
    }

    std::cout << message << std::endl;

    std::string line;

    SystemDialogResult result = SDR_OK;

    bool done = false;
    while (!done)
    {
        switch (type)
        {
            case SDT_INFO:
            case SDT_WARNING:
            case SDT_ERROR:
                std::cout << "Press ENTER to continue";
                break;

            case SDT_YES_NO:
                std::cout << "Type 'Y' for Yes or 'N' for No";
                break;

            case SDT_OK_CANCEL:
                std::cout << "Type 'O' for OK or 'C' for Cancel";
                break;
        }

        std::getline(std::cin, line);

        switch (type)
        {
            case SDT_INFO:
            case SDT_WARNING:
            case SDT_ERROR:
                done = true;
                break;

            case SDT_YES_NO:
                if (line == "Y" || line == "y")
                {
                    result = SDR_YES;
                    done = true;
                }
                else if (line == "N" || line == "n")
                {
                    result = SDR_NO;
                    done = true;
                }
                break;

            case SDT_OK_CANCEL:
                if (line == "O" || line == "o")
                {
                    done = true;
                    result = SDR_OK;
                }
                else if (line == "C" || line == "c")
                {
                    done = true;
                    result = SDR_CANCEL;
                }
                break;
        }
    }

    return result;
}

SystemTimeStamp* CSystemUtils::CreateTimeStamp()
{
    auto timeStamp = MakeUnique<SystemTimeStamp>();
    SystemTimeStamp* timeStampPtr = timeStamp.get();
    m_timeStamps.push_back(std::move(timeStamp));
    return timeStampPtr;
}

void CSystemUtils::DestroyTimeStamp(SystemTimeStamp *stamp)
{
    m_timeStamps.erase(std::remove_if(m_timeStamps.begin(), m_timeStamps.end(), [&](const std::unique_ptr<SystemTimeStamp>& timeStamp) { return timeStamp.get() == stamp; }));
}

void CSystemUtils::CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src)
{
    *dst = *src;
}

float CSystemUtils::TimeStampDiff(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit)
{
    long long exact = TimeStampExactDiff(before, after);

    float result = 0.0f;
    if (unit == STU_SEC)
        result = exact * 1e-9;
    else if (unit == STU_MSEC)
        result = exact * 1e-6;
    else if (unit == STU_USEC)
        result = exact * 1e-3;
    else
        assert(false);

    return result;
}

std::string CSystemUtils::GetBasePath()
{
    if (m_basePath.empty())
    {
        auto* path = SDL_GetBasePath();
        m_basePath = path;
        SDL_free(path);
    }
    return m_basePath;
}

std::string CSystemUtils::GetDataPath()
{
#ifdef USE_RELATIVE_PATHS
    return GetBasePath() + COLOBOT_DEFAULT_DATADIR;
#else
    return COLOBOT_DEFAULT_DATADIR;
#endif
}

std::string CSystemUtils::GetLangPath()
{
#ifdef USE_RELATIVE_PATHS
    return GetBasePath() + COLOBOT_I18N_DIR;
#else
    return COLOBOT_I18N_DIR;
#endif
}

std::string CSystemUtils::GetSaveDir()
{
    return GetBasePath() + "saves";
}

std::string CSystemUtils::GetEnvVar(const std::string& name)
{
    return "";
}

bool CSystemUtils::OpenPath(const std::string& path)
{
    return false;
}

bool CSystemUtils::OpenWebsite(const std::string& url)
{
    return false;
}
