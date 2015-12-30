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

#include "CBot/CBotDefines.h"

#include "CBot/CBotInstr/CBotInstr.h"
#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

/**
 * \brief Expression representing a variable name
 *
 * Verifies that variable is known during compilation
 * Verifies taht variable is initialized during execution
 */
class CBotExprVar : public CBotInstr
{
public:
    CBotExprVar();
    ~CBotExprVar();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \param privat
     * \return
     */
    static CBotInstr* Compile(CBotToken*& p, CBotCStack* pStack,
                              CBotVar::ProtectionLevel privat = CBotVar::ProtectionLevel::Protected);

    /*!
     * \brief CompileMethode
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* CompileMethode(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Execute, making the value of a variable.
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

    /*!
     * \brief ExecuteVar Fetch a variable at runtime.
     * \param pVar
     * \param pile
     * \param prevToken
     * \param bStep
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep);

    /*!
     * \brief RestoreStateVar Fetch variable at runtime.
     * \param pj
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() { return "CBotExprVar"; }
    virtual std::string GetDebugData();

private:
    long m_nIdent;
    friend class CBotPostIncExpr;
    friend class CBotPreIncExpr;

};

} // namespace CBot
