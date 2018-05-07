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

#include "CBot/CBotInstr/CBotInstr.h"

namespace CBot
{

/**
 * \brief Compilation of assignment of an array - {{1, 2, 3}, {3, 2, 1}}
 */
class CBotListArray : public CBotInstr
{
public:
    CBotListArray();
    ~CBotListArray();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \param type
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);

    /*!
     * \brief Execute Executes the definition of an array.
     * \param pj
     * \param pVar
     * \return
     */
    bool Execute(CBotStack* &pj, CBotVar* pVar) override;

    /*!
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    void RestoreState(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotListArray"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! An expression for an element others are linked with CBotInstr :: m_next3b;
    CBotInstr* m_expr;
};

} // namespace CBot
