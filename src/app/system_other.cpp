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

#include "app/system_other.h"


SystemDialogResult CSystemUtilsOther::SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
    return ConsoleSystemDialog(type, title, message);
}

void CSystemUtilsOther::GetCurrentTimeStamp(SystemTimeStamp* stamp)
{
    stamp->sdlTicks = SDL_GetTicks();
}

long long int CSystemUtilsOther::GetTimeStampExactResolution()
{
    return 1000000ll;
}

long long int CSystemUtilsOther::TimeStampExactDiff(SystemTimeStamp* before, SystemTimeStamp* after)
{
    return (after->sdlTicks - before->sdlTicks) * 1000000ll;
}

