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

#include <vector>
#include <string>
#include <map>

#include "CBot/CBotEnums.h"

/**
 * \brief Class representing one token of a program.
 *
 * A CBot program starts as a text string. This string is first transformed into a list of tokens.
 *
 * \section Example Example
 * This code:
 * \code
 *     int var = 3 * ( pos.y + x );
 *     string test = "Hello world";
 * \endcode
 *
 * Is decomposed into (each line is a token, separate CBotToken instance):
 * \code
 *     int
 *     var
 *     =
 *     3
 *     *
 *     (
 *     pos
 *     .
 *     y
 *     +
 *     x
 *     )
 *     ;
 *     string
 *     test
 *     =
 *     "Hello world"
 *     ;
 * \endcode
 */

class CBotToken
{

public:

    /*!
     * \brief CBotToken Default Constructor.
     */
    CBotToken();
    CBotToken(const CBotToken* pSrc);
    CBotToken(const std::string& mot,
              const std::string& sep = "",
              int start=0,
              int end=0);

    /*!
     * \brief ~CBotToken Destructor. Be careful when you delete a CBotToken that
     *  is in a linked list all the following CBotToken will be deleted too.
     */
    ~CBotToken();

    /*!
     * \brief GetType Return the token type or the keyword id.
     * \return The token type or the keyword id.
     */
    int GetType();

    /*!
     * \brief GetString Return the token string.
     * \return The token string if a string has been set. An empty string
     * otherwise.
     */
    std::string GetString();

    /*!
     * \brief SetString Set the token string.
     * \param [in] name The new string to set.
     */
    void SetString(const std::string& name);

    /*!
     * \brief GetSep Return the token separator.
     * \return The token separator a separator has been set. An empty separator
     *  otherwise.
     */
    std::string GetSep();

    /*!
     * \brief GetStart Return the start position of the string token in the
     * original CBotProgram.
     * \return The start position of the string token or -1 if no string has
     * been set.
     */
    int GetStart();

    /*!
     * \brief GetEnd Return the end position of the string token in the
     * original CBotProgram.
     * \return The start position of the string token or -1 if no string has
     * been set.
     */
    int GetEnd();

    /*!
     * \brief GetNext Gives the next CBotToken in the list.
     * \return The next CBotToken if set nullptr otherwise.
     */
    CBotToken* GetNext();

    /*!
     * \brief GetPrev Gives the previous CBotToken in the list.
     * \return The previous CBotToken if set nullptr otherwise.
     */
    CBotToken* GetPrev();

    /*!
     * \brief SetPos Set the token position in the CBot program.
     * \param [in] start The start position of the token.
     * \param [in] end The end position of the token.
     */
    void SetPos(int start, int end);

    /*!
     * \brief GetIdKey Get the token id key.
     * \return The id key.
     */
    long GetIdKey();

    /*!
     * \brief operator =
     * \param [in] src The CBotToken to copy.
     * \return The CBotToken with the copied value.
     */
    const CBotToken& operator=(const CBotToken& src);

    /*!
     * \brief CompileTokens This function will transform the entire CBot program
     * in CBotToken. If an error occured during the processing an error number
     * will be set. Each CBotToken will be linked with the previous one and the
     * next one.
     * \param [in] p The program string.
     * \param [out] error The error code.
     * \return The first token of the linked liste.
     * \todo Replace the error code by an enum.
     */
    static CBotToken* CompileTokens(const std::string& p);

    /**
     * \brief Delete Releases the CBotToken linked list.
     * \deprecated This function is deprecated because it only delete a pointer.
     * \todo Remove this function.
     */
    static void Delete(CBotToken* pToken);

    /*!
     * \brief DefineNum This function define a language keyword with an associated
     * number.
     * \param [in] name The new word to define.
     * \param [in] val The number associated with the keyword.
     * \return Ture if the number is available false oterhwise.
     */
    static bool DefineNum(const std::string& name, long val);

    /*!
     * \brief Free Free the array created with DefineNum.
     * \see DefineNum
     */
    static void Free();

private:
    /*!
     * \brief NextToken Looking for the next token in the string. The string must
     * not start with separators. The separator is part of the previous token.
     * \param [in] program The program string.
     * \param [out] error The error code.
     * \param [in] first True if this is the first call false othewise.
     * \return A CBotTOken.
     */
    static CBotToken* NextToken(const char*& program, bool first);

private:

    //! The next token in the linked list
    CBotToken* m_next; // following in the list
    //! The previous token in the linked list
    CBotToken* m_prev;
    //! The token type
    TokenType m_type; // type of Token
    //! The id of the keyword
    long m_IdKeyWord;

    //! The token string
    std::string m_Text;
    //! The token separator
    std::string m_Sep;

    //! The strat position of the token in the CBotProgram
    int m_start;
    //! The end position of the token in the CBotProgram
    int m_end;

    //! Map of all predefined constants (see DefineNum())
    static std::map<std::string, long> m_defineNum;

    /*!
     * \brief Check if the word is a keyword
     * \param w The word to check
     * \return the keyword ID, or -1 if this is not a keyword
     */
    static int GetKeyWord(const std::string& w);

    /**
     * \brief Resolve a constant defined with DefineNum()
     * \param name Constant name
     * \param token Token that we are working on, will be filled with data about found constant
     * \return true if the constant was found, false otherwise
     */
    static bool GetKeyDefNum(const std::string& name, CBotToken* token);

};

/*!
 * \brief IsOfType This function tell if a token is of a specific type.
 * \param [in] p The token to compare.
 * \param [in] type1 First token type to comapre to the token.
 * \param [in] type2 Second token type to comapre to the token.
 * \return True if the type of the token match one of a parameter.
 */
extern bool IsOfType(CBotToken* &p, int type1, int type2 = -1);

/*!
 * \brief IsOfType This function tell if a token is of a specific type.
 * \param [in] p The token to compare.
 * \param [in] type1 The list of token type to comapre to the token.
 * \return True if the type of the token match one of a parameter.
 */
extern bool IsOfTypeList(CBotToken* &p, int type1, ...);




/**
 * \brief LoadString Maps given ID to its string equivalent.
 * \param id            Provided identifier.
 * \return              String if found, else NullString.
 */
const std::string& LoadString(TokenId id);
