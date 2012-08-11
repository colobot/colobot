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


#include <string>


/* Dialog utils */

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

//! Displays a system dialog
SystemDialogResult SystemDialog(SystemDialogType, const std::string &title, const std::string &message);


/* Time utils */

enum SystemTimeUnit
{
    //! seconds
    STU_SEC,
    //! milliseconds
    STU_MSEC,
    //! microseconds
    STU_USEC
};

/* Forward declaration of time stamp struct
  * SystemTimeStamp should be used in a pointer context.
  * The implementation details are hidden because of platform dependence. */
struct SystemTimeStamp;

//! Creates a new time stamp object
SystemTimeStamp* CreateTimeStamp();

//! Destroys a time stamp object
void DestroyTimeStamp(SystemTimeStamp *stamp);

//! Copies the time stamp from \a src to \a dst
void CopyTimeStamp(SystemTimeStamp *dst, SystemTimeStamp *src);

//! Returns a time stamp associated with current time
void GetCurrentTimeStamp(SystemTimeStamp *stamp);

//! Returns the platform's expected time stamp resolution
float GetTimeStampResolution(SystemTimeUnit unit = STU_SEC);

//! Returns the platform's exact (in nanosecond units) expected time stamp resolution
long long GetTimeStampExactResolution();

//! Returns a difference between two timestamps in given time unit
/** The difference is \a after - \a before. */
float TimeStampDiff(SystemTimeStamp *before, SystemTimeStamp *after, SystemTimeUnit unit = STU_SEC);

//! Returns the exact (in nanosecond units) difference between two timestamps
/** The difference is \a after - \a before. */
long long TimeStampExactDiff(SystemTimeStamp *before, SystemTimeStamp *after);
