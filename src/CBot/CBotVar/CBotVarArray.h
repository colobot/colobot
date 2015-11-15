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
#include "CBot.h"
#include "CBotDefines.h"

// Local include

// Global include


/*!
 * \brief The CBotVarArray class Classe pour les tableaux.
 */
class CBotVarArray : public CBotVar
{
public:

    /*!
     * \brief CBotVarArray
     * \param name
     * \param type
     */
    CBotVarArray( const CBotToken* name, CBotTypResult& type );

    /*!
     * \brief ~CBotVarArray
     */
    ~CBotVarArray();

    /*!
     * \brief SetPointer
     * \param p
     */
    void SetPointer(CBotVar* p) override;

    /*!
     * \brief GetPointer
     * \return
     */
    CBotVarClass* GetPointer() override;

    /*!
     * \brief Copy Copy a variable into another.
     * \param pSrc
     * \param bName
     */
    void Copy(CBotVar* pSrc, bool bName=true) override;

    /*!
     * \brief GetItem Makes an element according to its numeric index enlarged
     * the table if necessary if bExtend.
     * \param n
     * \param bGrow
     * \return
     */
    CBotVar* GetItem(int n, bool bGrow=false) override;

    /*!
     * \brief GetItemList Gives the first item in the list.
     * \return
     */
    CBotVar* GetItemList() override;

    /*!
     * \brief GetValString Gets the contents of the array into a string.
     * \return
     */
    CBotString GetValString() override;

    /*!
     * \brief Save1State
     * \param pf
     * \return
     */
    bool Save1State(FILE* pf) override;

private:
    //! Instance manager of table.
    CBotVarClass* m_pInstance;
    //! My daddy is a buddy.
    friend class CBotVar;
};
