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

/**
 * \file common/system/system.h
 * \brief System functions: time stamps, info dialogs, etc.
 */

#pragma once

#include "common/config.h"

#include <chrono>
#include <filesystem>
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
    ERROR_MSG, // windows.h defines ERROR which collides with the "ERROR" enum name
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
 * \class CSystemUtils
 * \brief Platform-specific utils
 *
 * This class provides system-specific utilities like displaying user dialogs and
 * querying system timers for exact timestamps.
 */
class CSystemUtils
{
public:
    CSystemUtils();

    virtual ~CSystemUtils();

    //! Returns unique instance of system utilities
    static CSystemUtils& GetInstance();

    //! Performs platform-specific initialization
    virtual void Init(const std::vector<std::string>& args) = 0;

    //! Returns the number of arguments
    int GetArgumentCount() const;

    //! Returns the argument
    std::string GetArgument(int index) const;

    //! Returns all arguments
    const std::vector<std::string>& GetArguments() const;

    //! Displays a system dialog
    virtual SystemDialogResult SystemDialog(SystemDialogType, const std::string &title, const std::string &message) = 0;

    //! Displays a fallback system dialog using console
    TEST_VIRTUAL SystemDialogResult ConsoleSystemDialog(SystemDialogType type, const std::string& title, const std::string& message);

    //! Returns the path where the executable binary is located (ends with the path separator)
    virtual std::filesystem::path GetBasePath() const;

    //! Returns the data path (containing textures, levels, helpfiles, etc)
    virtual std::filesystem::path GetDataPath() const;

    //! Returns the translations path
    virtual std::filesystem::path GetLangPath() const;

    //! Returns the save dir location
    virtual std::filesystem::path GetSaveDir() const;

    //! Returns the environment variable with the given name or an empty string if it does not exist
    virtual std::string GetEnvVar(const std::string &name) const;

    //! Opens a path with default file browser
    /** \returns true if successful */
    virtual bool OpenPath(const std::filesystem::path& path);

    //! Opens a website with default web browser
    /** \returns true if successful */
    virtual bool OpenWebsite(const std::string& url);

    //! Checks if the game is running with a debugger
    /** \returns true if debugger is present */
    virtual bool IsDebuggerPresent() const;

    //! Sleep for given amount of microseconds
    void Usleep(int usecs);

protected:
    //! Creates system utils for specific platform
    static std::unique_ptr<CSystemUtils> Create();

    std::filesystem::path m_basePath;
    std::vector<std::string> m_arguments;

    inline static std::unique_ptr<CSystemUtils> m_instance = nullptr;
};
