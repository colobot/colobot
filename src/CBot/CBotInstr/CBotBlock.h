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
 * \brief The CBotBlock class An instruction block { .... }.
 */
class CBotBlock : public CBotInstr
{
public:

    /*!
     * \brief Compile Compiles a statement block " { i ; i ; } "
     * \param p
     * \param pStack
     * \param bLocal
     * \return
     */
    static CBotInstr* Compile(CBotToken* &p, CBotCStack* pStack, bool bLocal = true);

    /*!
     * \brief CompileBlkOrInst
     * \param p
     * \param pStack
     * \param bLocal
     * \return
     */
    static CBotInstr* CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, bool bLocal = false);

private:

    /*!
     * \brief CBotBlock This class have no constructor because there is never an
     * instance of this class the object returned by Compile is usually of type
     * CBotListInstr
     */
    CBotBlock() = delete;
    CBotBlock(const CBotBlock &) = delete;
};
