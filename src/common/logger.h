// * This file is part of the COLOBOT source code
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
 *  \file common/logger.h
 *  \brief Class for logging information to file or console
 */

#pragma once


#include "common/singleton.h"

#include <string>
#include <cstdarg>
#include <cstdio>


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
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Message(const char *str, ...);

    /** Write message to console or file with LOG_TRACE level
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Trace(const char *str, ...);

    /** Write message to console or file with LOG_DEBUG level
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Debug(const char *str, ...);

    /** Write message to console or file with LOG_INFO level
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Info(const char *str, ...);

    /** Write message to console or file with LOG_WARN level
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Warn(const char *str, ...);

    /** Write message to console or file with LOG_ERROR level
    * \param str - message to write
    * \param ... - additional arguments
    */
    void Error(const char *str, ...);

    /** Set output file to write logs to
    * \param filename - output file to write to
    */
    void SetOutputFile(std::string filename);

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
    std::string mFilename;
    FILE *mFile;
    LogLevel mLogLevel;

    void Open();
    void Close();
    bool IsOpened();
    void Log(LogLevel type, const char* str, va_list args);
};


//! Global function to get Logger instance
inline CLogger* GetLogger() {
    return CLogger::GetInstancePointer();
}
