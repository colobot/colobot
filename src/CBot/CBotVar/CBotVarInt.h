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
#include <CBot/CBotEnums.h>
#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


/*!
 * \brief The CBotVarInt class Class for the management of integer numbers (int).
 */
class CBotVarInt : public CBotVar
{
public:

    /*!
     * \brief CBotVarInt
     * \param name
     */
    CBotVarInt( const CBotToken* name );

    /*!
     * \brief SetValInt
     * \param val
     * \param s
     */
    void SetValInt(int val, const std::string& s = nullptr) override;

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
    std::string GetValString() override;

    /*!
     * \brief Copy Copy a variable in to another.
     * \param pSrc
     * \param bName
     */
    void Copy(CBotVar* pSrc, bool bName=true) override;

    /*!
     * \brief Add
     * \param left
     * \param right
     */
    void Add(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Sub
     * \param left
     * \param right
     */
    void Sub(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Mul
     * \param left
     * \param right
     */
    void Mul(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Div
     * \param left
     * \param right
     * \return
     */
    CBotError Div(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Modulo
     * \param left
     * \param right
     * \return
     */
    CBotError Modulo(CBotVar* left, CBotVar* right) override;

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
     * \brief XOr
     * \param left
     * \param right
     */
    void XOr(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Or
     * \param left
     * \param right
     */
    void Or(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief And
     * \param left
     * \param right
     */
    void And(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief SL
     * \param left
     * \param right
     */
    void SL(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief SR
     * \param left
     * \param right
     */
    void SR(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief ASR
     * \param left
     * \param right
     */
    void ASR(CBotVar* left, CBotVar* right) override;

    /*!
     * \brief Neg
     */
    void Neg() override;

    /*!
     * \brief Not
     */
    void Not() override;

    /*!
     * \brief Inc
     */
    void Inc() override;

    /*!
     * \brief Dec
     */
    void Dec() override;

    /*!
     * \brief Save0State
     * \param pf
     * \return
     */
    bool Save0State(FILE* pf) override;

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

private:
    //! The value.
    int m_val;
    //! The name if given by DefineNum.
    std::string m_defnum;
    friend class CBotVar;
};
