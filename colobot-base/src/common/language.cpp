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

#include "common/language.h"

#include <map>

const std::map<Language, std::string> LANGUAGE_MAP = {
    { LANGUAGE_CZECH,   "cs" },
    { LANGUAGE_ENGLISH, "en" },
    { LANGUAGE_GERMAN,  "de" },
    { LANGUAGE_FRENCH,  "fr" },
    { LANGUAGE_POLISH,  "pl" },
    { LANGUAGE_RUSSIAN, "ru" },
    { LANGUAGE_PORTUGUESE_BRAZILIAN, "pt" },
    { LANGUAGE_SIMPLIFIED_CHINESE, "zh_CN"}
};

bool ParseLanguage(const std::string& str, Language& language)
{
    for (auto it = LANGUAGE_MAP.begin(); it != LANGUAGE_MAP.end(); ++it)
    {
        if (it->second == str)
        {
            language = it->first;
            return true;
        }
    }

    return false;
}

bool LanguageToString(const Language& language, std::string& str)
{
    if (LANGUAGE_MAP.count(language) > 0)
    {
        str = LANGUAGE_MAP.at(language);
        return true;
    }
    return false;
}
