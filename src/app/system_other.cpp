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

#include "app/system_other.h"


SystemDialogResult SystemDialog_Other(SystemDialogType type, const std::string& title, const std::string& message)
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



void GetCurrentTimeStamp_Other(SystemTimeStamp *stamp)
{
    stamp->sdlTicks = SDL_GetTicks();
}

long long GetTimeStampExactResolution_Other()
{
    return 1000000ll;
}

long long TimeStampExactDiff_Other(SystemTimeStamp *before, SystemTimeStamp *after)
{
    return (after->sdlTicks - before->sdlTicks) * 1000000ll;
}
