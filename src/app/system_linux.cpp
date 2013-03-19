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

#include <stdlib.h>


SystemDialogResult SystemDialog_Linux(SystemDialogType type, const std::string& title, const std::string& message)
{
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

void GetCurrentTimeStamp_Linux(SystemTimeStamp *stamp)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &stamp->clockTime);
}

long long GetTimeStampExactResolution_Linux()
{
    return 1ll;
}

long long TimeStampExactDiff_Linux(SystemTimeStamp *before, SystemTimeStamp *after)
{
    return (after->clockTime.tv_nsec - before->clockTime.tv_nsec) +
           (after->clockTime.tv_sec  - before->clockTime.tv_sec) * 1000000000ll;
}
