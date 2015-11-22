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
#include "CBotVar/CBotVar.h"

// Local include

// Global include


/*!
 * \brief The CBotVarFloat class Class for managing real numbers (float).
 */
class CBotVarFloat : public CBotVar
{
public:

    /*!
     * \brief CBotVarFloat
     * \param name
     */
    CBotVarFloat( const CBotToken* name );

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
     * \brief Add Addition.
     * \param left
     * \param right
     */
    void Add(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Sub Substraction.
     * \param left
     * \param right
     */
    void Sub(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Mul Multiplication.
     * \param left
     * \param right
     */
    void Mul(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Div Division.
     * \param left
     * \param right
     * \return
     */
    int Div(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Modulo Remainder of division.
     * \param left
     * \param right
     * \return
     */
    int Modulo(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Power
     * \param left
     * \param right
     */
    void Power(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Lo
     * \param left
     * \param right
     * \return
     */
    bool Lo(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Hi
     * \param left
     * \param right
     * \return
     */
    bool Hi(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Ls
     * \param left
     * \param right
     * \return
     */
    bool Ls(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Hs
     * \param left
     * \param right
     * \return
     */
    bool Hs(CBotVar* left, CBotVar* right) override;

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
     * \brief Neg
     */
    void Neg() override;

    /*!
     * \brief Inc
     */
    void Inc() override;

    /*!
     * \brief Dec
     */
    void Dec() override;

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

private:
    //! The value.
    float m_val;
};
