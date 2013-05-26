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

#include "app/system_linux.h"

#include "common/logger.h"

#include <stdlib.h>


void CSystemUtilsLinux::Init()
{
    m_zenityAvailable = true;
    if (system("zenity --version") != 0)
    {
        m_zenityAvailable = false;
        GetLogger()->Warn("Zenity not available, will fallback to console users dialogs.\n");
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
        case SDT_INFO:
        default:
            options = "--info";
            break;
        case SDT_WARNING:
            options = "--warning";
            break;
        case SDT_ERROR:
            options = "--error";
            break;
        case SDT_YES_NO:
            options = "--question --ok-label=\"Yes\" --cancel-label=\"No\"";
            break;
        case SDT_OK_CANCEL:
            options = "--question --ok-label=\"OK\" --cancel-label=\"Cancel\"";
            break;
    }

    std::string command = "zenity " + options + " --text=\"" + message + "\" --title=\"" + title + "\"";
    int code = system(command.c_str());

    SystemDialogResult result = SDR_OK;
    switch (type)
    {
        case SDT_YES_NO:
            result = code ? SDR_NO : SDR_YES;
            break;
        case SDT_OK_CANCEL:
            result = code ? SDR_CANCEL : SDR_OK;
            break;
        default:
            break;
    }

    return result;
}

void CSystemUtilsLinux::GetCurrentTimeStamp(SystemTimeStamp *stamp)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &stamp->clockTime);
}

long long CSystemUtilsLinux::GetTimeStampExactResolution()
{
    return 1ll;
}

long long CSystemUtilsLinux::TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after)
{
    return (after->clockTime.tv_nsec - before->clockTime.tv_nsec) +
           (after->clockTime.tv_sec  - before->clockTime.tv_sec) * 1000000000ll;
}

std::string CSystemUtilsLinux::GetProfileFileLocation()
{
    std::string profileFile;

    // Determine profileFile according to XDG Base Directory Specification
    char* envXDG_CONFIG_HOME = getenv("XDG_CONFIG_HOME");
    if (envXDG_CONFIG_HOME == NULL)
    {
        char *envHOME = getenv("HOME");
        if (envHOME == NULL)
        {
            profileFile = "colobot.ini";
        }
        else
        {
            profileFile = std::string(envHOME) + "/.config/colobot.ini";
        }
    }
    else
    {
        profileFile = std::string(envXDG_CONFIG_HOME) + "/colobot.ini";
    }
    GetLogger()->Trace("Profile configuration is %s\n", profileFile.c_str());

    return profileFile;
}

std::string CSystemUtilsLinux::GetSavegameDirectoryLocation()
{
    std::string savegameDir;

    // Determine savegame dir according to XDG Base Directory Specification
    char *envXDG_DATA_HOME = getenv("XDG_CONFIG_DATA");
    if (envXDG_DATA_HOME == NULL)
    {
        char *envHOME = getenv("HOME");
        if (envHOME == NULL)
        {
            savegameDir = "/tmp/colobot-savegame";
        }
        else
        {
            savegameDir = std::string(envHOME) + "/.local/share/colobot";
        }
    }
    else
    {
        savegameDir = std::string(envXDG_DATA_HOME) + "/colobot";
    }
    GetLogger()->Trace("Saved game files are going to %s\n", savegameDir.c_str());

    return savegameDir;
}

