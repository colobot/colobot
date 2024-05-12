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

#include "common/resources/resourcemanager.h"

#include "level/robotmain.h"

#include <SDL.h>

#include <cassert>
#include <iostream>
#include <algorithm>
#include <thread>

CSystemUtils::CSystemUtils()
{
    m_basePath = std::filesystem::current_path();
}

CSystemUtils::~CSystemUtils() = default;

CSystemUtils& CSystemUtils::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = Create();
    }

    return *m_instance;
}

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

std::filesystem::path CSystemUtils::GetBasePath() const
{
    return m_basePath;
}

std::filesystem::path CSystemUtils::GetDataPath() const
{
    if constexpr (Version::RELATIVE_PATHS)
        return GetBasePath() / COLOBOT_DEFAULT_DATADIR;
    else
        return COLOBOT_DEFAULT_DATADIR;
}

std::filesystem::path CSystemUtils::GetLangPath() const
{
    if constexpr (Version::RELATIVE_PATHS)
        return GetBasePath() / COLOBOT_I18N_DIR;
    else
        return COLOBOT_I18N_DIR;
}

std::filesystem::path CSystemUtils::GetSaveDir() const
{
    return GetBasePath() / "saves";
}

std::string CSystemUtils::GetEnvVar(const std::string& name) const
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

bool CSystemUtils::IsDebuggerPresent() const
{
    return false;
}

void CSystemUtils::Usleep(int usecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds{usecs});
}

void CSystemUtils::CriticalError(std::string_view message)
{
    static bool triedSaving = false;

    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        // Close the SDL window on crash, because otherwise the error doesn't show on in fullscreen mode and the game appears to freeze
        SDL_Quit();
    }

    std::stringstream msg;
    msg << "Unhandled exception occurred!" << std::endl;
    msg << "==============================" << std::endl;
    msg << message << std::endl;
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "This is usually caused by a bug. Please report this on http://github.com/colobot/colobot/issues" << std::endl;
    msg << "including information on what you were doing before this happened and all the information below." << std::endl;
    msg << "==============================" << std::endl;

    if constexpr (Version::BUILD_NUMBER == 0)
    {
        if constexpr (Version::OFFICIAL_BUILD)
        {
            msg << "You are running official " << Version::VERSION_DISPLAY << " build." << std::endl;
        }
        else
        {
            msg << "You seem to be running a custom compilation of version " << Version::VERSION_DISPLAY << ", but please verify that." << std::endl;
        }
    }
    else
    {
        msg << "You are running version " << Version::VERSION_DISPLAY << " from CI build #" << Version::BUILD_NUMBER << std::endl;
    }

    msg << std::endl;
    bool canSave = false;
    CRobotMain* robotMain = nullptr;
    if (!CRobotMain::IsCreated())
    {
        msg << "CRobotMain instance does not seem to exist" << std::endl;
    }
    else
    {
        robotMain = CRobotMain::GetInstancePointer();
        msg << "The game was in phase " << PhaseToString(robotMain->GetPhase()) << " (ID=" << robotMain->GetPhase() << ")" << std::endl;
        msg << "Last started level was: category=" << GetLevelCategoryDir(robotMain->GetLevelCategory()) << " chap=" << robotMain->GetLevelChap() << " rank=" << robotMain->GetLevelRank() << std::endl;
        canSave = (robotMain->GetPhase() == PHASE_SIMUL || IsInSimulationConfigPhase(robotMain->GetPhase())) && !robotMain->IsLoading();
    }
    msg << "==============================" << std::endl;
    msg << std::endl;
    msg << "Sorry for inconvenience!";

    std::cerr << std::endl << msg.str() << std::endl;

    SystemDialog(SystemDialogType::ERROR_MSG, "Unhandled exception occurred!", msg.str());

    if (canSave && !triedSaving)
    {
        msg.str("");
        msg << "You can try saving the game at the moment of a crash. Keep in mind, the game engine is in" << std::endl;
        msg << "an unstable state so the saved game may be corrupted or even cause another crash." << std::endl;
        msg << std::endl;
        msg << "Do you want to try saving now?";

        SystemDialogResult result = SystemDialog(SystemDialogType::YES_NO, "Try to save?", msg.str());
        if (result == SystemDialogResult::YES)
        {
            triedSaving = true;
            CResourceManager::CreateNewDirectory("crashsave");
            robotMain->IOWriteScene("crashsave/data.sav", "crashsave/cbot.run", "crashsave/screen.png", "Backup at the moment of a crash", true);
            SystemDialog(SystemDialogType::INFO, "Try to save?", "Saving finished.\nPlease restart the game now");
        }
    }

    exit(1);
}
