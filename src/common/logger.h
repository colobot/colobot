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

// logger.h


#pragma once

#include <string>
#include <cstdarg>

#include <common/singleton.h>

/**
 *  @file common/logger.h
 *  @brief Class for loggin information to file or console
 */


/**
 * \public
 * \enum    LogType common/logger.h
 * \brief   Enum representing log level
**/
enum LogType
{
    LOG_INFO = 1, /*!< lowest level, information */
    LOG_WARN = 2, /*!< warning */
    LOG_ERROR = 3, /*!< error */
    LOG_NONE = 4 /*!< none level, used for custom messages */
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
         * @param const char str - message to write
         * @param ... - additional arguments
         */
        void Message(const char *str, ...);

        /** Write message to console or file with LOG_INFO level
         * @param const char str - message to write
         * @param ... - additional arguments
         */
        void Info(const char *str, ...);

        /** Write message to console or file with LOG_WARN level
         * @param const char str - message to write
         * @param ... - additional arguments
         */
        void Warn(const char *str, ...);

        /** Write message to console or file with LOG_ERROR level
         * @param const char str - message to write
         * @param ... - additional arguments
         */
        void Error(const char *str, ...);

        /** Set output file to write logs to
         * @param std::string filename - output file to write to
         */
        void SetOutputFile(std::string filename);

        /** Set log level. Logs with level below will not be shown
         * @param LogType level - minimum log level to write
         */
        void SetLogLevel(LogType level);

        static CLogger& GetInstance();
        static CLogger* GetInstancePointer();

    private:
        std::string mFilename;
        FILE *mFile;
        LogType mLogLevel;

        void Open();
        void Close();
        bool IsOpened();
        void Log(LogType type, const char* str, va_list args);
};


//! Global function to get Logger instance
inline CLogger* GetLogger() {
    return CLogger::GetInstancePointer();
}
