/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

namespace CBot
{

/**
 * \brief CBotVar subclass for managing 1-byte integer values (::CBotTypByte)
 */
class CBotVarByte : public CBotVarInteger<signed char, CBotTypByte>
{
public:
    CBotVarByte(const CBotToken &name) : CBotVarInteger(name) {}

    void SR(CBotVar* left, CBotVar* right) override
    {
        SetValByte(static_cast<unsigned char>(left->GetValByte()) >> right->GetValInt());
    }

    bool Save1State(std::ostream &ostr) override
    {
        return WriteByte(ostr, m_val);
    }
};

} // namespace CBot
