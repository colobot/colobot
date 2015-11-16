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
#include "CBot.h"

#include "CBotInstr.h"

// Local include

// Global include

/*!
 * \brief The CBotClassInst class Definition of an element of any class.
 */
class CBotClassInst : public CBotInstr
{

public:

    /*!
     * \brief CBotClassInst
     */
    CBotClassInst();

    /*!
     * \brief ~CBotClassInst
     */
    ~CBotClassInst();

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

private:

    //! Variable to initialise.
    CBotInstr* m_var;
    //! Reference to the class.
    CBotClass* m_pClass;
    //! Parameters to be evaluated for the contructor.
    CBotInstr* m_Parameters;
    //! A value to put, if there is.
    CBotInstr* m_expr;
    //! Has it parameters.
    bool m_hasParams;
    long m_nMethodeIdent;

};
