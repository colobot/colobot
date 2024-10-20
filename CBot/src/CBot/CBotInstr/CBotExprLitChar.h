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

#include "CBot/CBotInstr/CBotInstr.h"

#include <cstdint>

namespace CBot
{

/**
 * \brief A character literal
 * \verbatim 'a', '\n', '\t', '\uFFFD', '\U0000FFFD', etc. \endverbatim
 */
class CBotExprLitChar : public CBotInstr
{
public:
    CBotExprLitChar(char32_t valchar);

    ~CBotExprLitChar();

    /*!
     * \brief Compile a character literal
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute, returns the corresponding char.
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief RestoreState
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotExprLitChar"; }
    virtual std::string GetDebugData() override;

private:
    char32_t m_valchar = 0;
};

} // namespace CBot
