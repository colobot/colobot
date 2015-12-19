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
 * \brief The CBotVarClass class Class management class instances.
 */
class CBotVarClass : public CBotVar
{
public:

    /*!
     * \brief CBotVarClass
     * \param name
     * \param type
     */
    CBotVarClass( const CBotToken* name, const CBotTypResult& type );

    /*!
     * \brief ~CBotVarClass
     */
    ~CBotVarClass();

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
    CBotVar* GetItem(const char* name) override;

    /*!
     * \brief GetItemRef
     * \param nIdent
     * \return
     */
    CBotVar* GetItemRef(int nIdent) override;

    /*!
     * \brief GetItem For the management of an array.
     * \param n
     * \param bExtend can enlarge the table, but not beyond the threshold size
     * of SetArray().
     * \return
     */
    CBotVar* GetItem(int n, bool bExtend) override;

    /*!
     * \brief GetItemList
     * \return
     */
    CBotVar* GetItemList() override;

    /*!
     * \brief GetValString
     * \return
     */
    CBotString GetValString() override;

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
     * \brief IncrementUse A reference to incrementation.
     */
    void IncrementUse();

    /*!
     * \brief DecrementUse A reference to decrementation.
     */
    void DecrementUse();

    /*!
     * \brief GetPointer
     * \return
     */
    CBotVarClass* GetPointer() override;

    /*!
     * \brief SetItemList
     * \param pVar
     */
    void SetItemList(CBotVar* pVar);

    /*!
     * \brief SetIdent
     * \param n
     */
    void SetIdent(long n) override;

    /*!
     * \brief Find Makes an instance according to its unique number.
     * \param id
     * \return
     */
    static CBotVarClass* Find(long id);

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
     * \brief ConstructorSet
     */
    void ConstructorSet() override;

private:
    //! List of existing instances at some point.
    static CBotVarClass* m_ExClass;
    //! For this general list.
    CBotVarClass* m_ExNext;
    //! For this general list.
    CBotVarClass* m_ExPrev;
    //! The class definition.
    CBotClass* m_pClass;
    //! The instance of a parent class.
    CBotVarClass* m_pParent;
    //! Contents.
    CBotVar* m_pVar;
    //! Counter usage.
    int m_CptUse;
    //! Identifier (unique) of an instance.
    long m_ItemIdent;
    //! Set if a constructor has been called.
    bool m_bConstructor;

    friend class CBotVar;
    friend class CBotVarPointer;
};
