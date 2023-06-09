/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "common/regex_utils.h"

static std::string FormatAssertRegexMatchError(const std::string& text,
                                        const std::string& pattern)
{
    return "Text \"" + text + "\" did not match regex \"" + pattern + "\"";
}

RegexUtils::CAssertRegexMatchError::CAssertRegexMatchError(
    const std::string& text, const std::string& pattern) NOEXCEPT
    : std::runtime_error(FormatAssertRegexMatchError(text, pattern))
{
}

std::smatch RegexUtils::AssertRegexMatch(const std::string& text, const std::string& pattern)
{
    std::regex regex(pattern);
    std::smatch matches;
    bool ok = std::regex_match(text, matches, regex);
    if (!ok)
        throw CAssertRegexMatchError(text, pattern);

    return matches;
}
