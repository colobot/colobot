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

#include "CBot/CBotString.h"

#include <vector>

/*!
 * \brief The CBotStringArray class Class used to arrays of strings management.
 * TODO: refactor code to use std::vector instead
 */
class CBotStringArray : public CBotString
{
public:

    /*!
     * \brief CBotStringArray
     */
    CBotStringArray();

    /*!
     * \brief ~CBotStringArray
     */
    ~CBotStringArray();

    /*!
     * \brief SetSize Set the array size.
     * \param nb
     */
    void SetSize(int nb);

    /*!
     * \brief GetSize
     * \return
     */
    int GetSize();

    /*!
     * \brief Add
     * \param str
     */
    void Add(const CBotString& str);

    /*!
     * \brief operator []
     * \param nIndex
     * \return
     */
    CBotString& operator[](int nIndex);

    /*!
     * \brief ElementAt
     * \param nIndex
     * \return
     */
    CBotString& ElementAt(int nIndex);

private:

    std::vector<CBotString> m_data;
};
