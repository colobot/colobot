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

// Modules inlcude
#include "CBot/CBotInstr/CBotInstr.h"

// Local include

// Global include


/*!
 * \brief The CBotInstrMethode class A call of method. Compile a method call.
 */
class CBotInstrMethode : public CBotInstr
{
public:

    /*!
     * \brief CBotInstrMethode
     */
    CBotInstrMethode();

    /*!
     * \brief ~CBotInstrMethode
     */
    ~CBotInstrMethode();

    /*!
     * \brief Compile
     * \param p
     * \param pStack
     * \param pVar
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* pVar);

    /*!
     * \brief Execute
     * \param pj
     * \return
     */
    bool Execute(CBotStack* &pj) override;

    /*!
     * \brief ExecuteVar Execute the method call.
     * \param pVar
     * \param pj
     * \param prevToken
     * \param bStep
     * \param bExtend
     * \return
     */
    bool ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, bool bStep, bool bExtend) override;

    /*!
     * \brief RestoreStateVar
     * \param pj
     * \param bMain
     */
    void RestoreStateVar(CBotStack* &pj, bool bMain) override;

private:
    //! The parameters to be evaluated.
    CBotInstr *m_Parameters;
    //! Complete type of the result.
    CBotTypResult m_typRes;
    //! Name of the method.
    CBotString m_NomMethod;
    //! Identifier of the method.
    long m_MethodeIdent;
    //! Name of the class.
    CBotString m_ClassName;
};
