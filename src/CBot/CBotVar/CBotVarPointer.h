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
#include "CBot/CBotVar/CBotVar.h"

// Local include

// Global include


/*!
 * \brief The CBotVarPointer class Class for the management of pointers to a
 * class instances.
 */
class CBotVarPointer : public CBotVar
{
public:

    /*!
     * \brief CBotVarPointer
     * \param name
     * \param type
     */
    CBotVarPointer( const CBotToken* name, CBotTypResult& type );

    /*!
     * \brief ~CBotVarPointer
     */
    ~CBotVarPointer();

    /*!
     * \brief Copy Copy a variable into another.
     * \param pSrc
     * \param bName
     */
    void Copy(CBotVar* pSrc, bool bName=true) override;

    /*!
     * \brief SetClass
     * \param pClass
     */
    void SetClass(CBotClass* pClass) override;

    /*!
     * \brief GetClass
     * \return
     */
    CBotClass* GetClass() override;

    /*!
     * \brief GetItem Return an element of a class according to its name (*).
     * \param name
     * \return
     */
    CBotVar* GetItem(const std::string& name) override;

    /*!
     * \brief GetItemRef
     * \param nIdent
     * \return
     */
    CBotVar* GetItemRef(int nIdent) override;

    /*!
     * \brief GetItemList
     * \return
     */
    CBotVar* GetItemList() override;

    /*!
     * \brief GetValString
     * \return
     */
    std::string GetValString() override;

    /*!
     * \brief SetPointer Initializes the pointer to the instance of a class.
     * \param p
     */
    void SetPointer(CBotVar* p) override;

    /*!
     * \brief GetPointer
     * \return
     */
    CBotVarClass* GetPointer() override;

    /*!
     * \brief SetIdent Associates an identification number (unique).
     * \param n
     */
    void SetIdent(long n) override;

    /*!
     * \brief GetIdent Gives the identification number associated with.
     * \return
     */
    long GetIdent();

    /*!
     * \brief ConstructorSet
     */
    void ConstructorSet() override;

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

    /*!
     * \brief Maj
     * \param pUser
     * \param bContinue
     */
    void Maj(void* pUser, bool bContinue) override;

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

private:
    //! Contents.
    CBotVarClass* m_pVarClass;
    //! Class provided for this pointer.
    CBotClass* m_pClass;
    friend class CBotVar;
};
