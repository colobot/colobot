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

// system.h

#pragma once


#include <string>


/**
 * \enum SysDialogType Type of system dialog
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
 * \enum SysDialogResult Result of system dialog
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
