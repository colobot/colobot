/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

CBotInstr* CompileExprLitNum(CBotToken* &p, CBotCStack* pStack);

CBotInstr* CompileSizeOf(CBotToken* &p, CBotCStack* pStack);

/**
 * \brief A number literal - 5, 1, 2.5, 3.75, etc. or a predefined numerical constant (see CBotToken::DefineNum())
 *
 * Can be of type ::CBotTypInt, ::CBotTypLong, ::CBotTypFloat, or ::CBotTypDouble
 */
template <typename T>
class CBotExprLitNum : public CBotInstr
{

public:
    // To keep linter happy, instead of = delete (see https://stackoverflow.com/a/37593094)
    CBotExprLitNum(T val) { static_assert(sizeof(T) == 0, "Only specializations of CBotExprLitNum can be used"); };

    ~CBotExprLitNum();

    /*!
     * \brief Execute Execute, returns the corresponding number.
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
    virtual const std::string GetDebugName() override { return "CBotExprLitNum"; }
    virtual std::string GetDebugData() override;

private:
    //! The type of number.
    CBotType m_numtype;
    //! Value
    T m_value;

};

} // namespace CBot
