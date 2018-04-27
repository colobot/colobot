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

#include "CBot/CBotTypResult.h"

#include <cstdio>
#include <string>
#include <cassert>

namespace CBot
{

class CBotVar;
class CBotToken;
class CBotCStack;

/*!
 * \brief MakeListVars Transforms the array of pointers to variables in a
 * chained list of variables
 * \param ppVars
 * \param bSetVal
 * \return
 */
CBotVar* MakeListVars(CBotVar** ppVars, bool bSetVal=false);

/*!
 * \brief TypeParam
 * \param p
 * \param pile
 * \return
 */
CBotTypResult TypeParam(CBotToken* &p, CBotCStack* pile);

/*!
 * \brief ArrayType
 * \param p
 * \param pile
 * \param type
 * \return
 */
CBotTypResult ArrayType(CBotToken* &p, CBotCStack* pile, CBotTypResult type);

/*!
 * \brief WriteWord
 * \param pf
 * \param w
 * \return
 */
bool WriteWord(FILE* pf, unsigned short w);

/*!
 * \brief WriteString
 * \param pf
 * \param s
 * \return
 */
bool WriteString(FILE* pf, std::string s);

/*!
 * \brief WriteFloat
 * \param pf
 * \param w
 * \return
 */
bool WriteFloat(FILE* pf, float w);

/*!
 * \brief GetNumInt Converts a string into integer may be of the form 0xabc123.
 * \param str
 * \return
 */
long GetNumInt(const std::string& str);

/*!
 * \brief GetNumFloat Converts a string into a float number.
 * \param str
 * \return
 */
float GetNumFloat(const std::string& str);

/*!
 * \brief Search a null-terminated string for a char value.
 * \param c The char to find.
 * \param list The string to search.
 * \return true if the char is found.
 */
bool CharInList(const char c, const char* list);

/*!
 * \brief Converts a Unicode code point to UTF-8 encoded character.
 * \param val Code point value.
 * \return UTF-8 encoded string or empty string.
 */
std::string CodePointToUTF8(unsigned int val);

template<typename T> class CBotLinkedList
{
public:
    /**
     * \brief Destructor. Be careful, destroys the whole linked list!
     */
    virtual ~CBotLinkedList()
    {
        if (m_next != nullptr)
        {
            delete m_next;
            m_next = nullptr;
        }
    }

    /**
     * \brief Returns the next variable in the linked list
     * \return Next element in the list, or nullptr if this was the last element
     */
    T* GetNext()
    {
        return m_next;
    }

    /**
     * \brief Appends a new element at the end of the linked list
     * \param elem Element to add
     */
    void AddNext(T* elem)
    {
        CBotLinkedList<T>* p = this;
        while (p->m_next != nullptr) p = p->m_next;
        p->m_next = elem;
    }

protected:
    T* m_next = nullptr;
};

template<typename T> class CBotDoublyLinkedList
{
public:
    /**
     * \brief Destructor. Be careful, destroys the whole linked list!
     */
    virtual ~CBotDoublyLinkedList()
    {
        assert(m_prev == nullptr);

        if (m_next != nullptr)
        {
            m_next->m_prev = nullptr;
            delete m_next;
            m_next = nullptr;
        }
    }

    /**
     * \brief Returns the next variable in the linked list
     * \return Next element in the list, or nullptr if this was the last element
     */
    T* GetNext()
    {
        return m_next;
    }

    /**
     * \brief Returns the previous variable in the linked list
     * \return Previous element in the list, or nullptr if this was the last element
     */
    T* GetPrev()
    {
        return m_prev;
    }

    /**
     * \brief Appends a new element at the end of the linked list
     * \param elem Element to add
     */
    void AddNext(T* elem)
    {
        CBotDoublyLinkedList<T>* p = this;
        while (p->m_next != nullptr) p = p->m_next;
        p->m_next = elem;
        elem->m_prev = p;
    }

protected:
    T* m_next = nullptr;
    T* m_prev = nullptr;
};

} // namespace CBot
