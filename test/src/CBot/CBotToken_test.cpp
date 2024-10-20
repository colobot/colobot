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

#include "CBot/CBotToken.h"
#include "CBot/CBotProgram.h"

#include <gtest/gtest.h>

using namespace CBot;

class CBotTokenUT : public testing::Test
{
public:
    CBotTokenUT()
    {
        CBotProgram::Init();
    }

    ~CBotTokenUT()
    {
        CBotProgram::Free();
    }

protected:
    struct TokenTest
    {
        std::string str;
        int type;
    };

    void ExecuteTest(const std::string& code, std::vector<TokenTest> data)
    {
        auto tokens = CBotToken::CompileTokens(code);
        ASSERT_TRUE(tokens != nullptr);
        CBotToken* token = tokens.get()->GetNext(); // TODO: why do we always have to skip the first one :/
        ASSERT_TRUE(token != nullptr);
        unsigned int i = 0;
        do
        {
            ASSERT_LT(i, data.size()) << "too many tokens processed";

            TokenTest correct = data[i];
            ASSERT_EQ(token->GetString(), correct.str) << "string mismatch at token #" << (i+1);
            ASSERT_EQ(token->GetType(), correct.type) << "type mismatch at token #" << (i+1);
            i++;
        }
        while((token = token->GetNext()) != nullptr && !IsOfType(token, TokenTypNone));
        ASSERT_EQ(i, data.size()) << "not enough tokens processed";
    }
};

TEST_F(CBotTokenUT, CodeExample)
{
    // this is the code example shown in the class documentation
    ExecuteTest("\tint var = 3 * ( pos.y + x );\n\tstring test = \"Hello world\";", {
        {"int",             ID_INT},
        {"var",             TokenTypVar},
        {"=",               ID_ASS},
        {"3",               TokenTypNum},
        {"*",               ID_MUL},
        {"(",               ID_OPENPAR},
        {"pos",             TokenTypVar},
        {".",               ID_DOT},
        {"y",               TokenTypVar},
        {"+",               ID_ADD},
        {"x",               TokenTypVar},
        {")",               ID_CLOSEPAR},
        {";",               ID_SEP},
        {"string",          ID_STRING},
        {"test",            TokenTypVar},
        {"=",               ID_ASS},
        {"\"Hello world\"", TokenTypString},
        {";",               ID_SEP},
    });
}

TEST_F(CBotTokenUT, IgnoreComments)
{
    ExecuteTest(" /*comment*/ int /* comment*/x = 5; //comment", {
        {"int", ID_INT},
        {"x",   TokenTypVar},
        {"=",   ID_ASS},
        {"5",   TokenTypNum},
        {";",   ID_SEP},
    });
}

TEST_F(CBotTokenUT, BasicProgram)
{
    ExecuteTest("extern void object::TestProgram()\n{\n\t\n\tmessage(\"test\"+2.0);\n\t\n}\n", {
        {"extern",      ID_EXTERN},
        {"void",        ID_VOID},
        {"object",      TokenTypVar},
        {"::",          ID_DBLDOTS},
        {"TestProgram", TokenTypVar},
        {"(",           ID_OPENPAR},
        {")",           ID_CLOSEPAR},
        {"{",           ID_OPBLK},
        {"message",     TokenTypVar},
        {"(",           ID_OPENPAR},
        {"\"test\"",    TokenTypString},
        {"+",           ID_ADD},
        {"2.0",         TokenTypNum},
        {")",           ID_CLOSEPAR},
        {";",           ID_SEP},
        {"}",           ID_CLBLK},
    });
}
