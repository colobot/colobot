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
#include "CBotString.h"
#include "CBotTypResult.h"

// Local include

// Global include
#include <cstdio>

// Forward declaration
class CBotString;
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
bool WriteString(FILE* pf, CBotString s);

/*!
 * \brief WriteFloat
 * \param pf
 * \param w
 * \return
 */
bool WriteFloat(FILE* pf, float w);

/*!
 * \brief ConstructElement
 * \param pNewData
 */
void ConstructElement(CBotString* pNewData);

/*!
 * \brief DestructElement
 * \param pOldData
 */
void DestructElement(CBotString* pOldData);

/*!
 * \brief CopyElement
 * \param pSrc
 * \param pDest
 */
void CopyElement(CBotString* pSrc, CBotString* pDest);

/*!
 * \brief ConstructElements
 * \param pNewData
 * \param nCount
 */
void ConstructElements(CBotString* pNewData, int nCount);

/*!
 * \brief DestructElements
 * \param pOldData
 * \param nCount
 */
void DestructElements(CBotString* pOldData, int nCount);

/*!
 * \brief GetNumInt Converts a string into integer may be of the form 0xabc123.
 * \param p
 * \return
 */
long GetNumInt(const char* p);

/*!
 * \brief GetNumFloat Converts a string into a float number.
 * \param p
 * \return
 */
float GetNumFloat(const char* p);

