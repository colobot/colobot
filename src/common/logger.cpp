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


#include "common/logger.h"

#include <stdio.h>


template<> CLogger* CSingleton<CLogger>::m_instance = nullptr;


CLogger::CLogger()
{
    mFile = NULL;
    mLogLevel = LOG_INFO;
}


CLogger::~CLogger()
{
    Close();
}


void CLogger::Log(LogLevel type, const char* str, va_list args)
{
    if (type < mLogLevel)
        return;

    switch (type)
    {
        case LOG_TRACE:
            fprintf(IsOpened() ? mFile : stderr, "[TRACE]: ");
            break;
        case LOG_DEBUG:
            fprintf(IsOpened() ? mFile : stderr, "[DEBUG]: ");
            break;
        case LOG_WARN:
            fprintf(IsOpened() ? mFile : stderr, "[WARN]: ");
            break;
        case LOG_INFO:
            fprintf(IsOpened() ? mFile : stderr, "[INFO]: ");
            break;
        case LOG_ERROR:
            fprintf(IsOpened() ? mFile : stderr, "[ERROR]: ");
            break;
        default:
            break;
    }

    vfprintf(IsOpened() ? mFile : stderr, str, args);
}


void CLogger::Trace(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_TRACE, str, args);
    va_end(args);
}


void CLogger::Debug(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_DEBUG, str, args);
    va_end(args);
}


void CLogger::Info(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_INFO, str, args);
    va_end(args);
}


void CLogger::Warn(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_WARN, str, args);
    va_end(args);
}


void CLogger::Error(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_ERROR, str, args);
    va_end(args);
}


void CLogger::Message(const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(LOG_NONE, str, args);
    va_end(args);
}


void CLogger::SetOutputFile(std::string filename)
{
    mFilename = filename;
    Open();
}


void CLogger::Open()
{
    mFile = fopen(mFilename.c_str(), "w");

    if (mFile == NULL)
        fprintf(stderr, "Could not create file %s\n", mFilename.c_str());
}


void CLogger::Close()
{
    if (IsOpened())
        fclose(mFile);
}


bool CLogger::IsOpened()
{
    return mFile != NULL;
}


void CLogger::SetLogLevel(LogLevel type)
{
    mLogLevel = type;
}


bool CLogger::ParseLogLevel(const std::string& str, LogLevel& logLevel)
{
    if (str == "trace")
    {
        logLevel = LOG_TRACE;
        return true;
    }
    else if (str == "debug")
    {
        logLevel = LOG_DEBUG;
        return true;
    }
    else if (str == "info")
    {
        logLevel = LOG_INFO;
        return true;
    }
    else if (str == "warn")
    {
        logLevel = LOG_WARN;
        return true;
    }
    else if (str == "error")
    {
        logLevel = LOG_ERROR;
        return true;
    }
    else if (str == "none")
    {
        logLevel = LOG_NONE;
        return true;
    }

    return false;
}

