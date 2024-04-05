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

#include "common/system/system_linux.h"

#include "common/logger.h"
#include "common/stringutils.h"
#include "common/version.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>


std::unique_ptr<CSystemUtils> CSystemUtils::Create()
{
    return std::make_unique<CSystemUtilsLinux>();
}

void CSystemUtilsLinux::Init(const std::vector<std::string>& args)
{
    m_arguments = args;

    m_zenityAvailable = true;
    if (system("zenity --version 1> /dev/null 2> /dev/null") != 0)
    {
        m_zenityAvailable = false;
        GetLogger()->Warn("Zenity not available, will fallback to console users dialogs.");
    }
}

SystemDialogResult CSystemUtilsLinux::SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    if (!m_zenityAvailable)
    {
        return ConsoleSystemDialog(type, title, message);
    }

    std::string options = "";
    switch (type)
    {
        case SystemDialogType::INFO:
        default:
            options = "--info";
            break;
        case SystemDialogType::WARNING:
            options = "--warning";
            break;
        case SystemDialogType::ERROR_MSG:
            options = "--error";
            break;
        case SystemDialogType::YES_NO:
            options = "--question --ok-label=\"Yes\" --cancel-label=\"No\"";
            break;
        case SystemDialogType::OK_CANCEL:
            options = "--question --ok-label=\"OK\" --cancel-label=\"Cancel\"";
            break;
    }

    std::string command = "zenity " + options + " --text=\"" + message + "\" --title=\"" + title + "\"";
    int code = system(command.c_str());

    SystemDialogResult result = SystemDialogResult::OK;
    switch (type)
    {
        case SystemDialogType::YES_NO:
            result = code ? SystemDialogResult::NO : SystemDialogResult::YES;
            break;
        case SystemDialogType::OK_CANCEL:
            result = code ? SystemDialogResult::CANCEL : SystemDialogResult::OK;
            break;
        default:
            break;
    }

    return result;
}

std::filesystem::path CSystemUtilsLinux::GetSaveDir()
{
    if constexpr (Version::PORTABLE_SAVES || Version::DEVELOPMENT_BUILD)
    {
        return CSystemUtils::GetSaveDir();
    }
    else
    {
        std::filesystem::path savegameDir;

        // Determine savegame dir according to XDG Base Directory Specification
        auto envXDG_DATA_HOME = GetEnvVar("XDG_DATA_HOME");
        if (envXDG_DATA_HOME.empty())
        {
            auto envHOME = GetEnvVar("HOME");
            if (envHOME.empty())
            {
                GetLogger()->Warn("Unable to find directory for saves - using default directory");
                savegameDir = CSystemUtils::GetSaveDir();
            }
            else
            {
                savegameDir = StrUtils::ToPath(envHOME) / ".local/share/colobot";
            }
        }
        else
        {
            savegameDir = StrUtils::ToPath(envXDG_DATA_HOME) / "colobot";
        }
        GetLogger()->Trace("Saved game files are going to %%", savegameDir);

        return savegameDir;
    }
}

std::string CSystemUtilsLinux::GetEnvVar(const std::string& name)
{
    char* envVar = getenv(name.c_str());
    if (envVar != nullptr)
    {
        GetLogger()->Trace("Detected environment variable %% = %%", name, envVar);
        return std::string(envVar);
    }
    return "";
}

bool CSystemUtilsLinux::OpenPath(const std::filesystem::path& path)
{
    int result = system(("xdg-open \"" + StrUtils::ToString(path) + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open path: %%, error code: %%", path, result);
        return false;
    }
    return true;
}

bool CSystemUtilsLinux::OpenWebsite(const std::string& url)
{
    int result = system(("xdg-open \"" + url + "\"").c_str());
    if (result != 0)
    {
        GetLogger()->Error("Failed to open website: %%, error code: %%", url, result);
        return false;
    }
    return true;
}

