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

#pragma once

#include <string>

/**
 * \enum Language
 * \brief Application language
 */
enum Language
{
    LANGUAGE_ENV = -1,
    LANGUAGE_ENGLISH = 0,
    LANGUAGE_FRENCH = 1,
    LANGUAGE_GERMAN = 2,
    LANGUAGE_POLISH = 3,
    LANGUAGE_RUSSIAN = 4,
    LANGUAGE_CZECH = 5,
    LANGUAGE_PORTUGUESE_BRAZILIAN = 6,
    LANGUAGE_SIMPLIFIED_CHINESE = 7
};

bool ParseLanguage(const std::string& str, Language& language);
bool LanguageToString(const Language& language, std::string& str);
