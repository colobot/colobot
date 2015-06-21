/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
 * \file object/level/parserexceptions.h
 * \brief Exceptions that could be thrown in level parser
 */

#pragma once

#include <exception>
#include <string>

class CLevelParserParam;

class CLevelParserException : public std::exception
{
public:
    CLevelParserException(std::string message) NOEXCEPT;
    virtual ~CLevelParserException() NOEXCEPT {}
    const char* what() const NOEXCEPT;

protected:
    std::string m_message;
};

class CLevelParserExceptionMissingParam : public CLevelParserException
{
public:
    CLevelParserExceptionMissingParam(CLevelParserParam* thisParam) NOEXCEPT;
    virtual ~CLevelParserExceptionMissingParam() NOEXCEPT {}
};

class CLevelParserExceptionBadParam : public CLevelParserException
{
public:
    CLevelParserExceptionBadParam(CLevelParserParam* thisParam, std::string requestedType) NOEXCEPT;
    virtual ~CLevelParserExceptionBadParam() NOEXCEPT {}
};
