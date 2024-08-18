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

#include "CBot/CBotVar/CBotVarValue.h"

#include <common/codepoint.h>
#include <common/stringutils.h>

namespace CBot
{

/**
 * \brief CBotVar subclass for managing 32-bit Unicode values (::CBotTypChar)
 */
class CBotVarChar : public CBotVarInteger<uint32_t, CBotTypChar>
{
public:
    CBotVarChar(const CBotToken &name) : CBotVarInteger(name) {}

    std::string GetValString() const override
    {
        if (m_binit == CBotVar::InitType::UNDEF)
            return UndefinedTokenString();

        if (0x10FFFF < m_val || (0xD7FF < m_val && m_val < 0xE000))
            return "\xEF\xBF\xBD"; // replacement character U+FFFD

        return StrUtils::ToUTF8(m_val).Data();
    }

    void SR(CBotVar* left, CBotVar* right) override
    {
        SetValChar(left->GetValChar() >> right->GetValInt());
    }

    bool Save1State(std::ostream &ostr) override
    {
        return WriteUInt32(ostr, m_val);
    }
};

} // namespace CBot
