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

#include "CBot/CBotToken.h"
#include "CBot/CBotCStack.h"

#include <vector>

namespace CBot
{
class CBotDebug;

/*
    for example, the following program
    int        x[]; x[1] = 4;
    int        y[x[1]][10], z;
    is generated
    CBotInstrArray
    m_next3b-> CBotEmpty
    m_next->
    CBotExpression ....
    m_next->
    CBotInstrArray
    m_next3b-> CBotExpression ('x') ( m_next3-> CBotIndexExpr ('1') )
    m_next3b-> CBotExpression ('10')
    m_next2-> 'z'
    m_next->...

*/
/**
 * \brief Class for one CBot instruction
 *
 * \todo More documentation
 */
class CBotInstr
{
public:
    /**
     * \brief Constructor
     */
    CBotInstr();

    /**
     * \brief Destructor
     */
    virtual ~CBotInstr();

    /**
     * \brief Compile an instruction.
     *
     * Supported instructions are:
     * * while
     * * do
     * * try
     * * throw
     * * if
     * * for
     * * switch
     * * break
     * * continue
     * * return
     * * int
     * * float
     * * boolean
     * * string
     * * declaration of an instance of a class
     * * arithmetic expression (with or without assigment)
     * \param[in, out] p Token to start at, updated to point at the next token
     * \param pStack Compilation stack
     * \return Compiled instruction
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack);

    /**
     * \brief CompileArray
     * \param p
     * \param pStack
     * \param type
     * \param first
     * \return
     */
    static CBotInstr* CompileArray(CBotToken* &p,
                                   CBotCStack* pStack,
                                   CBotTypResult type,
                                   bool first = true);

    /**
     * \brief Execute
     * \param pj
     * \return
     */
    virtual bool Execute(CBotStack* &pj);

    /**
     * \brief Execute
     * \param pj
     * \param pVar
     * \return
     */
    virtual bool Execute(CBotStack* &pj,
                         CBotVar* pVar);

    /**
     * \brief RestoreState
     * \param pj
     * \param bMain
     */
    virtual void RestoreState(CBotStack* &pj,
                              bool bMain);

    /**
     * \brief ExecuteVar
     * \param pVar
     * \param pile
     * \return
     */
    virtual bool ExecuteVar(CBotVar* &pVar,
                            CBotCStack* &pile);

    /**
     * \brief ExecuteVar
     * \param pVar
     * \param pile
     * \param prevToken
     * \param bStep
     * \param bExtend
     * \return
     */
    virtual bool ExecuteVar(CBotVar* &pVar,
                            CBotStack* &pile,
                            CBotToken* prevToken,
                            bool bStep,
                            bool bExtend);

    /**
     * \brief RestoreStateVar
     * \param pile
     * \param bMain
     */
    virtual void RestoreStateVar(CBotStack* &pile,
                                 bool bMain);

    /**
     * \brief CompCase This routine is defined only for the subclass CBotCase
     * this allows to make the call on all instructions CompCase to see if it's
     * a case to the desired value..
     * \param pj
     * \param val
     * \return
     */
    virtual bool CompCase(CBotStack* &pj,
                          int val);

    /**
     * \brief SetToken Set the token corresponding to the instruction.
     * \param p
     */
    void SetToken(CBotToken* p);

    /**
     * \brief GetTokenType Return the type of the token assicated with the
     * instruction.
     * \return
     */
    int GetTokenType();

    /**
     * \brief GetToken Return associated token.
     * \return
     */
    CBotToken* GetToken();

    /**
     * \brief AddNext Adds the statement following the other.
     * \param n
     */
    void AddNext(CBotInstr* n);

    /**
     * \brief GetNext Returns next statement.
     * \return
     */
    CBotInstr* GetNext();

    /**
     * \brief AddNext3
     * \param n
     */
    void AddNext3(CBotInstr* n);

    /**
     * \brief GetNext3
     * \return
     */
    CBotInstr* GetNext3();

    /**
     * \brief AddNext3b
     * \param n
     */
    void AddNext3b(CBotInstr* n);

    /**
     * \brief GetNext3b
     * \return
     */
    CBotInstr* GetNext3b();

    /**
     * \brief IncLvl Adds a level with a label.
     * \param label
     */
    static void IncLvl(std::string& label);

    /**
     * \brief IncLvl Adds a level (switch statement).
     */
    static void IncLvl();

    /**
     * \brief DecLvl Free a level.
     */
    static void DecLvl();

    /**
     * \brief ChkLvl Control validity of break and continue.
     * \param label
     * \param type
     * \return
     */
    static bool ChkLvl(const std::string& label, int type);

protected:
    friend class CBotDebug;
    virtual const std::string GetDebugName() = 0;
    virtual std::string GetDebugData() { return ""; }
    virtual std::map<std::string, CBotInstr*> GetDebugLinks();

protected:

    //! Keeps the token.
    CBotToken m_token;
    //! Linked command.
    CBotInstr* m_next;
    //! Second list definition chain.
    CBotInstr* m_next2b;
    //! Third list for indices and fields.
    CBotInstr* m_next3;
    //! Necessary for reporting tables.
    CBotInstr* m_next3b;

    //! Counter of nested loops, to determine the break and continue valid.
    static int m_LoopLvl;
    friend class CBotClassInst;
    friend class CBotInt;
    friend class CBotListArray;

private:
    //! List of labels used.
    static std::vector<std::string> m_labelLvl;
};

} // namespace CBot
