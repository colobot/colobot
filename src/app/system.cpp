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

// system.cpp

#include "app/system.h"

#include "common/config.h"


#if defined(PLATFORM_WINDOWS)
#include "system_windows.h"

#elif defined(PLATFORM_LINUX)
#include "system_linux.h"

#else
#include "system_other.h"

#endif


#include <cassert>


/**
 * Displays a system dialog with info, error, question etc. message.
 *
 * \param type type of dialog
 * \param message text of message (in UTF-8)
 * \param title dialog title (in UTF-8)
 * \returns result (which button was clicked)
 */
SystemDialogResult SystemDialog(SystemDialogType type, const std::string& title, const std::string& message)
{
#if defined(PLATFORM_WINDOWS)
    return SystemDialog_Windows(type, title, message);
#elif defined(PLATFORM_LINUX)
    return SystemDialog_Linux(type, title, message);
#else
    return SystemDialog_Other(type, title, message);
#endif
}

SystemTimeStamp* CreateTimeStamp()
{
    return new SystemTimeStamp();
}

void DestroyTimeStamp(SystemTimeStamp *stamp)
{
    delete stamp;
}

void CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src)
{
    *dst = *src;
}

void GetCurrentTimeStamp(SystemTimeStamp *stamp)
{
#if defined(PLATFORM_WINDOWS)
    GetCurrentTimeStamp_Windows(stamp);
#elif defined(PLATFORM_LINUX)
    GetCurrentTimeStamp_Linux(stamp);
#else
    GetCurrentTimeStamp_Other(stamp);
#endif
}

float GetTimeStampResolution(SystemTimeUnit unit)
{
    unsigned long long exact = 0;
#if defined(PLATFORM_WINDOWS)
    exact = GetTimeStampExactResolution_Windows();
#elif defined(PLATFORM_LINUX)
    exact = GetTimeStampExactResolution_Linux();
#else
    exact = GetTimeStampExactResolution_Other();
#endif
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

long long GetTimeStampExactResolution()
{
#if defined(PLATFORM_WINDOWS)
    return GetTimeStampExactResolution_Windows();
#elif defined(PLATFORM_LINUX)
    return GetTimeStampExactResolution_Linux();
#else
    return GetTimeStampExactResolution_Other();
#endif
}

float TimeStampDiff(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit)
{
    long long exact = 0;
#if defined(PLATFORM_WINDOWS)
    exact = TimeStampExactDiff_Windows(before, after);
#elif defined(PLATFORM_LINUX)
    exact = TimeStampExactDiff_Linux(before, after);
#else
    exact = TimeStampExactDiff_Other(before, after);
#endif
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

long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after)
{
#if defined(PLATFORM_WINDOWS)
    return TimeStampExactDiff_Windows(before, after);
#elif defined(PLATFORM_LINUX)
    return TimeStampExactDiff_Linux(before, after);
#else
    return TimeStampExactDiff_Other(before, after);
#endif
}
