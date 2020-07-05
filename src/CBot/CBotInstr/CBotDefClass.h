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
 * \brief Definition of class instance variable
 *
 * Examples:
 * \code
 * ClassName varname;
 * ClassName varname();
 * ClassName varname = new ClassName();
 * ClassName varname = new ClassName(args);
 * ClassName varname1(), varname2();
 * ClassName varname1 = new ClassName(), varname2;
 * \endcode
 */
class CBotDefClass : public CBotInstr
{

public:
    CBotDefClass();
    ~CBotDefClass();

    /*!
     * \brief Compile Definition of pointer (s) to an object style CPoint A, B ;
     * \param p
     * \param pStack
     * \param pClass
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass = nullptr);

    /*!
     * \brief Execute Declaration of the instance of a class, for example:
     * CPoint A, B;
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
    virtual const std::string GetDebugName() override { return "CBotClassInstr"; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks() override;

private:

    //! Variable to initialise.
    CBotInstr* m_var;
    //! Parameters to be evaluated for the contructor.
    CBotInstr* m_parameters;
    //! A value to put, if there is.
    CBotInstr* m_expr;
    //! Has it parameters.
    bool m_hasParams;
    //! Constructor method unique identifier
    long m_nMethodeIdent;

    //! Instruction to chain method calls after constructor
    CBotInstr* m_exprRetVar;

};

} // namespace CBot
