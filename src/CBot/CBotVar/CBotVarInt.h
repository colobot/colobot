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
 * \brief CBotVar subclass for managing integer values (::CBotTypInt)
 */
class CBotVarInt : public CBotVarNumber<int, CBotTypInt>
{
public:
    CBotVarInt(const CBotToken &name) : CBotVarNumber(name) {}

    void SetValInt(int val, const std::string& s = "") override;
    std::string GetValString() override;

    void Copy(CBotVar* pSrc, bool bName = true) override;

    void Neg() override;
    void Inc() override;
    void Dec() override;

    void XOr(CBotVar* left, CBotVar* right) override;
    void Or(CBotVar* left, CBotVar* right) override;
    void And(CBotVar* left, CBotVar* right) override;
    void Not() override;

    void SL(CBotVar* left, CBotVar* right) override;
    void SR(CBotVar* left, CBotVar* right) override;
    void ASR(CBotVar* left, CBotVar* right) override;

    bool Save0State(FILE* pf) override;
    bool Save1State(FILE* pf) override;

protected:
    //! The name if given by DefineNum.
    std::string m_defnum;
    friend class CBotVar;
};

} // namespace CBot
