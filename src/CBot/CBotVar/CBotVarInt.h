/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \brief CBotVar subclass for managing integer values (::CBotTypInt)
 */
class CBotVarInt : public CBotVarInteger<int, CBotTypInt>
{
public:
    CBotVarInt(const CBotToken &name) : CBotVarInteger(name) {}

    void SetValInt(int val, const std::string& s = "") override;
    std::string GetValString() override;

    void Copy(CBotVar* pSrc, bool bName = true) override;

    void Neg() override;
    void Inc() override;
    void Dec() override;
    void Not() override;

    void SR(CBotVar* left, CBotVar* right) override;

    bool Save0State(std::ostream &ostr) override;
    bool Save1State(std::ostream &ostr) override;

protected:

    void SetValue(int val) override
    {
        CBotVarNumberBase::SetValue(val);
        m_defnum.clear();
    }

protected:
    //! The name if given by DefineNum.
    std::string m_defnum;
    friend class CBotVar;
};

} // namespace CBot
