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

class CBotUT : public testing::Test
{
public:
    void SetUp()
    {
        CBotProgram::Init();
        CBotProgram::AddFunction("FAIL", rFail, cFail);
        CBotProgram::AddFunction("ASSERT", rAssert, cAssert);
    }

    void TearDown()
    {
        CBotProgram::Free();
    }

private:
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

    static CBotTypResult cFail(CBotVar* &var, void* user)
    {
        if (var != nullptr)
        {
            if (var->GetType() != CBotTypString) return CBotTypResult(CBotErrBadString);
            var = var->GetNext();
        }
        if (var != nullptr) return CBotTypResult(CBotErrOverParam);
        return CBotTypResult(CBotTypVoid);
    }

    static bool rFail(CBotVar* var, CBotVar* result, int& exception, void* user)
    {
        std::string message = "CBot test failed";
        if (var != nullptr)
        {
            message = var->GetValString();
        }

        throw CBotTestFail(message);
    }

    static CBotTypResult cAssert(CBotVar* &var, void* user)
    {
        if (var == nullptr) return CBotTypResult(CBotErrLowParam);
        if (var->GetType() != CBotTypBoolean) return CBotTypResult(CBotErrBadString);
        var = var->GetNext();
        return CBotTypResult(CBotTypVoid);
    }

    static bool rAssert(CBotVar* var, CBotVar* result, int& exception, void* user)
    {
        bool status = var->GetValInt();
        if (!status)
        {
            throw CBotTestFail("CBot assertion failed");
        }
        return true;
    }

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

protected:
    void ExecuteTest(const std::string& code, CBotError expectedError = CBotNoErr)
    {
        CBotError expectedCompileError = expectedError < 6000 ? expectedError : CBotNoErr;
        CBotError expectedRuntimeError = expectedError >= 6000 ? expectedError : CBotNoErr;

        auto program = std::unique_ptr<CBotProgram>(new CBotProgram());
        std::vector<std::string> tests;
        program->Compile(code, tests);

        CBotError error;
        int cursor1, cursor2;
        program->GetError(error, cursor1, cursor2);
        if (error != expectedCompileError)
        {
            std::stringstream ss;
            if (error != CBotNoErr)
            {
                FAIL() << "Compile error - " << error << " (" << cursor1 << "-" << cursor2 << ")"; // TODO: Error messages are on Colobot side
            }
            else
            {
                FAIL() << "No compile error, expected " << expectedCompileError; // TODO: Error messages are on Colobot side
            }
        }
        if (expectedCompileError != CBotNoErr) return;

        for (const std::string& test : tests)
        {
            try
            {
                program->Start(test);
                while (!program->Run());
                program->GetError(error, cursor1, cursor2);
                if (error != expectedRuntimeError)
                {
                    std::stringstream ss;
                    if (error != CBotNoErr)
                    {
                        ss << "RUNTIME ERROR - " << error; // TODO: Error messages are on Colobot side
                    }
                    else
                    {
                        ss << "No runtime error, expected " << expectedRuntimeError; // TODO: Error messages are on Colobot side
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

TEST_F(CBotUT, EmptyTest)
{
    ExecuteTest(
        "extern void EmptyTest()"
        "{"
        "}"
    );
}

TEST_F(CBotUT, DivideByZero)
{
    ExecuteTest(
        "extern void DivideByZero()"
        "{"
        "    float a = 5/0;"
        "}",
        CBotErrZeroDiv
    );
}

TEST_F(CBotUT, MissingSemicolon)
{
    ExecuteTest(
        "extern void MissingSemicolon()"
        "{"
        "    string a = \"hello\""
        "}",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, UndefinedFunction)
{
    ExecuteTest(
        "extern void UndefinedFunction()"
        "{"
        "    foo();"
        "}",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, BasicOperations)
{
    ExecuteTest(
        "extern void Comparations()"
        "{"
        "    ASSERT(true);"
        "    ASSERT(!false);"
        "    ASSERT(1 != 0);"
        "    ASSERT(1 == 1);"
        "    ASSERT(1 > 0);"
        "    ASSERT(1 >= 0);"
        "    ASSERT(1 >= 1);"
        "    ASSERT(0 < 1);"
        "    ASSERT(0 <= 1);"
        "    ASSERT(1 <= 1);"
        "}"
        ""
        "extern void BasicMath()"
        "{"
        "    ASSERT(2+2 == 4);"
        "    ASSERT(4-2 == 2);"
        "    ASSERT(2*2 == 4);"
        "    ASSERT(2/2 == 1);"
        "    ASSERT(5%2 == 1);"
        "    ASSERT(5**3 == 125);"
        "}"
    );
}