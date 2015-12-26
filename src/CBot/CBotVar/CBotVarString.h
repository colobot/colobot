/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

/**
 * \brief CBotVar subclass for managing string values (::CBotTypString)
 */
class CBotVarString : public CBotVar
{
public:
    /**
     * \brief Constructor. Do not call directly, use CBotVar::Create()
     */
    CBotVarString(const CBotToken& name);

    void SetValString(const std::string& val) override;
    std::string GetValString() override;

    void Copy(CBotVar* pSrc, bool bName = true) override;

    void Add(CBotVar* left, CBotVar* right) override;

    bool Lo(CBotVar* left, CBotVar* right) override;
    bool Hi(CBotVar* left, CBotVar* right) override;
    bool Ls(CBotVar* left, CBotVar* right) override;
    bool Hs(CBotVar* left, CBotVar* right) override;
    bool Eq(CBotVar* left, CBotVar* right) override;
    bool Ne(CBotVar* left, CBotVar* right) override;

    bool Save1State(FILE* pf) override;

private:
    //! The value.
    std::string m_val;
};

} // namespace CBot
