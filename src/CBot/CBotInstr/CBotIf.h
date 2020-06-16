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
 * \brief Instruction if (condition) { ... } else { ... }
 */
class CBotIf : public CBotInstr
{
public:
    CBotIf();
    ~CBotIf();

    /*!
     * \brief Compile Compilation (static routine) called when the token "if"
     * has been found
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute Execution of the instruction.
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

    /**
     * \brief Check 'if' and 'else' for return statements.
     * Returns true when 'if' and 'else' have return statements,
     * if not, the next block or instruction is checked.
     * \return true if a return statement is found.
     */
    bool HasReturn() override;

protected:
    virtual const std::string GetDebugName() override { return "CBotIf"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! Condition
    CBotInstr* m_condition;
    //! Instruction
    CBotInstr* m_block;
    //! Instruction
    CBotInstr* m_blockElse;
};

} // namespace CBot
