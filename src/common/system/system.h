/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file common/system/system.h
 * \brief System functions: time stamps, info dialogs, etc.
 */

#pragma once

#include "common/config.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

/**
 * \enum SystemDialogType
 * \brief Type of system dialog
 */
enum class SystemDialogType
{
    //! Information message
    INFO,
    //! Warning message
    WARNING,
    //! Error message
    ERROR,
    //! Yes/No question
    YES_NO,
    //! Ok/Cancel question
    OK_CANCEL
};

/**
 * \enum SystemDialogResult
 * \brief Result of system dialog
 *
 * Means which button was pressed.
 */
enum class SystemDialogResult
{
    OK,
    CANCEL,
    YES,
    NO
};

/**
 * \enum SystemTimeUnit
 * \brief Time unit
 */
enum class SystemTimeUnit
{
    SECONDS,
    MILLISECONDS,
    MICROSECONDS
};

using SystemTimeStamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

/**
 * \class CSystemUtils
 * \brief Platform-specific utils
 *
 * This class provides system-specific utilities like displaying user dialogs and
 * querying system timers for exact timestamps.
 */
class CSystemUtils
{
public:
    virtual ~CSystemUtils();

    //! Creates system utils for specific platform
    static std::unique_ptr<CSystemUtils> Create();

    //! Performs platform-specific initialization
    virtual void Init() = 0;

    //! Displays a system dialog
    virtual SystemDialogResult SystemDialog(SystemDialogType, const std::string &title, const std::string &message) = 0;

    //! Displays a fallback system dialog using console
    TEST_VIRTUAL SystemDialogResult ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message);

    //! Returns a time stamp associated with current time
    TEST_VIRTUAL SystemTimeStamp GetCurrentTimeStamp();

    //! Linearly interpolates between two timestamps.
    SystemTimeStamp TimeStampLerp(SystemTimeStamp a, SystemTimeStamp b, float t);

    //! Returns a difference between two timestamps in given time unit
    /** The difference is \a after - \a before. */
    float TimeStampDiff(SystemTimeStamp before, SystemTimeStamp after, SystemTimeUnit unit = SystemTimeUnit::SECONDS);

    //! Returns the exact (in nanosecond units) difference between two timestamps
    /** The difference is \a after - \a before. */
    long long TimeStampExactDiff(SystemTimeStamp before, SystemTimeStamp after);

    //! Returns the path where the executable binary is located (ends with the path separator)
    virtual std::string GetBasePath();

    //! Returns the data path (containing textures, levels, helpfiles, etc)
    virtual std::string GetDataPath();

    //! Returns the translations path
    virtual std::string GetLangPath();

    //! Returns the save dir location
    virtual std::string GetSaveDir();

    //! Returns the environment variable with the given name or an empty string if it does not exist
    virtual std::string GetEnvVar(const std::string &name);

    //! Opens a path with default file browser
    /** \returns true if successful */
    virtual bool OpenPath(const std::string& path);

    //! Opens a website with default web browser
    /** \returns true if successful */
    virtual bool OpenWebsite(const std::string& url);

    //! Sleep for given amount of microseconds
    void Usleep(int usecs);

private:
    std::string m_basePath;
    std::vector<std::unique_ptr<SystemTimeStamp>> m_timeStamps;
};
