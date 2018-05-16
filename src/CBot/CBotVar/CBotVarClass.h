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

#include "CBot/CBotVar/CBotVar.h"

#include <set>

namespace CBot
{

/**
 * \brief CBotVar subclass for managing classes (::CBotTypClass, ::CBotTypIntrinsic)
 *
 * \nosubgrouping
 */
class CBotVarClass : public CBotVar
{
public:
    /**
     * \brief Constructor. Do not call directly, use CBotVar::Create()
     */
    CBotVarClass(const CBotToken& name, const CBotTypResult& type);
    /**
     * \brief Destructor. Do not call directly, use CBotVar::Destroy()
     */
    ~CBotVarClass();

    void Copy(CBotVar* pSrc, bool bName = true) override;

    void SetClass(CBotClass* pClass) override;
    CBotClass* GetClass() override;

    CBotVar* GetItem(const std::string& name) override;
    CBotVar* GetItemRef(int nIdent) override;
    CBotVar* GetItem(int n, bool bExtend) override;
    CBotVar* GetItemList() override;
    std::string GetValString() override;

    bool Save1State(FILE* pf) override;

    void Update(void* pUser) override;

    //! \name Reference counter
    //@{

    /**
     * \brief Increment reference counter
     */
    void IncrementUse();

    /**
     * \brief Decrement reference counter
     */
    void DecrementUse();

    //@}

    CBotVarClass* GetPointer() override;

    //! \name Unique instance identifier
    //@{

    void SetIdent(long n) override;

    /*!
     * \brief Finds a class instance by unique identifier
     * \param id Identifier to find
     * \return Found class instance
     */
    static CBotVarClass* Find(long id);

    //@}

    bool Eq(CBotVar* left, CBotVar* right) override;
    bool Ne(CBotVar* left, CBotVar* right) override;

    void ConstructorSet() override;

private:
    //! List of all class instances - first
    static std::set<CBotVarClass*> m_instances;
    //! Class definition
    CBotClass* m_pClass;
    //! Parent class instance
    CBotVarClass* m_pParent;
    //! Class members
    CBotVar* m_pVar;
    //! Reference counter
    int m_CptUse;
    //! Identifier (unique) of an instance
    long m_ItemIdent;
    //! Set after constructor is called, allows destructor to be called
    bool m_bConstructor;

    friend class CBotVar;
    friend class CBotVarPointer;
};

} // namespace CBot
