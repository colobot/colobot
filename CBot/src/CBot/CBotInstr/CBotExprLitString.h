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

namespace CBot
{

/**
 * \brief A string literal - "Some text"
 */
class CBotExprLitString : public CBotInstr
{
public:
    CBotExprLitString();
    CBotExprLitString(const std::string& valstring) : m_valstring(valstring) {}

    ~CBotExprLitString();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Execute, returns the corresponding string.
     * \param pj
     * \return
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotExprLitString"; }
    virtual std::string GetDebugData() override;

private:
    std::string m_valstring = "";
};

} // namespace CBot
