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


#include "common/logger.h"
#include "common/version.h"

#include <stdio.h>

CLogger::CLogger()
{
    m_logLevel = Version::DEVELOPMENT_BUILD
        ? LOG_DEBUG
        : LOG_INFO;
}

CLogger::~CLogger()
{
    for (FILE* out : m_outputs)
    {
        fclose(out);
    }
}

void CLogger::LogMessage(LogLevel type, std::string_view message)
{
    if (type < m_logLevel)
        return;

    std::string line;

    switch (type)
    {
        case LOG_TRACE:
            line += "[TRACE]: ";
            break;
        case LOG_DEBUG:
            line += "[DEBUG]: ";
            break;
        case LOG_WARN:
            line += "[WARN]: ";
            break;
        case LOG_INFO:
            line += "[INFO]: ";
            break;
        case LOG_ERROR:
            line += "[ERROR]: ";
            break;
        default:
            break;
    }

    line += message;

    if (line.empty() || line.back() != '\n')
        line += '\n';

    for (FILE* out : m_outputs)
    {
        fputs(line.c_str(), out);
    }
}

void CLogger::AddOutput(FILE* file)
{
    assert(file != nullptr);
    m_outputs.push_back(file);
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

