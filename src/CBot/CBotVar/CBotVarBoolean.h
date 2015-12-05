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
#include "CBot/CBotToken.h"

#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


/*!
 * \brief The CBotVarBoolean class Class for the management of boolean.
 */
class CBotVarBoolean : public CBotVar
{
public:

    /*!
     * \brief CBotVarBoolean
     * \param name
     */
    CBotVarBoolean( const CBotToken* name );

    /*!
     * \brief SetValInt
     * \param val
     * \param s
     */
    void SetValInt(int val, const char* s = nullptr) override;

    /*!
     * \brief SetValFloat
     * \param val
     */
    void SetValFloat(float val) override;

    /*!
     * \brief GetValInt
     * \return
     */
    int GetValInt() override;

    /*!
     * \brief GetValFloat
     * \return
     */
    float GetValFloat() override;

    /*!
     * \brief GetValString
     * \return
     */
    CBotString GetValString() override;

    /*!
     * \brief Copy Copy a variable into another.
     * \param pSrc
     * \param bName
     */
    void Copy(CBotVar* pSrc, bool bName=true) override;

    /*!
     * \brief And
     * \param left
     * \param right
     */
    void And(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Or
     * \param left
     * \param right
     */
    void Or(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief XOr
     * \param left
     * \param right
     */
    void XOr(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Not
     */
    void Not() override;

    /*!
     * \brief Eq
     * \param left
     * \param right
     * \return
     */
    bool Eq(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Ne
     * \param left
     * \param right
     * \return
     */
    bool Ne(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

private:
    //! The value.
    bool m_val;
};
