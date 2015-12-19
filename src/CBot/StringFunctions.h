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
#include "CBot/CBotTypResult.h"

// Local include

// Global include

// Forward declaration
class CBotVar;

/*!
 * \brief rStrLen Gives the length of a chain execution
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrLen( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief cIntStr int xxx ( string ) compilation
 * \param pVar
 * \param pUser
 * \return
 */
CBotTypResult cIntStr( CBotVar* &pVar, void* pUser );

/*!
 * \brief rStrLeft Gives the left side of a chain execution
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrLeft( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief cStrStrInt string xxx ( string, int ) compilation
 * \param pVar
 * \param pUser
 * \return
 */
CBotTypResult cStrStrInt( CBotVar* &pVar, void* pUser );

/*!
 * \brief rStrRight Gives the right of a string execution
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrRight( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief rStrMid Gives the central part of a chain execution
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrMid( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief rStrVal Gives the number stored in a string execution.
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrVal( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief cIntStrStr int xxx ( string, string ) compilation
 * \param pVar
 * \param pUser
 * \return
 */
CBotTypResult cIntStrStr( CBotVar* &pVar, void* pUser );

/*!
 * \brief rStrUpper Gives a string to uppercase exécution
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrUpper( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief rStrLower Gives a string to lowercase exécution.
 * \param pVar
 * \param pResult
 * \param ex
 * \param pUser
 * \return
 */
bool rStrLower( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser );

/*!
 * \brief cStrStr String xxx ( string ) compilation
 * \param pVar
 * \param pUser
 * \return
 */
CBotTypResult cStrStr( CBotVar* &pVar, void* pUser );

/*!
 * \brief InitStringFunctions
 */
void InitStringFunctions();
