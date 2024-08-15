/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"

#include <vector>
#include <string>
#include <map>
#include <memory>

namespace CBot
{

/**
 * \brief Class representing one token of a program.
 *
 * A CBot program starts as a text string. This string is first transformed into a list of tokens.
 *
 * \section Tokens Example tokens
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
 *
 * \section Usage Example usage
 * \code
 * std::unique_ptr<CBotToken> tokens = CBotToken::CompileTokens(program);
 * CBotToken* token = tokens.get();
 * while(token != nullptr)
 * {
 *     printf("%s\n", token->GetString());
 *
 *     token = token->GetNext();
 * }
 * \endcode
 */

class CBotToken : public CBotDoublyLinkedList<CBotToken>
{
public:
    /**
     * \brief Default constructor
     */
    CBotToken();
    /**
     * \brief Copy constructor
     */
    CBotToken(const CBotToken& pSrc);
    /**
     * \brief Constructor
     *
     * \param text The string this token represents
     * \param sep All separators that appeared after this token
     * \param start Beginning location in the source code of this token
     * \param end Ending location in the source code of this token
     */
    CBotToken(const std::string& text,
              const std::string& sep = "",
              int start = 0,
              int end = 0);

    /**
     * \brief Destructor
     *
     * Be careful! This destroys the whole linked list of tokens
     *
     * Never call in the middle of the sequence - always on the first token in the list
     */
    ~CBotToken();

    /**
     * \brief Return the token type or the keyword id
     * \return A value from ::TokenType. For ::TokenTypKeyWord, returns the keyword ID instead.
     */
    int GetType();

    /**
     * \brief Return the token string
     * \return The string associated with this token
     */
    const std::string& GetString() const;

    /**
     * \brief Set the token string
     * \param name The new string to set
     */
    void SetString(const std::string& name);

    /**
     * \brief Return the beginning location of this token in the original program string
     */
    int GetStart();

    /**
     * \brief Return the ending location of this token in the original program string
     */
    int GetEnd();

    /**
     * \brief SetPos Set the token position in the CBot program
     * \param start The start position of the token
     * \param end The end position of the token
     */
    void SetPos(int start, int end);

    /**
     * \brief Get the keyword id
     * \return The keyword id, see ::TokenId
     */
    long GetKeywordId();

    /**
     * \brief Copy operator
     */
    const CBotToken& operator=(const CBotToken& src);

    /**
     * \brief Transforms a CBot program from a string to a list of tokens
     * \param prog The program string
     * \return The first token in the linked list
     */
    static std::unique_ptr<CBotToken> CompileTokens(const std::string& prog);

    /**
     * \brief Define a new constant
     * \param name Name of the constant
     * \param val Value of the constant
     * \return true on success, false if already defined
     */
    static bool DefineNum(const std::string& name, long val);

    /**
     * \brief Clear the list of defined constants
     * \see DefineNum()
     */
    static void ClearDefineNum();

private:
    /**
     * \brief Find the next token in the string
     *
     * The string must not start with separators. The separator is part of the previous token.
     *
     * \param [in, out] program The program string, modified to point at the next token
     * \param first true if this is the first call (beginning of the program string)
     * \return A processed CBotToken
     */
    static CBotToken* NextToken(const char*& program, bool first);

private:
    //! The token type
    TokenType m_type = TokenTypVar;
    //! The id of the keyword
    long m_keywordId = -1;

    //! The token string
    std::string m_text = "";
    //! The separator that appeared after this token
    std::string m_sep = "";

    //! The strat position of the token in the CBotProgram
    int m_start = 0;
    //! The end position of the token in the CBotProgram
    int m_end = 0;

    //! Map of all defined constants (see DefineNum())
    static std::map<std::string, long> m_defineNum;

    /**
     * \brief Check if the word is a keyword
     * \param w The word to check
     * \return the keyword ID (::CBotTokenId), or -1 if this is not a keyword
     */
    static int GetKeyWord(const std::string& w);

    /**
     * \brief Resolve a constant defined with DefineNum()
     * \param name Constant name
     * \param token Token that we are working on, will be filled with data about found constant
     * \return true if the constant was found, false otherwise
     */
    static bool GetDefineNum(const std::string& name, CBotToken* token);
};

/**
 * \brief Check if this token is of specified type
 * \param p The token to compare
 * \param type1 First token type to comapre to the token
 * \param type2 Second token type to comapre to the token
 * \return true if the type of the token matches one of the parameters
 */
extern bool IsOfType(CBotToken* &p, int type1, int type2 = -1);

/**
 * \brief Check if this token is of specified type
 * \param p The token to compare
 * \param type1 The list of token types to comapre to the token, 0-terminated
 * \return true if the type of the tokens matched one of the parameters
 */
extern bool IsOfTypeList(CBotToken* &p, int type1, ...);

/**
 * \brief Return the value associated with TX_UNDEF TokenId
 *
 * Mapping other TokenIds to its string values was not used and does not seem useful
 * so the only supported mapping is TX_UNDEF -> value.
 *
 * \return String associated with TX_UNDEF TokenId
 */
const std::string& UndefinedTokenString();

} // namespace CBot
