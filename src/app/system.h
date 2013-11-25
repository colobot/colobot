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

/**
 * \file app/system.h
 * \brief System functions: time stamps, info dialogs, etc.
 */

#pragma once

#include "common/singleton.h"

#include <string>

/**
 * \enum SystemDialogType
 * \brief Type of system dialog
 */
enum SystemDialogType
{
    //! Information message
    SDT_INFO,
    //! Warning message
    SDT_WARNING,
    //! Error message
    SDT_ERROR,
    //! Yes/No question
    SDT_YES_NO,
    //! Ok/Cancel question
    SDT_OK_CANCEL
};

/**
 * \enum SystemDialogResult
 * \brief Result of system dialog
 *
 * Means which button was pressed.
 */
enum SystemDialogResult
{
    SDR_OK,
    SDR_CANCEL,
    SDR_YES,
    SDR_NO
};

/**
 * \enum SystemTimeUnit
 * \brief Time unit
 */
enum SystemTimeUnit
{
    //! seconds
    STU_SEC,
    //! milliseconds
    STU_MSEC,
    //! microseconds
    STU_USEC
};

/*
 * Forward declaration of time stamp struct
 * SystemTimeStamp should only be used in a pointer context.
 * The implementation details are hidden because of platform dependence.
 */
struct SystemTimeStamp;

/**
 * \class CSystemUtils
 * \brief Platform-specific utils
 *
 * This class provides system-specific utilities like displaying user dialogs and
 * querying system timers for exact timestamps.
 */
class CSystemUtils : public CSingleton<CSystemUtils>
{
protected:
    CSystemUtils();

public:
    //! Creates system utils for specific platform
    static CSystemUtils* Create();

    //! Performs platform-specific initialization
    virtual void Init() = 0;

    //! Displays a system dialog
    virtual SystemDialogResult SystemDialog(SystemDialogType, const std::string &title, const std::string &message) = 0;

    //! Displays a fallback system dialog using console
    TEST_VIRTUAL SystemDialogResult ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message);

    //! Creates a new time stamp object
    TEST_VIRTUAL SystemTimeStamp* CreateTimeStamp();

    //! Destroys a time stamp object
    TEST_VIRTUAL void DestroyTimeStamp(SystemTimeStamp *stamp);

    //! Copies the time stamp from \a src to \a dst
    TEST_VIRTUAL void CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src);

    //! Returns a time stamp associated with current time
    virtual void GetCurrentTimeStamp(SystemTimeStamp *stamp) = 0;

    //! Returns the platform's expected time stamp resolution
    TEST_VIRTUAL float GetTimeStampResolution(SystemTimeUnit unit = STU_SEC);

    //! Returns the platform's exact (in nanosecond units) expected time stamp resolution
    virtual long long GetTimeStampExactResolution() = 0;

    //! Returns a difference between two timestamps in given time unit
    /** The difference is \a after - \a before. */
    TEST_VIRTUAL float TimeStampDiff(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit = STU_SEC);

    //! Returns the exact (in nanosecond units) difference between two timestamps
    /** The difference is \a after - \a before. */
    virtual long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after) = 0;

    //! Returns the data path (containing textures, levels, helpfiles, etc)
    virtual std::string GetDataPath();

    //! Returns the translations path
    virtual std::string GetLangPath();

    //! Returns the profile (colobot.ini) file location
    virtual std::string GetProfileFileLocation();

    //! Returns the savegame directory location
    virtual std::string GetSavegameDirectoryLocation();
};

//! Global function to get CSystemUtils instance
inline CSystemUtils* GetSystemUtils()
{
    return CSystemUtils::GetInstancePointer();
}

