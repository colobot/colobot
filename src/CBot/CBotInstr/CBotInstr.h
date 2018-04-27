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

#include "CBot/CBotToken.h"
#include "CBot/CBotCStack.h"

#include <vector>

namespace CBot
{
class CBotDebug;

/**
 * \brief Class for one CBot instruction
 *
 * For example, for program:
 * \code
 * int x[]; x[1] = 4;
 * int y[x[1]][10], z;
 * \endcode
 * the following structure is generated:
 * \dot
 * # Generated using the CBot_compile_graph tool
 * # and slightly modified
 * digraph {
 * start [label=<START> shape=box3d color=cyan]
 * instr00000000015304D0 [label=<<b>CBotInstrArray</b><br/>int[]>]
 * instr0000000001530870 [label=<<b>CBotExpression</b>>]
 * instr0000000001530920 [label=<<b>CBotLeftExpr</b><br/>x>]
 * instr00000000015309D0 [label=<<b>CBotIndexExpr</b>>]
 * instr0000000001530DC0 [label=<<b>CBotExprLitNum</b><br/>(int) 1>]
 * instr00000000015309D0 -> instr0000000001530DC0 [label="m_expr" weight=5]
 * instr0000000001530920 -> instr00000000015309D0 [label="m_next3" weight=5]
 * instr0000000001530870 -> instr0000000001530920 [label="m_leftop" weight=5]
 * instr0000000001530E80 [label=<<b>CBotInstrArray</b><br/>int[][]>]
 * instr00000000015315F0 [label=<<b>CBotDefInt</b>>]
 * instr0000000001531C20 [label=<<b>CBotLeftExprVar</b><br/>z>]
 * instr00000000015315F0 -> instr0000000001531C20 [label="m_var" weight=5]
 * instr0000000001530E80 -> instr00000000015315F0 [label="m_next2b" weight=5]
 * { rank=same; instr0000000001530E80; instr00000000015315F0; }
 * instr0000000001530B50 [label=<<b>CBotExprVar</b><br/>x>]
 * instr0000000001531700 [label=<<b>CBotIndexExpr</b>>]
 * instr0000000001531B60 [label=<<b>CBotExprLitNum</b><br/>(int) 1>]
 * instr0000000001531700 -> instr0000000001531B60 [label="m_expr" weight=5]
 * instr0000000001530B50 -> instr0000000001531700 [label="m_next3" weight=5]
 * instr0000000001531A00 [label=<<b>CBotExprLitNum</b><br/>(int) 10>]
 * instr0000000001530B50 -> instr0000000001531A00 [label="m_next3b" weight=5]
 * instr0000000001530E80 -> instr0000000001530B50 [label="m_next3b" weight=5]
 * instr0000000001530A80 [label=<<b>CBotLeftExprVar</b><br/>y>]
 * instr0000000001530E80 -> instr0000000001530A80 [label="m_var" weight=5]
 * instr0000000001530870 -> instr0000000001530E80 [label="m_next" weight=1]
 * { rank=same; instr0000000001530870; instr0000000001530E80; }
 * instr0000000001530C80 [label=<<b>CBotExprLitNum</b><br/>(int) 4>]
 * instr0000000001530870 -> instr0000000001530C80 [label="m_rightop" weight=5]
 * instr00000000015304D0 -> instr0000000001530870 [label="m_next" weight=1]
 * { rank=same; instr00000000015304D0; instr0000000001530870; }
 * instr0000000001530670 [label=<<b>CBotEmpty</b>>]
 * instr00000000015304D0 -> instr0000000001530670 [label="m_next3b" weight=5]
 * instr00000000015305A0 [label=<<b>CBotLeftExprVar</b><br/>x>]
 * instr00000000015304D0 -> instr00000000015305A0 [label="m_var" weight=5]
 * { rank=same; start; instr00000000015304D0; }
 * start -> instr00000000015304D0
 * }
 * \enddot
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

    /**
     * \brief Check a list of instructions for a return statement.
     * \return true if a return statement was found.
     */
    virtual bool HasReturn();

protected:
    friend class CBotDebug;
    /**
     * \brief Returns the name of this class
     * \see CBotDebug
     */
    virtual const std::string GetDebugName() = 0;
    /**
     * \brief Returns additional data associated with this instruction for debugging purposes
     * \see CBotDebug
     */
    virtual std::string GetDebugData() { return ""; }
    /**
     * Returns a map of all instructions connected with this one
     * \see CBotDebug
     */
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
    friend class CBotDefClass;
    friend class CBotDefInt;
    friend class CBotListArray;

private:
    //! List of labels used.
    static std::vector<std::string> m_labelLvl;
};

} // namespace CBot
