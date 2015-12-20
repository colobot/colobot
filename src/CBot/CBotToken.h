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

/////////////////////////////////////////////////////////////////////////////////////
// Token management (tokens)

#define TokenTypKeyWord 1    // a keyword of the language (see TokenKeyWord)
#define TokenTypNum     2    // number
#define TokenTypString  3    // string
#define TokenTypVar     4    // a variable name
#define TokenTypDef     5    // value according DefineNum

#define TokenKeyWord    2000 // keywords of the language
#define TokenKeyDeclare 2100 // keywords of declarations (int, float,..)
#define TokenKeyVal     2200 // keywords representing the value (true, false, null, nan)
#define TokenKeyOp      2300 // operators

#define MAXDEFNUM 1000 // limited number of DefineNum

/**
 * \class CBotToken
 * Responsible for token management. A CBot program is a text string. This string
 * is first transformed into a list of token. It will only treat the case as an
 * error where there is an illegal character in a string.
 * For example :
 *     int var = 3 * ( pos.y + x )
 *  is decomposed into (each line is a token)
 *      int
 *      var
 *      =
 *      3
 *      *
 *      (
 *      pos.y
 *      +
 *      x
 *      )
 */

class CBotToken
{

public:

    /*!
     * \brief CBotToken Default Constructor.
     */
    CBotToken();
    CBotToken(const CBotToken* pSrc);
    CBotToken(const CBotString& mot,
              const CBotString& sep,
              int start=0,
              int end=0);
    CBotToken(const char* mot, const char* sep = nullptr);

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
    CBotString GetString();

    /*!
     * \brief SetString Set the token string.
     * \param [in] name The new string to set.
     */
    void SetString(const char* name);

    /*!
     * \brief GetSep Return the token separator.
     * \return The token separator a separator has been set. An empty separator
     *  otherwise.
     */
    CBotString GetSep();

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
    static CBotToken* CompileTokens(const char* p, int& error);

    /*!
     * \brief NextToken Looking for the next token in the string. The string must
     * not start with separators. The separator is part of the previous token.
     * \param [in] program The program string.
     * \param [out] error The error code.
     * \param [in] first True if this is the first call false othewise.
     * \return A CBotTOken.
     */
    static CBotToken* NextToken(char* &program, int& error, bool first = false);

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
    static bool DefineNum(const char* name, long val);

    /*!
     * \brief Free Free the array created with DefineNum.
     * \see DefineNum
     */
    static void Free();

private:

    //! The next token in the linked list
    CBotToken* m_next; // following in the list
    //! The previous token in the linked list
    CBotToken* m_prev;
    //! The token type
    int m_type; // type of Token
    //! The id of the keyword
    long m_IdKeyWord;

    //! The token string
    CBotString m_Text;
    //! The token separator
    CBotString m_Sep;

    //! The strat position of the token in the CBotProgram
    int m_start;
    //! The end position of the token in the CBotProgram
    int m_end;

    /*!
     * \brief GetKeyWords Check if the word is a keyword.
     * \param w The word to compare.
     * \return -1 if this is not a keyword the keyword number otherwise.
     */
    static int GetKeyWords(const char* w);    // is it a keyword?

    /*!
     * \brief GetKeyDefNum Check if this is a defined word and set the defined
     * word type in a CBotToken.
     * \param [in] w The word to compaire.
     * \param [out] token The token in which the type will be set.
     * \return True if the defined word is found false otherwise.
     */
    static bool GetKeyDefNum(const char* w, CBotToken* &token);

    /*!
     * \brief LoadKeyWords Loads the list of keywords. The list of keyword is
     * CBotString::s_keywordString. This keywords are keywords languages (if, +,
     * for, while, case, extern ...)
     * \todo Fixme Figure out how this should work.
     */
    static void LoadKeyWords();

    //! List of keywords of the CBot language (if, +, for, while, case, extern ...)
    static std::vector<CBotString> m_ListKeyWords;
    //! List of id correponding to the keywords of the CBot language
    static int m_ListIdKeyWords[200];

    //! List of CBot language error and list of colobot specific keywords
    //! This keywords are defined in :
    //!      - void CScriptFunctions::Init()
    //!      - void CBotProgram::Init()
    static std::vector<CBotString> m_ListKeyDefine;
    //! List of id correponding to the defined words
    static long m_ListKeyNums[MAXDEFNUM];

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
