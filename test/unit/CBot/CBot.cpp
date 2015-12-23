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

#include "CBot/CBot.h"

#include <gtest/gtest.h>

class CBotTestFail : public std::runtime_error {
public:
    CBotTestFail(const std::string& message) : runtime_error(message)
    {
    }

    CBotTestFail(std::string message, int cursor1, int cursor2) : CBotTestFail(message)
    {
        this->cursor1 = cursor1;
        this->cursor2 = cursor2;
    }

    int cursor1 = -1;
    int cursor2 = -1;
};

CBotTypResult cFail(CBotVar* &var, void* user)
{
    if (var != nullptr)
    {
        if (var->GetType() != CBotTypString) return CBotTypResult(CBotErrBadString);
        var = var->GetNext();
    }
    if (var != nullptr) return CBotTypResult(CBotErrOverParam);
    return CBotTypResult(CBotTypVoid);
}

bool rFail(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    std::string message = "CBot test failed";
    if (var != nullptr)
    {
        message = var->GetValString();
    }

    throw CBotTestFail(message);
}

class CBotUT : public testing::Test
{
public:
    void SetUp()
    {
        CBotProgram::Init();
        CBotProgram::AddFunction("FAIL", rFail, cFail);
    }

    void TearDown()
    {
        CBotProgram::Free();
    }

protected:
    // Modified version of PutList from src/script/script.cpp
    // Should be probably moved somewhere into the CBot library
    void PrintVars(std::stringstream& ss, CBotVar* var, const std::string& baseName = "", bool bArray = false)
    {
        if (var == nullptr && !baseName.empty())
        {
            ss << "    " << baseName << " = null" << std::endl;
            return;
        }

        int index = 0;
        while (var != nullptr)
        {
            CBotVar* pStatic = var->GetStaticVar();  // finds the static element

            std::string p = pStatic->GetName();  // variable name

            std::stringstream varName;
            if (baseName.empty())
            {
                varName << p;
            }
            else
            {
                if (bArray)
                {
                    varName << baseName << "[" << index << "]";
                }
                else
                {
                    varName << baseName << "." << p;
                }
            }

            CBotType type = pStatic->GetType();
            if ( type < CBotTypBoolean )
            {
                ss << "    " << varName.str() << " = " << pStatic->GetValString() << std::endl;
            }
            else if ( type == CBotTypString )
            {
                ss << "    " << varName.str() << " = " << "\"" << pStatic->GetValString() << "\"" << std::endl;
            }
            else if ( type == CBotTypArrayPointer )
            {
                PrintVars(ss, pStatic->GetItemList(), varName.str(), true);
            }
            else if ( type == CBotTypClass   ||
                      type == CBotTypPointer )
            {
                PrintVars(ss, pStatic->GetItemList(), varName.str(), false);
            }
            else
            {
                ss << "    " << varName.str() << " = ?" << std::endl;
            }

            index ++;
            var = var->GetNext();
        }
    }

    void ExecuteTest(const std::string& code, CBotError expectedError = CBotNoErr)
    {
        auto program = std::unique_ptr<CBotProgram>(new CBotProgram());
        std::vector<std::string> tests;
        program->Compile(code, tests);

        CBotError error;
        int cursor1, cursor2;
        if (program->GetError(error, cursor1, cursor2))
        {
            FAIL() << "Compile error - " << error << " (" << cursor1 << "-" << cursor2 << ")"; // TODO: Error messages are on Colobot side
        }

        for (const std::string& test : tests)
        {
            try
            {
                program->Start(test);
                while (!program->Run());
                program->GetError(error, cursor1, cursor2);
                if (error != expectedError)
                {
                    std::stringstream ss;
                    if (error != CBotNoErr)
                    {
                        ss << "RUNTIME ERROR - " << error; // TODO: Error messages are on Colobot side
                    }
                    else
                    {
                        ss << "No runtime error, expected " << expectedError; // TODO: Error messages are on Colobot side
                        cursor1 = cursor2 = -1;
                    }
                    throw CBotTestFail(ss.str(), cursor1, cursor2);
                }
            }
            catch (const CBotTestFail& e)
            {
                std::stringstream ss;
                ss << "*** Failed test " << test << ": " << e.what() << std::endl;

                std::string funcName;
                program->GetRunPos(funcName, cursor1, cursor2);
                if (!funcName.empty())
                {
                    ss << "    while executing function " << funcName << " (" << cursor1 << "-" << cursor2 << ")" << std::endl;
                }
                else if(e.cursor1 >= 0 && e.cursor2 >= 0)
                {
                    ss << "    at unknown location " << e.cursor1 << "-" << e.cursor2 << std::endl;
                }
                ss << std::endl;

                ss << "Variables:" << std::endl;
                int level = 0;
                while (true)
                {
                    CBotVar* var = program->GetStackVars(funcName, level--);
                    if (var == nullptr) break;

                    ss << "  Block " << -level << ":" << std::endl;
                    PrintVars(ss, var);
                }

                ADD_FAILURE() << ss.str();
            }
        }
    }
};

TEST_F(CBotUT, Test)
{
    ExecuteTest("extern void EmptyTest() { }");
}

TEST_F(CBotUT, DISABLED_TestFail)
{
    ExecuteTest("extern void FailingTest() { FAIL(); } extern void AnotherFailingTest() { FAIL(\"This is a message\"); }");
}

TEST_F(CBotUT, DivideByZero)
{
    ExecuteTest("extern void DivideByZero() { float a = 5/0; }", CBotErrZeroDiv);
}
