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
 * \brief A call to a function - func()
 *
 * \see CBotInstrMethode for class methods
 */
class CBotInstrCall : public CBotInstr
{
public:
    CBotInstrCall();
    ~CBotInstrCall();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /*!
     * \brief Execute
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
    virtual const std::string GetDebugName() override { return "CBotInstrCall"; }
    virtual std::string GetDebugData() override;
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:
    //! The parameters to be evaluated.
    CBotInstr* m_parameters;
    //! Complete type of the result.
    CBotTypResult m_typRes;
    //! Id of a function.
    long m_nFuncIdent;

    //! Instruction to return a member of the returned object.
    CBotInstr* m_exprRetVar;

    friend class CBotDebug;
};

} // namespace CBot
