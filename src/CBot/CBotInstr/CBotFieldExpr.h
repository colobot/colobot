/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \brief Accessing a class field using dot operator - toto.x
 */
class CBotFieldExpr : public CBotInstr
{
public:
    CBotFieldExpr();
    ~CBotFieldExpr();

    /*!
     * \brief SetUniqNum
     * \param num
     */
    void SetUniqNum(int num);

    /*!
     * \brief ExecuteVar Find a field from the instance at compile.
     * \param pVar
     * \param pile
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotCStack* &pile) override;

    /*!
     * \brief ExecuteVar
     * \param pVar
     * \param pile
     * \param prevToken
     * \param bStep
     * \param bExtend
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, bool bStep, bool bExtend) override;

    /*!
     * \brief RestoreStateVar
     * \param pj
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pj, bool bMain) override;

    /*!
     * \brief ProtectionError Test if access to a variable is not allowed.
     * \param pStack
     * \param pPrev
     * \param pVar
     * \param privat
     * \return True if pVar is protected in the current context.
     */
    static bool ProtectionError(CBotCStack* pStack, CBotVar* pPrev, CBotVar* pVar,
                                CBotVar::ProtectionLevel privat = CBotVar::ProtectionLevel::Protected);

protected:
    virtual const std::string GetDebugName() override { return "CBotFieldExpr"; }
    virtual std::string GetDebugData() override;

private:
    friend class CBotExpression;
    int m_nIdent;
};

} // namespace CBot
