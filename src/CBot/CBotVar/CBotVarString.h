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
#include "CBotDll.h"

#include "CBotVar/CBotVar.h"

// Local include

// Global include


/*!
 * \brief The CBotVarString class Class for management of strings (String).
 */
class CBotVarString : public CBotVar
{
public:

    /*!
     * \brief CBotVarString
     * \param name
     */
    CBotVarString( const CBotToken* name );

    /*!
     * \brief SetValString
     * \param p
     */
    void SetValString(const char* p) override;

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
     * \brief Add
     * \param left
     * \param right
     */
    void Add(CBotVar* left, CBotVar* right) override;

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
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

private:
    //! The value.
    CBotString m_val;
};
