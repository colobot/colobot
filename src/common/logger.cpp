/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "common/logger.h"

#include <stdio.h>


template<> CLogger* CSingleton<CLogger>::m_instance = nullptr;


CLogger::CLogger()
{
    m_file = nullptr;
    #if DEV_BUILD
    m_logLevel = LOG_DEBUG;
    #else
    m_logLevel = LOG_INFO;
    #endif
}

CLogger::~CLogger()
{
    Close();
}

void CLogger::Log(LogLevel type, const char* str, va_list args)
{
    if (type < m_logLevel)
        return;

    switch (type)
    {
        case LOG_TRACE:
            fprintf(IsOpened() ? m_file : stderr, "[TRACE]: ");
            break;
        case LOG_DEBUG:
            fprintf(IsOpened() ? m_file : stderr, "[DEBUG]: ");
            break;
        case LOG_WARN:
            fprintf(IsOpened() ? m_file : stderr, "[WARN]: ");
            break;
        case LOG_INFO:
            fprintf(IsOpened() ? m_file : stderr, "[INFO]: ");
            break;
        case LOG_ERROR:
            fprintf(IsOpened() ? m_file : stderr, "[ERROR]: ");
            break;
        default:
            break;
    }

    vfprintf(IsOpened() ? m_file : stderr, str, args);
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

void CLogger::Log(LogLevel logLevel, const char* str, ...)
{
    va_list args;
    va_start(args, str);
    Log(logLevel, str, args);
    va_end(args);
}

void CLogger::SetOutputFile(std::string filename)
{
    m_filename = filename;
    Open();
}

void CLogger::Open()
{
    m_file = fopen(m_filename.c_str(), "w");

    if (m_file == nullptr)
        fprintf(stderr, "Could not create file %s\n", m_filename.c_str());
}

void CLogger::Close()
{
    if (IsOpened())
        fclose(m_file);
}

bool CLogger::IsOpened()
{
    return m_file != nullptr;
}

void CLogger::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
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

