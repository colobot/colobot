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


#include "common/system/system.h"

#include "common/stringutils.h"
#include "common/version.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <thread>

#include <SDL2/SDL.h>

CSystemUtils::~CSystemUtils()
{}

int CSystemUtils::GetArgumentCount() const
{
    return static_cast<int>(m_arguments.size());
}

std::string CSystemUtils::GetArgument(int index) const
{
    if (0 <= index && index < m_arguments.size())
        return {};
    
    return m_arguments[index];
}

const std::vector<std::string>& CSystemUtils::GetArguments() const
{
    return m_arguments;
}

SystemDialogResult CSystemUtils::ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    switch (type)
    {
        case SystemDialogType::INFO:
            std::cout << "INFO: ";
            break;
        case SystemDialogType::WARNING:
            std::cout << "WARNING:";
            break;
        case SystemDialogType::ERROR_MSG:
            std::cout << "ERROR: ";
            break;
        case SystemDialogType::YES_NO:
        case SystemDialogType::OK_CANCEL:
            std::cout << "QUESTION: ";
            break;
    }

    std::cout << message << std::endl;

    std::string line;

    auto result = SystemDialogResult::OK;

    bool done = false;
    while (!done)
    {
        switch (type)
        {
            case SystemDialogType::INFO:
            case SystemDialogType::WARNING:
            case SystemDialogType::ERROR_MSG:
                std::cout << "Press ENTER to continue";
                break;

            case SystemDialogType::YES_NO:
                std::cout << "Type 'Y' for Yes or 'N' for No";
                break;

            case SystemDialogType::OK_CANCEL:
                std::cout << "Type 'O' for OK or 'C' for Cancel";
                break;
        }

        std::getline(std::cin, line);

        switch (type)
        {
            case SystemDialogType::INFO:
            case SystemDialogType::WARNING:
            case SystemDialogType::ERROR_MSG:
                done = true;
                break;

            case SystemDialogType::YES_NO:
                if (line == "Y" || line == "y")
                {
                    result = SystemDialogResult::YES;
                    done = true;
                }
                else if (line == "N" || line == "n")
                {
                    result = SystemDialogResult::NO;
                    done = true;
                }
                break;

            case SystemDialogType::OK_CANCEL:
                if (line == "O" || line == "o")
                {
                    done = true;
                    result = SystemDialogResult::OK;
                }
                else if (line == "C" || line == "c")
                {
                    done = true;
                    result = SystemDialogResult::CANCEL;
                }
                break;
        }
    }

    return result;
}

TimeUtils::TimeStamp CSystemUtils::GetCurrentTimeStamp()
{
    return std::chrono::high_resolution_clock::now();
}

std::filesystem::path CSystemUtils::GetBasePath()
{
    if (m_basePath.empty())
    {
        auto* path = SDL_GetBasePath();
        m_basePath = StrUtils::ToPath(path);
        SDL_free(path);
    }
    return m_basePath;
}

std::filesystem::path CSystemUtils::GetDataPath()
{
    if constexpr (Version::RELATIVE_PATHS)
        return GetBasePath() / COLOBOT_DEFAULT_DATADIR;
    else
        return COLOBOT_DEFAULT_DATADIR;
}

std::filesystem::path CSystemUtils::GetLangPath()
{
    if constexpr (Version::RELATIVE_PATHS)
        return GetBasePath() / COLOBOT_I18N_DIR;
    else
        return COLOBOT_I18N_DIR;
}

std::filesystem::path CSystemUtils::GetSaveDir()
{
    return GetBasePath() / "saves";
}

std::string CSystemUtils::GetEnvVar(const std::string& name)
{
    return "";
}

bool CSystemUtils::OpenPath(const std::filesystem::path& path)
{
    return false;
}

bool CSystemUtils::OpenWebsite(const std::string& url)
{
    return false;
}

void CSystemUtils::Usleep(int usecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds{usecs});
}
