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

#include "level/parser/parserexceptions.h"

#include "level/parser/parser.h"

#include "common/stringutils.h"


static std::string FormatMissingParamError(CLevelParserParam* thisParam) noexcept
{
    auto paramName = thisParam->GetName();
    auto lineNumber = StrUtils::ToString(thisParam->GetLine()->GetLineNumber());
    auto fileName = StrUtils::ToString(thisParam->GetLine()->GetLevelFilename());
    return "Missing required param '" + paramName + "' (in " + fileName + ":" + lineNumber + ")";
}

CLevelParserExceptionMissingParam::CLevelParserExceptionMissingParam(CLevelParserParam* thisParam) noexcept
: CLevelParserException(FormatMissingParamError(thisParam))
{
}

static std::string FormatBadParamError(CLevelParserParam* thisParam, std::string requestedType) noexcept
{
    auto paramName = thisParam->GetName();
    auto paramValue = thisParam->GetValue();
    auto lineNumber = StrUtils::ToString(thisParam->GetLine()->GetLineNumber());
    auto fileName = StrUtils::ToString(thisParam->GetLine()->GetLevelFilename());
    return "Unable to parse '" + paramValue + "' as " + requestedType + " (param '" + paramName + "' in " + fileName + ":" + lineNumber + ")";
}

CLevelParserExceptionBadParam::CLevelParserExceptionBadParam(CLevelParserParam* thisParam, std::string requestedType) noexcept
: CLevelParserException(FormatBadParamError(thisParam, requestedType))
{
}
