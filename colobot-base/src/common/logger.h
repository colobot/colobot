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
 *  \file common/logger.h
 *  \brief Class for logging information to file or console
 */

#pragma once


#include "common/singleton.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>


/**
 * \public
 * \enum    LogLevel common/logger.h
 * \brief   Enum representing log level
**/
enum LogLevel
{
    LOG_TRACE = 1, /*!< lowest level, execution tracing */
    LOG_DEBUG = 2, /*!< debugging messages */
    LOG_INFO  = 3, /*!< information */
    LOG_WARN  = 4, /*!< warning */
    LOG_ERROR = 5, /*!< error */
    LOG_NONE  = 6  /*!< none level, used for custom messages */
};


/**
* @class CLogger
*
* @brief Class for loggin information to file or console
*
*/
class CLogger : public CSingleton<CLogger>
{
public:
    CLogger();
    ~CLogger();

    /** Write message to console or file
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Message(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_NONE, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with LOG_TRACE level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Trace(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_TRACE, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with LOG_DEBUG level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Debug(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_DEBUG, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with LOG_INFO level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Info(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_INFO, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with LOG_WARN level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Warn(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_WARN, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with LOG_ERROR level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Error(std::string_view message, Args&&... args)
    {
        Log(LogLevel::LOG_ERROR, message, std::forward<Args>(args)...);
    }

    /** Write message to console or file with given log level
    * \param logLevel - log level
    * \param message - message to write
    * \param args - additional arguments
    */
    template<typename... Args>
    void Log(LogLevel logLevel, std::string_view message, Args&&... args)
    {
        LogMessage(logLevel, FormatMessage(message, std::forward<Args>(args)...));
    }

    /** Adds output stream to write logs to
    * \param stream - stream to write to
    */
    void AddOutput(std::ostream& stream);

    /** Set log level. Logs with level below will not be shown
    * \param level - minimum log level to write
    */
    void SetLogLevel(LogLevel level);

    /** Parses string as a log level
     * \param str string to parse
     * \param logLevel result log level
     *
     * Valid values are "trace", "debug", "info", "warn", "error" and "none".
     * On invalid value, returns \c false.
     */
    static bool ParseLogLevel(const std::string& str, LogLevel& logLevel);

private:
    /** Write message to console or file with given log level
    * \param logLevel - log level
    * \param message - message to write
    */
    void LogMessage(LogLevel type, std::string_view message);

    template<typename... Args>
    std::string FormatMessage(std::string_view format, Args&&... args)
    {
        std::string result;

        auto print = [&](auto&& arg)
        {
            if (format.empty()) return;

            std::size_t index = format.find("%%");

            if (index == std::string_view::npos)
            {
                result.append(format);
                format = {};
                return;
            }

            result.append(format.substr(0, index));

            PrintValue(result, arg);

            format.remove_prefix(index + 2);
        };

        (print(args), ...);

        if (!format.empty())
            result.append(format);

        return result;
    }

    static void PrintValue(std::string& string, char value);
    static void PrintValue(std::string& string, signed char value);
    static void PrintValue(std::string& string, unsigned char value);

    static void PrintValue(std::string& string, bool value);

    static void PrintValue(std::string& string, short value);
    static void PrintValue(std::string& string, unsigned short value);
    static void PrintValue(std::string& string, int value);
    static void PrintValue(std::string& string, unsigned int value);
    static void PrintValue(std::string& string, long value);
    static void PrintValue(std::string& string, unsigned long value);
    static void PrintValue(std::string& string, long long value);
    static void PrintValue(std::string& string, unsigned long long value);

    static void PrintValue(std::string& string, float value);
    static void PrintValue(std::string& string, double value);
    
    static void PrintValue(std::string& string, const char* value);
    static void PrintValue(std::string& string, std::string_view value);
    static void PrintValue(std::string& string, const std::string& value);
    static void PrintValue(std::string& string, const std::filesystem::path& value);

    std::vector<std::ostream*> m_outputs;
    LogLevel m_logLevel;
};


//! Global function to get Logger instance
inline CLogger* GetLogger()
{
    return CLogger::GetInstancePointer();
}
