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

#include "CBot/CBot.h"

#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>

extern bool g_cbotTestSaveState;
bool g_cbotTestSaveState = false;

using namespace CBot;

class CBotUT : public testing::Test
{
public:
    CBotUT()
    {
        CBotProgram::Init();
        CBotProgram::AddFunction("FAIL", rFail, cFail);
        CBotProgram::AddFunction("ASSERT", rAssert, cAssert);
    }

    ~CBotUT()
    {
        CBotProgram::Free();
    }

private:
    class CBotTestFail : public std::runtime_error
    {
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

    unsigned int GetLine(const std::string& code, unsigned int pos, std::string& line)
    {
        line.clear();
        unsigned int lineNum = 1;
        for (unsigned int i = 0; i < pos && i < code.length(); i++)
        {
            if (code[i] == '\n')
            {
                lineNum++;
                line.clear();
                continue;
            }

            line += code[i];
        }
        for (unsigned int i = pos; i < code.length(); i++)
        {
            if (code[i] == '\n')
            {
                break;
            }

            line += code[i];
        }

        return lineNum;
    }

    std::string GetFormattedLineInfo(const std::string& code, int pos)
    {
        std::string line;
        unsigned int lineNum = GetLine(code, static_cast<unsigned int>(pos), line);

        std::stringstream ss;
        ss << "Line " << lineNum << ": " << line << std::endl;
        return ss.str();
    }

    static void TestSaveAndRestore(CBotProgram* program)
    {
        std::stringstream sstr("");
        // save
        if (!program->SaveState(sstr))
            throw CBotTestFail("CBotProgram::SaveState Failed");

        if (!CBotClass::SaveStaticState(sstr))
            throw CBotTestFail("CBotClass::SaveStaticState Failed");
        // restore
        if (!program->RestoreState(sstr))
            throw CBotTestFail("CBotProgram::RestoreState Failed");

        if (!CBotClass::RestoreStaticState(sstr))
            throw CBotTestFail("CBotClass::RestoreStaticState Failed");
    }

protected:
    std::unique_ptr<CBotProgram> ExecuteTest(const std::string& code, CBotError expectedError = CBotNoErr)
    {
        CBotError expectedCompileError = expectedError < 6000 ? expectedError : CBotNoErr;
        CBotError expectedRuntimeError = expectedError >= 6000 ? expectedError : CBotNoErr;

        auto program = m_progGroup.AddProgram(nullptr);
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
                ADD_FAILURE() << "Compile error - " << error << " (" << cursor1 << "-" << (cursor2 >= 0 ? cursor2 : cursor1) << ")" << std::endl << GetFormattedLineInfo(code, cursor1); // TODO: Error messages are on Colobot side
                return program;
            }
            else
            {
                ADD_FAILURE() << "No compile error, expected " << expectedCompileError; // TODO: Error messages are on Colobot side
                return program;
            }
        }
        if (expectedCompileError != CBotNoErr) return program;

        for (const std::string& test : tests)
        {
            try
            {
                program->Start(test);
                if (g_cbotTestSaveState)
                {
                    while (!program->Run(nullptr, 0)) // save/restore at each step
                    {
                        TestSaveAndRestore(program.get());
                    }
                }
                else
                {
                    while (!program->Run(nullptr, 0)); // execute in step mode
                }
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
                bool unknown = true;
                if (!funcName.empty())
                {
                    ss << "    while executing function " << funcName << " (" << cursor1 << "-" << (cursor2 >= 0 ? cursor2 : cursor1) << ")" << std::endl << GetFormattedLineInfo(code, cursor1);
                    unknown = false;
                }
                else if(e.cursor1 > 0 || e.cursor2 > 0)
                {
                    ss << "    at unknown location " << e.cursor1 << "-" << (e.cursor2 >= 0 ? e.cursor2 : e.cursor1) << std::endl << GetFormattedLineInfo(code, e.cursor1);
                    unknown = false;
                }
                ss << std::endl;

                if (!unknown)
                {
                    ss << "Variables:" << std::endl;
                    int level = 0;
                    while (true)
                    {
                        CBotVar* var = program->GetStackVars(funcName, level--);
                        if (var == nullptr) break;

                        ss << "  Block " << -level << ":" << std::endl;
                        PrintVars(ss, var);
                    }
                }

                ADD_FAILURE() << ss.str();
            }
        }
        return program; // Take it if you want, destroy on exit otherwise
    }

private:
    CBotProgramGroup m_progGroup;
};

TEST_F(CBotUT, EmptyTest)
{
    ExecuteTest(
        "extern void EmptyTest()\n"
        "{\n"
        "}\n"
    );
}

TEST_F(CBotUT, FunctionCompileErrors)
{
    ExecuteTest(
        "public",
        CBotErrNoType
    );

    ExecuteTest(
        "extern",
        CBotErrNoType
    );

    ExecuteTest(
        "public void",
        CBotErrNoFunc
    );

    ExecuteTest(
        "extern void",
        CBotErrNoFunc
    );

    ExecuteTest(
        "extern void MissingParameterType(",
        CBotErrNoType
    );

    ExecuteTest(
        "extern void MissingParamName(int",
        CBotErrNoVar
    );

    ExecuteTest(
        "extern void MissingCloseParen(int i",
        CBotErrClosePar
    );

    ExecuteTest(
        "extern void ParamTrailingComma(int i, ) {\n"
        "}\n",
        CBotErrNoType
    );

   ExecuteTest(
        "extern void MissingOpenBlock(int i)",
        CBotErrOpenBlock
    );

    ExecuteTest(
        "extern void MissingCloseBlock()\n"
        "{\n",
        CBotErrCloseBlock
    );

}

TEST_F(CBotUT, ClassCompileErrors)
{
    ExecuteTest(
        "public class",
        CBotErrNoClassName
    );

    ExecuteTest(
        "public class 1234",
        CBotErrNoClassName
    );

    ExecuteTest(
        "public class TestClass",
        CBotErrOpenBlock
    );

    ExecuteTest(
        "public class TestClass\n"
        "{\n",
        CBotErrCloseBlock
    );

    ExecuteTest(
        "public class TestClass extends",
        CBotErrNoClassName
    );

    ExecuteTest(
        "public class TestClass extends 1234",
        CBotErrNoClassName
    );
}

TEST_F(CBotUT, DivideByZero)
{
    ExecuteTest(
        "extern void DivideByZero()\n"
        "{\n"
        "    float a = 5/0;\n"
        "}\n",
        CBotErrZeroDiv
    );
}

TEST_F(CBotUT, MissingSemicolon)
{
    ExecuteTest(
        "extern void MissingSemicolon()\n"
        "{\n"
        "    string a = \"hello\"\n"
        "}\n",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, UndefinedFunction)
{
    ExecuteTest(
        "extern void UndefinedFunction()\n"
        "{\n"
        "    foo();\n"
        "}\n",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, BasicOperations)
{
    ExecuteTest(
        "extern void Comparations()\n"
        "{\n"
        "    ASSERT(1 != 0);\n"
        "    ASSERT(1 == 1);\n"
        "    ASSERT(1 > 0);\n"
        "    ASSERT(1 >= 0);\n"
        "    ASSERT(1 >= 1);\n"
        "    ASSERT(0 < 1);\n"
        "    ASSERT(0 <= 1);\n"
        "    ASSERT(1 <= 1);\n"
        "}\n"
        "\n"
        "extern void BasicMath()\n"
        "{\n"
        "    ASSERT(2+2 == 4);\n"
        "    ASSERT(4-2 == 2);\n"
        "    ASSERT(2*2 == 4);\n"
        "    ASSERT(2/2 == 1);\n"
        "    ASSERT(5%2 == 1);\n"
        "    ASSERT(5**3 == 125);\n"
        "}\n"
        "\n"
        "extern void BitwiseMath()\n"
        "{\n"
        "    ASSERT((1 << 5) == 32);\n"
        "    ASSERT((32 >> 5) == 1);\n"
        "    ASSERT((3 & 2) == 2);\n"
        "    ASSERT((1 & 2) == 0);\n"
        "    ASSERT((1 | 2) == 3);\n"
        "    ASSERT((2 | 2) == 2);\n"
        "    ASSERT((5 ^ 3) == 6);\n"
        "    ASSERT((~1024) == -1025);\n"
        "}\n"
        "\n"
        "extern void BooleanLogic()\n"
        "{\n"
        "    ASSERT(true);\n"
        "    ASSERT(!false);\n"
        "    ASSERT(true && true);\n"
        "    ASSERT(!(true && false));\n"
        "    ASSERT(!(false && true));\n"
        "    ASSERT(!(false && false));\n"
        "    ASSERT(true || true);\n"
        "    ASSERT(true || false);\n"
        "    ASSERT(false || true);\n"
        "    ASSERT(!(false || false));\n"
        "    ASSERT(!(true ^ true));\n"
        "    ASSERT(true ^ false);\n"
        "    ASSERT(false ^ true);\n"
        "    ASSERT(!(false ^ false));\n"
        "}\n"
        "\n"
        "extern void NumberFormats()\n"
        "{\n"
        "    ASSERT(2.0 == 2);\n"
        "    ASSERT(2.00000 == 2);\n"
        "    ASSERT(2.50000 == 2.5);\n"
        "    ASSERT(-2.0 == -2);\n"
        "    ASSERT(2e3 == 2000);\n"
        "    ASSERT(-2e3 == -2000);\n"
        "    ASSERT(2e-3 == 0.002);\n"
        "    ASSERT(-2e-3 == -0.002);\n"
        "    ASSERT(0xFF == 255);\n"
        "    ASSERT(0xAB == 171);\n"
        "}\n"
    );
}

TEST_F(CBotUT, VarBasic)
{
    ExecuteTest(
        "extern void VarBasic()\n"
        "{\n"
        "    int a = 5;\n"
        "    ASSERT(a == 5);\n"
        "    int b = 3;\n"
        "    ASSERT(b == 3);\n"
        "    b = a;\n"
        "    ASSERT(b == 5);\n"
        "    ASSERT(b == a);\n"
        "    b = a + b;\n"
        "    ASSERT(b == 10);\n"
        "    ASSERT(b == 2*a);\n"
        "}\n"
    );
}

TEST_F(CBotUT, VarDefinitions)
{
    ExecuteTest(
        "extern void TestUndefined()\n"
        "{\n"
        "    ASSERT(a == 0);\n"
        "}\n",
        CBotErrUndefVar
    );
    ExecuteTest(
        "extern void TestRedefined()\n"
        "{\n"
        "    int a = 5;\n"
        "    int a = 3;\n"
        "}\n",
        CBotErrRedefVar
    );
}

// TODO: I don't actually know what the exact rules should be, but it looks a bit wrong
TEST_F(CBotUT, VarImplicitCast)
{
    ExecuteTest(
        "extern void ImplicitCast()\n"
        "{\n"
        "    int a = 5;\n"
        //"    ASSERT(a == \"5\");\n"
        "    string b = a;\n"
        "    ASSERT(b == \"5\");\n"
        //"    ASSERT(b == a);\n"
        "    \n"
        "    string c = \"2.5\";\n"
        //"    ASSERT(c == 2.5);\n"
        //"    float d = c;\n"
        //"    ASSERT(d == c);\n"
        //"    ASSERT(d == 2.5);\n"
        "}\n"
        "\n"
        "extern void AssignImplicitCast()\n"
        "{\n"
        "    string a = 2;\n"
        "    ASSERT(a == \"2\");\n"
        "    a = 3;\n"
        "    ASSERT(a == \"3\");\n"
        "    string b = 2.5;\n"
        "    ASSERT(b == \"2.5\");\n"
        "    b = 3.5;\n"
        "    ASSERT(b == \"3.5\");\n"
        "}\n"
    );

    ExecuteTest(
        "string func()\n"
        "{\n"
        "    return 5;\n"
        "}\n"
        "extern void ImplicitCastOnReturn()\n"
        "{\n"
        "    string a = func();\n"
        "    ASSERT(a == \"5\");"
        "}\n"
    );
}

TEST_F(CBotUT, IntegerMathNearLimits_Issue993)
{
    ExecuteTest(
        "extern void Test_Issue993() {\n"
        "    ASSERT(-2147483600 *  1 == -2147483600);\n"
        "    ASSERT( 2147483600 *  1 ==  2147483600);\n"
        "    ASSERT( 2147483646 *  1 ==  2147483646);\n"
        "    ASSERT( 2147483646 * -1 == -2147483646);\n"
        "    ASSERT( 2147483000 * -1 == -2147483000);\n"
        "    ASSERT( 2147483000 *  1 ==  2147483000);\n"
        "}\n"
    );
}

TEST_F(CBotUT, BinaryLiterals)
{
    ExecuteTest(
        "extern void TestBinaryLiterals() {\n"
        "    ASSERT(  8 ==   0b00001000);\n"
        "    ASSERT( 12 ==   0b00001100);\n"
        "    ASSERT( 16 ==   0b00010000);\n"
        "    ASSERT( 24 ==   0b00011000);\n"
        "    ASSERT( 32 ==   0b00100000);\n"
        "    ASSERT( 48 ==   0b00110000);\n"
        "    ASSERT( 64 ==   0b01000000);\n"
        "    ASSERT( 96 ==   0b01100000);\n"
        "    ASSERT(128 ==   0b10000000);\n"
        "    ASSERT(192 ==   0b11000000);\n"
        "    ASSERT(256 ==  0b100000000);\n"
        "    ASSERT(384 ==  0b110000000);\n"
        "    ASSERT(512 == 0b1000000000);\n"
        "}\n"
    );
}

TEST_F(CBotUT, TestSwitchCase)
{
    ExecuteTest(
        "extern void Test_Switch_Case() {\n"
        "    int n = 0, c = 0;\n"
        "    for (int i = -9; i < 11; ++i) {\n"
        "        switch (i) {\n"
        "            case -9: n = -9; ++c; break;\n"
        "            case -8: n = -8; ++c; break;\n"
        "            case -7: n = -7; ++c; break;\n"
        "            case -6: n = -6; ++c; break;\n"
        "            case -5: n = -5; ++c; break;\n"
        "            case -4: n = -4; ++c; break;\n"
        "            case -3: n = -3; ++c; break;\n"
        "            case -2: n = -2; ++c; break;\n"
        "            case -1: n = -1; ++c; break;\n"
        "            case 0: n = 0; ++c; break;\n"
        "            case 1: n = 1; ++c; break;\n"
        "            case 2: n = 2; ++c; break;\n"
        "            case 3: n = 3; ++c; break;\n"
        "            case 4: n = 4; ++c; break;\n"
        "            case 5: n = 5; ++c; break;\n"
        "            case 6: n = 6; ++c; break;\n"
        "            case 7: n = 7; ++c; break;\n"
        "            case 8: n = 8; ++c; break;\n"
        "            case 9: n = 9; ++c; break;\n"
        "            default: n = 10; ++c; break;\n"
        "        }\n"
        "        ASSERT(n == i);\n"
        "    }\n"
        "    ASSERT(n == 10);\n"
        "    ASSERT(c == 20);\n"
        "}\n"
        "extern void Test_Case_With_Math() {\n"
        "    int n = 0, c = 0;\n"
        "    for (int i = -9; i < 11; ++i) {\n"
        "        switch (i * 10) {\n"
        "            case -9*10: n = -90; ++c; break;\n"
        "            case -8*10: n = -80; ++c; break;\n"
        "            case -7*10: n = -70; ++c; break;\n"
        "            case -6*10: n = -60; ++c; break;\n"
        "            case -5*10: n = -50; ++c; break;\n"
        "            case -4*10: n = -40; ++c; break;\n"
        "            case -3*10: n = -30; ++c; break;\n"
        "            case -2*10: n = -20; ++c; break;\n"
        "            case -1*10: n = -10; ++c; break;\n"
        "            case 0*10: n = 0; ++c; break;\n"
        "            case 1*10: n = 10; ++c; break;\n"
        "            case 2*10: n = 20; ++c; break;\n"
        "            case 3*10: n = 30; ++c; break;\n"
        "            case 4*10: n = 40; ++c; break;\n"
        "            case 5*10: n = 50; ++c; break;\n"
        "            case 6*10: n = 60; ++c; break;\n"
        "            case 7*10: n = 70; ++c; break;\n"
        "            case 8*10: n = 80; ++c; break;\n"
        "            case 9*10: n = 90; ++c; break;\n"
        "            default: n = 100; ++c; break;\n"
        "        }\n"
        "        ASSERT(n == i * 10);\n"
        "    }\n"
        "    ASSERT(n == 100);\n"
        "    ASSERT(c == 20);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void Duplicate_Case() {\n"
        "    switch(0) {\n"
        "        case 1000:\n"
        "        case 10*100:\n"
        "    }\n"
        "}\n",
        CBotErrRedefCase
    );

    ExecuteTest(
        "extern void Duplicate_Default() {\n"
        "    switch(0) {\n"
        "        default:\n"
        "        default:\n"
        "    }\n"
        "}\n",
        CBotErrRedefCase
    );
}

TEST_F(CBotUT, TestRepeatInstruction)
{
    ExecuteTest(
        "extern void TestRepeat() {\n"
        "    int c = 0;\n"
        "    for (int i = 1; i < 11; ++i)\n"
        "    {\n"
        "        repeat (i) ++c;\n"
        "    }\n"
        "    ASSERT(c == 55);\n"
        "}\n"
        "extern void TestRepeatBreakAndContinue() {\n"
        "    int c = 0;\n"
        "    repeat (10)\n"
        "    {\n"
        "        if (++c == 5) break;\n"
        "        continue;\n"
        "        FAIL();\n"
        "    }\n"
        "    ASSERT(c == 5);\n"
        "    label:repeat (10)\n"
        "    {\n"
        "        if (++c == 10) break label;\n"
        "        continue label;\n"
        "        FAIL();\n"
        "    }\n"
        "    ASSERT(c == 10);\n"
        "}\n"
        "extern void NoRepeatNumberLessThanOne() {\n"
        "    repeat (0) FAIL();\n"
        "    repeat (-1) FAIL();\n"
        "    repeat (-2) FAIL();\n"
        "}\n"
        "extern void EvaluateExpressionOnlyOnce() {\n"
        "    int c = 0;\n"
        "    repeat (c + 5) ASSERT(++c < 6);\n"
        "    ASSERT(c == 5);\n"
        "}\n"
    );
    ExecuteTest(
        "extern void MissingOpenParen() {\n"
        "    repeat ;\n"
        "}\n",
        CBotErrOpenPar
    );
    ExecuteTest(
        "extern void MissingNumber() {\n"
        "    repeat (;\n"
        "}\n",
        CBotErrBadNum
    );
    ExecuteTest(
        "extern void WrongType() {\n"
        "    repeat (\"not number\");\n"
        "}\n",
        CBotErrBadType1
    );
    ExecuteTest(
        "extern void MissingCloseParen() {\n"
        "    repeat (2;\n"
        "}\n",
        CBotErrClosePar
    );
}

TEST_F(CBotUT, ToString)
{
    ExecuteTest(
        "extern void ArrayToString()\n"
        "{\n"
        "    int[] array = {2, 4, 6};\n"
        "    string arrayStr = \"\"+array;\n"
        "    ASSERT(arrayStr == \"{ 2, 4, 6 }\");\n"
        "}\n"
    );

    ExecuteTest(
        "public class Test { int a = 1337; }\n"
        "extern void ClassToString()\n"
        "{\n"
        "    Test test();\n"
        "    string testStr = \"\"+test;\n"
        "    ASSERT(testStr == \"Pointer to Test( a=1337 )\");\n"
        "}\n"
    );

    ExecuteTest(
        "extern void ClassToString_2()\n"
        "{\n"
        "    string s = new TestClass;\n"
        "    ASSERT(s == \"Pointer to TestClass(  )\");\n"
        "}\n"
        "public class TestClass { /* no fields */ }\n"
    );

    ExecuteTest(
        "extern void ClassInheritanceToString()\n"
        "{\n"
        "    string s = new SubClass;\n"
        "    ASSERT(s == \"Pointer to SubClass( c=7, d=8, e=9 ) extends MidClass( b=4, c=5, d=6 ) extends BaseClass( a=1, b=2, c=3 )\");\n"
        "}\n"
        "public class BaseClass { int a = 1, b = 2, c = 3; }\n"
        "public class MidClass extends BaseClass { int b = 4, c = 5, d = 6; }\n"
        "public class SubClass extends MidClass { int c = 7, d = 8, e = 9; }\n"
    );

    ExecuteTest(
        "extern void ClassInheritanceToString_2()\n"
        "{\n"
        "    string s = new SubClass;\n"
        "    ASSERT(s == \"Pointer to SubClass( c=7, d=8, e=9 ) extends MidClass(  ) extends BaseClass( a=1, b=2, c=3 )\");\n"
        "}\n"
        "public class BaseClass { int a = 1, b = 2, c = 3; }\n"
        "public class MidClass extends BaseClass { /* no fields */ }\n"
        "public class SubClass extends MidClass { int c = 7, d = 8, e = 9; }\n"
    );

    ExecuteTest(
        "extern void ClassInheritanceToString_3()\n"
        "{\n"
        "    string s = new SubClass;\n"
        "    ASSERT(s == \"Pointer to SubClass( c=7, d=8, e=9 ) extends MidClass(  ) extends BaseClass(  )\");\n"
        "}\n"
        "public class BaseClass { /* no fields */ }\n"
        "public class MidClass extends BaseClass { /* no fields */ }\n"
        "public class SubClass extends MidClass { int c = 7, d = 8, e = 9; }\n"
    );

    // TODO: IntrinsicClassToString ? (e.g. point)
}

TEST_F(CBotUT, Arrays)
{
    ExecuteTest(
        "extern void ArrayTest()\n"
        "{\n"
        "    int a[];\n"
        "    ASSERT(sizeof(a) == 0);\n"
        "    ASSERT(a == null);\n" // TODO: I'm not sure if this is correct behaviour or not
        "    a[0] = 5;\n"
        "    ASSERT(a[0] == 5);\n"
        "    ASSERT(sizeof(a) == 1);\n"
        "    ASSERT(a != null);\n"
        "    a[5] = 5;\n"
        "    ASSERT(sizeof(a) == 6);\n"
        "    a[3] = 5;"
        "    ASSERT(sizeof(a) == 6);\n"
        "    \n"
        "    int[] b;\n"
        "    ASSERT(sizeof(b) == 0);\n"
        "    ASSERT(b == null);\n" // TODO: I'm not sure if this is correct behaviour or not
        "    b[0] = 5;\n"
        "    ASSERT(b[0] == 5);\n"
        "    ASSERT(sizeof(b) == 1);\n"
        "    ASSERT(b != null);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void LimitedArrayTest()\n"
        "{\n"
        "    int a[5];\n"
        "    ASSERT(sizeof(a) == 0);\n"
        "    a[0] = 1;\n"
        "    ASSERT(sizeof(a) == 1);\n"
        "    a[4] = 1;\n"
        "    ASSERT(sizeof(a) == 5);\n"
        "    a[5] = 1;\n"
        "}\n",
        CBotErrOutArray
    );

    ExecuteTest(
        "extern void BadArrayDeclarationTest()\n"
        "{\n"
        "    int[5] a;\n"
        "}\n",
        CBotErrCloseIndex
    );
}

TEST_F(CBotUT, ArraysInClasses)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    private int test[];\n"
        "    private int test2[5];\n"
        "    \n"
        "    public void TestClass() {\n"
        "        ASSERT(sizeof(test) == 0);\n"
        "        ASSERT(sizeof(this.test) == 0);\n"
        "        ASSERT(test == null);\n" // TODO: Again, not sure
        "        test[0] = 5;\n"
        "        this.test[1] = 5;\n"
        "        ASSERT(sizeof(test) == 2);\n"
        "        ASSERT(sizeof(this.test) == 2);\n"
        "        ASSERT(test != null);\n"
        "    }\n"
        "}\n"
        "extern void ArraysInClasses()\n"
        "{\n"
        "    TestClass t();\n"
        "}\n"
    );
}

TEST_F(CBotUT, ArraysOfClasses)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public int i = 0;\n"
        "}\n"
        "extern void ArraysInClasses()\n"
        "{\n"
        "    TestClass test[];\n"
        "    test[0] = new TestClass();\n"
        "    test[0].i = 5;\n"
        "    ASSERT(test[0].i == 5);\n"
        "    \n"
        "    TestClass[] test2;\n"
        "}\n"
    );
}

TEST_F(CBotUT, Functions)
{
    ExecuteTest(
        "bool notThisOne()\n"
        "{\n"
        "    return false;\n"
        "}\n"
        "bool testFunction()\n"
        "{\n"
        "    return true;\n"
        "}\n"
        "\n"
        "extern void Functions()\n"
        "{\n"
        "    ASSERT(testFunction());\n"
        "}\n"
    );
}


TEST_F(CBotUT, FunctionRecursion)
{
    ExecuteTest(
        "int fact(int x)\n"
        "{\n"
        "    if(x == 0) return 1;\n"
        "    return fact(x-1) * x;\n"
        "}\n"
        "\n"
        "extern void FunctionRecursion()\n"
        "{\n"
        "    ASSERT(fact(10) == 3628800);\n"
        "}\n"
    );
}

TEST_F(CBotUT, FunctionRecursionStackOverflow)
{
    ExecuteTest(
        "extern void StackOverflow()\n"
        "{\n"
        "    StackOverflow();\n"
        "}\n",
        CBotErrStackOver
    );
}

TEST_F(CBotUT, FunctionOverloading)
{
    ExecuteTest(
        "int func(string test)\n"
        "{\n"
        "    return 1;\n"
        "}\n"
        "int func(int test)\n"
        "{\n"
        "    return 2;\n"
        "}\n"
        "\n"
        "extern void FunctionOverloading()\n"
        "{\n"
        "    ASSERT(func(\"5\") == 1);\n"
        "    ASSERT(func(5) == 2);\n"
        "}\n"
    );
}

TEST_F(CBotUT, FunctionRedefined)
{
    ExecuteTest(
        "int func(int test)\n"
        "{\n"
        "    return 1;\n"
        "}\n"
        "int func(int test)\n"
        "{\n"
        "    return 2;\n"
        "}\n",
        CBotErrRedefFunc
    );

    ExecuteTest(
        "int func(int[] test)\n"
        "{\n"
        "    return 1;\n"
        "}\n"
        "int func(int[] test)\n"
        "{\n"
        "    return 2;\n"
        "}\n",
        CBotErrRedefFunc
    );
}

TEST_F(CBotUT, FunctionBadReturn)
{
    ExecuteTest(
        "int func()\n"
        "{\n"
        "    return \"test\";\n"
        "}\n"
        "extern void FunctionBadReturn()\n"
        "{\n"
        "    int a = func();\n"
        "}\n",
        CBotErrBadType1
    );
}

TEST_F(CBotUT, FunctionNoReturn)
{
    ExecuteTest(
        "int func()\n"
        "{\n"
        "}\n"
        "extern void FunctionNoReturn()\n"
        "{\n"
        "    func();\n"
        "}\n",
        CBotErrNoReturn
    );

    ExecuteTest(
        "int FuncDoesNotReturnAValue()\n"
        "{\n"
        "    if (false) return 1;\n"
        "    while (false) return 1;\n"
        "    if (true) ; else return 1;\n"
        "    do { break; return 1; } while (false);\n"
        "    do { continue; return 1; } while (false);\n"
        "}\n",
        CBotErrNoReturn
    );

    ExecuteTest(
        "int FuncHasReturn()\n"
        "{\n"
        "    return 1;\n"
        "}\n"
        "int BlockHasReturn()\n"
        "{\n"
        "    {\n"
        "        {\n"
        "        }\n"
        "        return 2;\n"
        "    }\n"
        "}\n"
        "int IfElseHasReturn()\n"
        "{\n"
        "    if (false) {\n"
        "        return 3;\n"
        "    } else {\n"
        "        if (false) return 3;\n"
        "        else return 3;\n"
        "    }\n"
        "}\n"
        "extern void Test()\n"
        "{\n"
        "    ASSERT(1 == FuncHasReturn());\n"
        "    ASSERT(2 == BlockHasReturn());\n"
        "    ASSERT(3 == IfElseHasReturn());\n"
        "}\n"
    );
}

TEST_F(CBotUT, PublicFunctions)
{
    // Keep the program, so that the function continues to exist after ExecuteTest finishes
    auto publicProgram = ExecuteTest(
        "public int test()\n"
        "{\n"
        "    return 1337;\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestPublic()\n"
        "{\n"
        "    ASSERT(test() == 1337);\n"
        "}\n"
    );

    publicProgram.reset(); // Now remove

    ExecuteTest(
        "extern void TestPublicRemoved()\n"
        "{\n"
        "    ASSERT(test() == 1337);\n"
        "}\n",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassConstructor)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public static int instanceCounter = 0;\n"
        "    public void TestClass() {\n"
        "        instanceCounter++;\n"
        "    }\n"
        "}\n"
        "\n"
        "extern void ClassConstructor()\n"
        "{\n"
        "    TestClass t1();\n"
        "    ASSERT(t1.instanceCounter == 1);\n"
        "    ASSERT(t1 != null);\n"
        "    TestClass t2; // not calling the constructor!\n"
        "    ASSERT(t1.instanceCounter == 1);\n"
        //"    ASSERT(t2 == null);\n" // TODO: I was pretty sure that's how it worked, but apparently not...
        "    TestClass t3 = new TestClass();\n"
        "    ASSERT(t1.instanceCounter == 2);\n"
        "    ASSERT(t3.instanceCounter == 2);\n"
        "    ASSERT(t3 != null);\n"
        "    ASSERT(t3 != t1);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassDestructor)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public static int instanceCounter = 0;\n"
        "    public void TestClass() {\n"
        "        instanceCounter++;\n"
        "    }\n"
        "    public void ~TestClass() {\n"
        "        instanceCounter--;\n"
        "    }\n"
        "}\n"
        "\n"
        "extern void ClassDestructor()\n"
        "{\n"
        "    TestClass t1();\n"
        "    ASSERT(t1.instanceCounter == 1);\n"
        "    {\n"
        "        TestClass t2();\n"
        "        ASSERT(t2.instanceCounter == 2);\n"
        "        ASSERT(t1.instanceCounter == 2);\n"
        "    }\n"
        "    ASSERT(t1.instanceCounter == 1);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassBadNew)
{
    ExecuteTest(
        "public class AClass {};\n"
        "extern void ClassBadNew()\n"
        "{\n"
        "    AClass a = new \"abc\";\n"
        "}\n",
        CBotErrBadNew
    );
}

TEST_F(CBotUT, ClassCallOnNull)
{
    ExecuteTest(
        "public class AClass {\n"
        "    public void test() {}\n"
        "};\n"
        "extern void ClassCallOnNull()\n"
        "{\n"
        "    AClass a = null;\n"
        "    a.test();\n"
        "}\n",
        CBotErrNull
    );
}

TEST_F(CBotUT, ClassNullPointer)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public void TestClass() {\n"
        "        FAIL();\n"
        "    }\n"
        "}\n"
        "extern void TestClassNullPointer()\n"
        "{\n"
        "    TestClass t;\n"
        //"    ASSERT(t == null);\n" // TODO: OH REALLY?
        "    TestClass t2 = null;\n"
        "    ASSERT(t2 == null);\n"
        "}\n"
    );
    ExecuteTest(
        "public class TestClass {\n"
        "    public int x = 0;"
        "    public void TestClass() {\n"
        "        FAIL();\n"
        "    }\n"
        "}\n"
        "extern void TestClassNullPointerAccess()\n"
        "{\n"
        "    TestClass t;\n"
        "    int y = t.x;\n"
        "}\n",
        CBotErrNull
    );
}

TEST_F(CBotUT, ClassDestructorNaming)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public void ~SomethingElse() {}\n"
        "}\n",
        CBotErrNoFunc
    );
    ExecuteTest(
        "public class SomethingElse {\n"
        "}\n"
        "public class TestClass2 {\n"
        "    public void ~SomethingElse() {}\n"
        "}\n",
        CBotErrNoFunc
    );
}

TEST_F(CBotUT, ClassDestructorSyntax)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    void ~TestClass(int i) {}\n"
        "}\n"
        "extern void DestructorNoParams() {}\n",
        CBotErrClosePar
    );

    ExecuteTest(
        "public class TestClass {\n"
        "    int ~TestClass() {}\n"
        "}\n"
        "extern void DestructorReturnTypeVoid() {}\n",
        CBotErrFuncNotVoid
    );
}

TEST_F(CBotUT, ClassMethodOverloading)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public int test(string test) {\n"
        "        return 1;\n"
        "    }\n"
        "    public int test(int test) {\n"
        "        return 2;"
        "    }\n"
        "}\n"
        "extern void ClassMethodOverloading() {\n"
        "    TestClass t();\n"
        "    ASSERT(t.test(\"5\") == 1);\n"
        "    ASSERT(t.test(5) == 2);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassMethodRedefined)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public int test(string test) {\n"
        "        return 1;\n"
        "    }\n"
        "    public int test(string test) {\n"
        "        return 2;\n"
        "    }\n"
        "}\n",
        CBotErrRedefFunc
    );

    ExecuteTest(
        "public class TestClass {\n"
        "    public int test(int[] test) {\n"
        "        return 1;\n"
        "    }\n"
        "    public int test(int[] test) {\n"
        "        return 2;\n"
        "    }\n"
        "}\n",
        CBotErrRedefFunc
    );

    ExecuteTest(
        "public class TestClass {\n"
        "    void ~TestClass() {}\n"
        "    void ~TestClass() {}\n"
        "}\n",
        CBotErrRedefFunc
    );
}

TEST_F(CBotUT, ClassFieldNaming)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int ~i = 1;\n"
        "}\n",
        CBotErrNoVar
    );

    ExecuteTest(
        "public class TestClass {\n"
        "    int TestClass = 1;\n"
        "}\n",
        CBotErrNoVar
    );

    ExecuteTest(
        "public class TestClass {\n"
        "    int i = 1;\n"
        "    int i = 2;\n"
        "}\n",
        CBotErrRedefVar
    );
}

TEST_F(CBotUT, ClassRedefinedInDifferentPrograms)
{
    auto publicProgram = ExecuteTest(
        "public class TestClass {}\n"
    );

    ExecuteTest(
        "public class TestClass {}\n",
        CBotErrRedefClass
    );
}

TEST_F(CBotUT, ClassRedefinedInOneProgram)
{
    ExecuteTest(
        "public class TestClass {}\n"
        "public class TestClass {}\n",
        CBotErrRedefClass
    );
}

TEST_F(CBotUT, ClassMissingCloseBlock)
{
    ExecuteTest(
        "public class Something\n"
        "{\n",
        CBotErrCloseBlock
    );
}

// TODO: NOOOOOO!!! Nononononono :/
TEST_F(CBotUT, DISABLED_PublicClasses)
{
    // Keep the program, so that the class continues to exist after ExecuteTest finishes
    auto publicProgram = ExecuteTest(
        "public class TestClass\n"
        "{\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestPublic()\n"
        "{\n"
        "    TestClass t();\n"
        "}\n"
    );

    publicProgram.reset(); // Now remove

    ExecuteTest(
        "extern void TestPublicRemoved()\n"
        "{\n"
        "    TestClass t();\n"
        "}\n",
        CBotErrUndefClass
    );
}

TEST_F(CBotUT, ThisEarlyContextSwitch_Issue436)
{
    ExecuteTest(
        "public class Something {\n"
        "    public int a = 7;"
        "    void test2(int i, int expected) {\n"
        "        ASSERT(i == expected);\n"
        "    }\n"
        "}"
        "public class TestClass {\n"
        "    public int i = 5;\n"
        "    public void test(Something s) {\n"
        "        s.test2(this.i, 5);\n"
        "    }\n"
        "}\n"
        "extern void WeirdClassThisAsParamThing()\n"
        "{\n"
        "    Something s();\n"
        "    TestClass t();"
        "    t.test(s);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassStringAdd_Issue535)
{
    ExecuteTest(
        "public class TestClass {}\n"
        "extern void ClassStringAdd()\n"
        "{\n"
        "    TestClass t();\n"
        "    string s = t + \"!\";\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceAssignment)
{
    ExecuteTest(
        "public class BaseClass {}\n"
        "public class MidClass extends BaseClass {}\n"
        "public class SubClass extends MidClass {}\n"
        "extern void ClassInheritanceAssignment()\n"
        "{\n"
        "    BaseClass bc = new MidClass();\n"
        "    MidClass  mc = bc;\n"
        "    mc = new SubClass();\n"
        "    SubClass  sc = mc;\n"
        "    bc = mc;\n"
        "    bc = new MidClass();\n"
        "    bc = new SubClass();\n"
        "    sc = bc;\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceVars)
{
    ExecuteTest(
        "public class BaseClass {\n"
        "    int a = 123;\n"
        "    int b = 456;\n"
        "    int c = 789;\n"
        "}\n"
        "public class MidClass extends BaseClass {\n"
        "    int b = 1011;\n"
        "    int c = 1213;\n"
        "    int d = 1415;\n"
        "}\n"
        "public class SubClass extends MidClass {\n"
        "    int c = 1617;\n"
        "    int d = 1819;\n"
        "    int e = 2021;\n"
        "}\n"
        "extern void ClassInheritanceVars()\n"
        "{\n"
        "    BaseClass bc();\n"
        "    ASSERT(bc.a == 123);\n"
        "    ASSERT(bc.b == 456);\n"
        "    ASSERT(bc.c == 789);\n"
        "    MidClass  mc();\n"
        "    ASSERT(mc.a == 123);\n"
        "    ASSERT(mc.b == 1011);\n"
        "    ASSERT(mc.c == 1213);\n"
        "    ASSERT(mc.d == 1415);\n"
        "    SubClass  sc();\n"
        "    ASSERT(sc.a == 123);\n"
        "    ASSERT(sc.b == 1011);\n"
        "    ASSERT(sc.c == 1617);\n"
        "    ASSERT(sc.d == 1819);\n"
        "    ASSERT(sc.e == 2021);\n"
        // Test polymorphism
        "    bc = mc;\n"
        "    ASSERT(bc.a == 123);\n"
        "    ASSERT(bc.b == 456);\n"
        "    ASSERT(bc.c == 789);\n"
        "    mc = sc;\n"
        "    ASSERT(mc.a == 123);\n"
        "    ASSERT(mc.b == 1011);\n"
        "    ASSERT(mc.c == 1213);\n"
        "    ASSERT(mc.d == 1415);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceMethods)
{
    ExecuteTest(
        "public class BaseClass {\n"
        "    int a = 123;\n"
        "    int b = 456;\n"
        "    int c = 789;\n"
        "    int testOverride() { return 123; }\n"
        "    int testNoOverride() { return 456; }\n"
        "    int testInsideBaseClass() {\n"
        "        ASSERT(a == 123);\n"
        "        ASSERT(b == 456);\n"
        "        ASSERT(c == 789);\n"
        "        ASSERT(456 == testNoOverride());\n"
        "        return c;\n"
        "    }\n"
        "    int testInsideBaseOverride() { return testOverride(); }\n"
        "}\n"
        "public class MidClass extends BaseClass {\n"
        "    int b = 1011;\n"
        "    int c = 1213;\n"
        "    int d = 1415;\n"
        "    int testOverride() { return 1011; }\n"
        "    int testInsideMidClass() {\n"
        "        ASSERT(a == 123);\n"
        "        ASSERT(b == 1011);\n"
        "        ASSERT(c == 1213);\n"
        "        ASSERT(d == 1415);\n"
        "        ASSERT(456  == testNoOverride());\n"
        "        ASSERT(789  == testInsideBaseClass());\n"
        "        return c;\n"
        "    }\n"
        "    int testSuper() {\n"
        "        ASSERT(super.a == 123);\n"
        "        ASSERT(super.b == 456);\n"
        "        ASSERT(super.c == 789);\n"
        "        ASSERT(123 == super.testOverride());\n"
        "        ASSERT(789 == super.testInsideBaseClass());\n"
        "        return super.testInsideBaseOverride();\n"
        "    }\n"
        "    int testInsideMidOverride() { return testOverride(); }\n"
        "}\n"
        "public class SubClass extends MidClass {\n"
        "    int c = 1617;\n"
        "    int d = 1819;\n"
        "    int e = 2021;\n"
        "    int testOverride() { return 1617; }\n"
        "    int testInsideSubClass() {\n"
        "        ASSERT(a == 123);\n"
        "        ASSERT(b == 1011);\n"
        "        ASSERT(c == 1617);\n"
        "        ASSERT(d == 1819);\n"
        "        ASSERT(e == 2021);\n"
        "        ASSERT(456  == testNoOverride());\n"
        "        ASSERT(789  == testInsideBaseClass());\n"
        "        ASSERT(1213 == testInsideMidClass());\n"
        "        return c;\n"
        "    }\n"
        "    int testSuper() {\n"
        "        ASSERT(super.a == 123);\n"
        "        ASSERT(super.b == 1011);\n"
        "        ASSERT(super.c == 1213);\n"
        "        ASSERT(super.d == 1415);\n"
        "        ASSERT(1011 == super.testOverride());\n"
        "        ASSERT(789  == super.testInsideBaseClass());\n"
        "        ASSERT(1213 == super.testInsideMidClass());\n"
        "        return super.testSuper();\n"
        "    }\n"
        "    int testInsideSubOverride() { return testOverride(); }\n"
        "}\n"
        "extern void InheritanceMethods()\n"
        "{\n"
        "    BaseClass bc();\n"
        "    ASSERT(123 == bc.testOverride());\n"
        "    ASSERT(456 == bc.testNoOverride());\n"
        "    ASSERT(789 == bc.testInsideBaseClass());\n"
        "    ASSERT(123 == bc.testInsideBaseOverride());\n"
        "    MidClass  mc();\n"
        "    ASSERT(1011 == mc.testSuper());\n"
        "    ASSERT(1011 == mc.testOverride());\n"
        "    ASSERT(456  == mc.testNoOverride());\n"
        "    ASSERT(789  == mc.testInsideBaseClass());\n"
        "    ASSERT(1213 == mc.testInsideMidClass());\n"
        "    ASSERT(1011 == mc.testInsideBaseOverride());\n"
        "    ASSERT(1011 == mc.testInsideMidOverride());\n"
        "    SubClass  sc();\n"
        "    ASSERT(1617 == sc.testSuper());\n"
        "    ASSERT(1617 == sc.testOverride());\n"
        "    ASSERT(456  == sc.testNoOverride());\n"
        "    ASSERT(789  == sc.testInsideBaseClass());\n"
        "    ASSERT(1213 == sc.testInsideMidClass());\n"
        "    ASSERT(1617 == sc.testInsideSubClass());\n"
        "    ASSERT(1617 == sc.testInsideBaseOverride());\n"
        "    ASSERT(1617 == sc.testInsideMidOverride());\n"
        "    ASSERT(1617 == sc.testInsideSubOverride());\n"
        // Test polymorphism
        "    bc = mc;\n"
        "    ASSERT(1011 == bc.testOverride());\n"
        "    ASSERT(789  == bc.testInsideBaseClass());\n"
        "    ASSERT(1011 == bc.testInsideBaseOverride());\n"
        "    bc = sc;\n"
        "    ASSERT(1617 == bc.testOverride());\n"
        "    ASSERT(789  == bc.testInsideBaseClass());\n"
        "    ASSERT(1617 == bc.testInsideBaseOverride());\n"
        "    mc = sc;\n"
        "    ASSERT(1617 == mc.testSuper());\n"
        "    ASSERT(1617 == mc.testOverride());\n"
        "    ASSERT(789  == mc.testInsideBaseClass());\n"
        "    ASSERT(1213 == mc.testInsideMidClass());\n"
        "    ASSERT(1617 == mc.testInsideBaseOverride());\n"
        "    ASSERT(1617 == mc.testInsideMidOverride());\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceTestThis)
{
    ExecuteTest(
        "public class BaseClass {\n"
        "    int a = 123;\n"
        "    int b = 456;\n"
        "    int c = 789;\n"
        "    void testBaseMembersAndParams(int a, int b, int c) {\n"
        "        ASSERT(a != 123);\n"
        "        ASSERT(b != 456);\n"
        "        ASSERT(c != 789);\n"
        "        ASSERT(this.a == 123);\n"
        "        ASSERT(this.b == 456);\n"
        "        ASSERT(this.c == 789);\n"
        "    }\n"
        "    BaseClass testSuperReturnThis(){ return this; }\n"
        "    BaseClass testReturnThisFromBaseClass() { return this; }\n"
        "}\n"
        "public class MidClass extends BaseClass {\n"
        "    int b = 1011;\n"
        "    int c = 1213;\n"
        "    int d = 1415;\n"
        "    void testMidMembersAndParams(int a, int b, int c, int d) {\n"
        "        ASSERT(a != 123);\n"
        "        ASSERT(b != 1011);\n"
        "        ASSERT(c != 1213);\n"
        "        ASSERT(d != 1415);\n"
        "        ASSERT(this.a == 123);\n"
        "        ASSERT(this.b == 1011);\n"
        "        ASSERT(this.c == 1213);\n"
        "        ASSERT(this.d == 1415);\n"
        "    }\n"
        "    MidClass testSuperReturnThis(){ return super.testSuperReturnThis(); }\n"
        "    MidClass testReturnThisFromMidClass() { return this; }\n"
        "}\n"
        "public class SubClass extends MidClass {\n"
        "    int c = 1617;\n"
        "    int d = 1819;\n"
        "    int e = 2021;\n"
        "    void testSubMembersAndParams(int a, int b, int c, int d, int e) {\n"
        "        ASSERT(a != 123);\n"
        "        ASSERT(b != 1011);\n"
        "        ASSERT(c != 1617);\n"
        "        ASSERT(d != 1819);\n"
        "        ASSERT(e != 2021);\n"
        "        ASSERT(this.a == 123);\n"
        "        ASSERT(this.b == 1011);\n"
        "        ASSERT(this.c == 1617);\n"
        "        ASSERT(this.d == 1819);\n"
        "        ASSERT(this.e == 2021);\n"
        "    }\n"
        "    SubClass testSuperReturnThis(){ return super.testSuperReturnThis(); }\n"
        "    SubClass testReturnThisFromSubClass() { return this; }\n"
        "}\n"
        "extern void ClassInheritanceTestThis()\n"
        "{\n"
        "    BaseClass bc();\n"
        "    MidClass  mc();\n"
        "    SubClass  sc();\n"
        "    ASSERT(bc == bc.testSuperReturnThis());\n"
        "    ASSERT(bc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    ASSERT(mc == mc.testSuperReturnThis());\n"
        "    ASSERT(mc == mc.testReturnThisFromBaseClass());\n"
        "    ASSERT(mc == mc.testReturnThisFromMidClass());\n"
        "                 mc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 mc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "    ASSERT(sc == sc.testSuperReturnThis());\n"
        "    ASSERT(sc == sc.testReturnThisFromBaseClass());\n"
        "    ASSERT(sc == sc.testReturnThisFromMidClass());\n"
        "    ASSERT(sc == sc.testReturnThisFromSubClass());\n"
        "                 sc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 sc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "                 sc.testSubMembersAndParams(-1, -2, -3, -4, -5);\n"
        // Test polymorphism
        "    bc = mc;\n"
        "    ASSERT(mc == bc.testSuperReturnThis());\n"
        "    ASSERT(mc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    bc = sc;\n"
        "    ASSERT(sc == bc.testSuperReturnThis());\n"
        "    ASSERT(sc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    mc = sc;\n"
        "    ASSERT(sc == mc.testSuperReturnThis());\n"
        "    ASSERT(sc == mc.testReturnThisFromBaseClass());\n"
        "    ASSERT(sc == mc.testReturnThisFromMidClass());\n"
        "                 mc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 mc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassCompileCircularReference_Issue433)
{
    ExecuteTest(
        "public class OtherClass {\n"
        "    TestClass testclass;\n"
        "}\n"
        "public class TestClass {\n"
        "    int test;\n"
        "    OtherClass otherclass;\n"
        "}\n"
        "extern void TestCompileCircularReference()\n"
        "{\n"
        "    TestClass t();\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassTestClassDefinedAfterReference)
{
    ExecuteTest(
        "public class OtherClass {\n"
        "    TestClass testclass = new TestClass();\n"
        "}\n"
        "public class TestClass {\n"
        "    int test = 246;\n"
        "}\n"
        "extern void TestDefinedAfterReference()\n"
        "{\n"
        "    OtherClass o();\n"
        "    TestClass t = o.testclass;\n"
        "    ASSERT(t.test == 246);\n"
        "}\n"
    );
}

TEST_F(CBotUT, String)
{
    ExecuteTest(
        "extern void StringTest()\n"
        "{\n"
        "    string a = \"Colo\";\n"
        "    string b = \"bot\";\n"
        "    string c = a + b + \"!\";\n"
        "    ASSERT(a == \"Colo\");\n"
        "    ASSERT(b == \"bot\");\n"
        "    ASSERT(c == \"Colobot!\");\n"
        "}\n"
    );

    ExecuteTest(
        "extern void MissingEndQuote()\n"
        "{\n"
        "    \"Colobot...\n"
        "}\n",
        CBotErrEndQuote
    );
}

TEST_F(CBotUT, StringEscapeCodes)
{
    ExecuteTest(
        "extern void HexEscapeCodes()\n"
        "{\n"
        "    ASSERT(\"  \\x07  \" == \"  \\a  \");\n"
        "    ASSERT(\"  \\x08  \" == \"  \\b  \");\n"
        "    ASSERT(\"  \\x09  \" == \"  \\t  \");\n"
        "    ASSERT(\"  \\x0A  \" == \"  \\n  \");\n"
        "    ASSERT(\"  \\x0B  \" == \"  \\v  \");\n"
        "    ASSERT(\"  \\x0C  \" == \"  \\f  \");\n"
        "    ASSERT(\"  \\x0D  \" == \"  \\r  \");\n"
        "    ASSERT(\"  \\x22  \" == \"  \\\"  \");\n"
        "    ASSERT(\"  \\x27  \" == \"  \\\'  \");\n"
        "    ASSERT(\"  \\x5C  \" == \"  \\\\  \");\n"
        "    string test = \"\\x31 \\x32 \\x33\";\n"
        "    ASSERT(test == \"1 2 3\");\n"
        "}\n"
        "extern void OctalEscapeCodes()\n"
        "{\n"
        "    ASSERT(\"  \\000  \" == \"  \\x00  \");\n"
        "    ASSERT(\"  \\007  \" == \"  \\x07  \");\n"
        "    ASSERT(\"  \\010  \" == \"  \\x08  \");\n"
        "    ASSERT(\"  \\011  \" == \"  \\x09  \");\n"
        "    ASSERT(\"  \\012  \" == \"  \\x0A  \");\n"
        "    ASSERT(\"  \\013  \" == \"  \\x0B  \");\n"
        "    ASSERT(\"  \\014  \" == \"  \\x0C  \");\n"
        "    ASSERT(\"  \\015  \" == \"  \\x0D  \");\n"
        "    ASSERT(\"  \\042  \" == \"  \\x22  \");\n"
        "    ASSERT(\"  \\047  \" == \"  \\x27  \");\n"
        "    ASSERT(\"  \\134  \" == \"  \\x5C  \");\n"
        "    string test = \"\\101 \\102 \\103\";\n"
        "    ASSERT(test == \"A B C\");\n"
        "}\n"
        "extern void UnicodeEscapeCodesToUTF_8()\n"
        "{\n"
        "    ASSERT(\"  \\u0000  \" == \"  \\0  \");\n"
        "    ASSERT(\"  \\u0007  \" == \"  \\a  \");\n"
        "    ASSERT(\"  \\u0008  \" == \"  \\b  \");\n"
        "    ASSERT(\"  \\u0009  \" == \"  \\t  \");\n"
        "    ASSERT(\"  \\u000A  \" == \"  \\n  \");\n"
        "    ASSERT(\"  \\u000B  \" == \"  \\v  \");\n"
        "    ASSERT(\"  \\u000C  \" == \"  \\f  \");\n"
        "    ASSERT(\"  \\u000D  \" == \"  \\r  \");\n"
        "    ASSERT(\"  \\u0022  \" == \"  \\\"  \");\n"
        "    ASSERT(\"  \\u0027  \" == \"  \\\'  \");\n"
        "    ASSERT(\"  \\u005C  \" == \"  \\\\  \");\n"
        "\n"
        "    ASSERT(\"\\u00A9\" == \"\\xC2\\xA9\");\n"
        "    ASSERT(\"\\u00AE\" == \"\\xC2\\xAE\");\n"
        "    ASSERT(\"\\u262E\" == \"\\xE2\\x98\\xAE\");\n"
        "    ASSERT(\"\\u262F\" == \"\\xE2\\x98\\xAF\");\n"
        "    ASSERT(\"\\U0001F60E\" == \"\\xF0\\x9F\\x98\\x8E\");\n"
        "    ASSERT(\"\\U0001F61C\" == \"\\xF0\\x9F\\x98\\x9C\");\n"
        "    ASSERT(\"\\U0001F6E0\" == \"\\xF0\\x9F\\x9B\\xA0\");\n"
        "}\n"
        "extern void UnicodeMaxCharacterNameToUTF_8()\n"
        "{\n"
        "    ASSERT(\"\\U0010FFFF\" == \"\\xF4\\x8F\\xBF\\xBF\");\n"
        "}\n"
    );
}

TEST_F(CBotUT, StringEscapeCodeErrors)
{
    ExecuteTest(
        "extern void UnknownEscapeSequence()\n"
        "{\n"
        "    \"Unknown: \\p \";\n"
        "}\n",
        CBotErrBadEscape
    );

    ExecuteTest(
        "extern void MissingHexDigits()\n"
        "{\n"
        "    \"  \\x  \";\n"
        "}\n",
        CBotErrHexDigits
    );

    ExecuteTest(
        "extern void HexValueOutOfRange()\n"
        "{\n"
        "    \"  \\x100  \";\n"
        "}\n",
        CBotErrHexRange
    );

    ExecuteTest(
        "extern void OctalValueOutOfRange()\n"
        "{\n"
        "    \"  \\400  \";\n"
        "}\n",
        CBotErrOctalRange
    );

    ExecuteTest(
        "extern void BadUnicodeCharacterName()\n"
        "{\n"
        "    \"  \\U00110000  \";\n"
        "}\n",
        CBotErrUnicodeName
    );
}

// TODO: not implemented, see issue #694
TEST_F(CBotUT, DISABLED_StringAsArray)
{
    ExecuteTest(
        "extern void StringAsArray()\n"
        "{\n"
        "    string s = \"Colobot\";\n"
        "    ASSERT(s[0] == \"C\");\n"
        "    ASSERT(s[3] == \"o\");\n"
        "    s[2] = \"L\"; s[4] = \"B\"; s[6] = \"T\";\n"
        "    ASSERT(s == \"CoLoBoT\");\n"
        "}\n"
    );
}

TEST_F(CBotUT, ArraysOfStrings)
{
    ExecuteTest(
        "extern void ArraysOfStrings()\n"
        "{\n"
        "    string[] a;\n"
        "    string b[];\n"
        "    string c[5];\n"
        "    string d[] = {\"test\"};\n"
        "    d[1] = \"test2\";\n"
        "    ASSERT(d[0] == \"test\");\n"
        "    ASSERT(d[1] == \"test2\");\n"
        "}\n"
    );
}

TEST_F(CBotUT, StringFunctions)
{
    ExecuteTest(
        "extern void StringFunctions()\n"
        "{\n"
        "    string s = \"Colobot\";\n"
        "    ASSERT(strlen(s) == 7);\n"
        "    ASSERT(strlower(s) == \"colobot\");\n"
        "    ASSERT(strupper(s) == \"COLOBOT\");\n"
        "    ASSERT(strleft(s, 3) == \"Col\");\n"
        "    ASSERT(strright(s, 3) == \"bot\");\n"
        "    ASSERT(strmid(s, 1, 3) == \"olo\");\n"
        "    ASSERT(strfind(s, \"o\") == 1);\n"
        "    ASSERT(strval(\"2.5\") == 2.5);\n"
        "}\n"
        "extern void StringFunctionsOutOfRange()\n"
        "{\n"
        "    ASSERT(strmid(\"asdf\", 5, 1) == \"\");\n"
        "    ASSERT(strmid(\"asdf\", 0, 100) == \"asdf\");\n"
        "    ASSERT(strmid(\"asdf\", -500, 100) == \"asdf\");\n"
        "    ASSERT(strleft(\"asdf\", 15) == \"asdf\");\n"
        "    ASSERT(strleft(\"asdf\", -15) == \"\");\n"
        "    ASSERT(strright(\"asdf\", 15) == \"asdf\");\n"
        "    ASSERT(strright(\"asdf\", -15) == \"\");\n"
        "}\n"
    );
}

TEST_F(CBotUT, LiteralCharacters)
{
    ExecuteTest(
        "extern void TestCharValue()\n"
        "{\n"
        "    ASSERT('A' == 65);\n"
        "    ASSERT('B' == 66);\n"
        "    ASSERT('C' == 67);\n"
        "    ASSERT('\\a' == 0x07);\n"
        "    ASSERT('\\b' == 0x08);\n"
        "    ASSERT('\\t' == 0x09);\n"
        "    ASSERT('\\n' == 0x0A);\n"
        "    ASSERT('\\v' == 0x0B);\n"
        "    ASSERT('\\f' == 0x0C);\n"
        "    ASSERT('\\r' == 0x0D);\n"
        "    ASSERT('\\\"' == 0x22);\n"
        "    ASSERT('\\\'' == 0x27);\n"
        "    ASSERT('\\\\' == 0x5C);\n"
        "}\n"
        "extern void TestCharUnicodeEscape()\n"
        "{\n"
        "    ASSERT('\\u0007' == '\\a');\n"
        "    ASSERT('\\u0008' == '\\b');\n"
        "    ASSERT('\\u0009' == '\\t');\n"
        "    ASSERT('\\u000A' == '\\n');\n"
        "    ASSERT('\\u000B' == '\\v');\n"
        "    ASSERT('\\u000C' == '\\f');\n"
        "    ASSERT('\\u000D' == '\\r');\n"
        "    ASSERT('\\u0022' == '\\\"');\n"
        "    ASSERT('\\u0027' == '\\\'');\n"
        "    ASSERT('\\u005C' == '\\\\');\n"
        "}\n"
        "extern void AssignCharToString_ToUTF_8()\n"
        "{\n"
        "    string test = '\\u00A9';\n"
        "    test += '\\u00AE';\n"
        "    ASSERT(test == \"\\xC2\\xA9\\xC2\\xAE\");\n"
        "}\n"
        "extern void AddCharToString_ToUTF_8()\n"
        "{\n"
        "    ASSERT(\"\" + 'A' + 'B' + 'C' == \"ABC\");\n"
        "    ASSERT(\"\" + '\\u00A9' == \"\\xC2\\xA9\");\n"
        "    ASSERT(\"\" + '\\u00AE' == \"\\xC2\\xAE\");\n"
        "    ASSERT(\"\" + '\\u262E' == \"\\xE2\\x98\\xAE\");\n"
        "    ASSERT(\"\" + '\\u262F' == \"\\xE2\\x98\\xAF\");\n"
        "    ASSERT(\"\" + '\\U0001F60E' == \"\\xF0\\x9F\\x98\\x8E\");\n"
        "    ASSERT(\"\" + '\\U0001F61C' == \"\\xF0\\x9F\\x98\\x9C\");\n"
        "    ASSERT(\"\" + '\\U0001F6E0' == \"\\xF0\\x9F\\x9B\\xA0\");\n"
        "    ASSERT(\"\" + '\\U0010FFFF' == \"\\xF4\\x8F\\xBF\\xBF\");\n"
        "}\n"
    );

    ExecuteTest(
        "extern void MissingEndQuote()\n"
        "{\n"
        "    '\n"
        "}\n",
        CBotErrEndQuote
    );

    ExecuteTest(
        "extern void MissingEndQuote()\n"
        "{\n"
        "    'a\n"
        "}\n",
        CBotErrEndQuote
    );

    ExecuteTest(
        "extern void EmptyQuotes()\n"
        "{\n"
        "    '';\n"
        "}\n",
        CBotErrCharEmpty
    );

    ExecuteTest(
        "extern void UnknownEscapeSequence()\n"
        "{\n"
        "    '\\p';\n"
        "}\n",
        CBotErrBadEscape
    );

    ExecuteTest(
        "extern void MissingHexDigits()\n"
        "{\n"
        "    '\\u';\n"
        "}\n",
        CBotErrHexDigits
    );

    ExecuteTest(
        "extern void BadUnicodeCharacterName()\n"
        "{\n"
        "    '\\U00110000';\n"
        "}\n",
        CBotErrUnicodeName
    );
}

TEST_F(CBotUT, TestNANParam_Issue642)
{
    ExecuteTest(
        "float test(float x) {\n"
        "    ASSERT(x == nan);\n"
        "    return x;\n"
        "}\n"
        "extern void TestNANParam() {\n"
        "    ASSERT(nan == nan);\n" // TODO: Shouldn't it be nan != nan ??
        "    ASSERT(test(nan) == nan);\n"
        "}\n"
    );
}

TEST_F(CBotUT, TestArrayInitialization)
{
    ExecuteTest(
        "extern void TestArrayInitialization() {\n"
        "    int[] a = {1, 2, 3};\n"
        "    ASSERT(sizeof(a) == 3);\n"
        "    ASSERT(a[0] == 1);\n"
        "    ASSERT(a[1] == 2);\n"
        "    ASSERT(a[2] == 3);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestArrayInitializationOutOfRange() {\n"
        "    int a[2] = {1, 2, 3};\n"
        "}\n",
        CBotErrOutArray
    );

    ExecuteTest(
        "extern void TestArrayInitializationSmallerThanRange() {\n"
        "    int a[4] = {1, 2, 3};\n"
        "    ASSERT(sizeof(a) == 3);\n"
        "    ASSERT(a[0] == 1);\n"
        "    ASSERT(a[1] == 2);\n"
        "    ASSERT(a[2] == 3);\n"
        "    a[3] = 4;\n"
        "    ASSERT(sizeof(a) == 4);\n"
        "    ASSERT(a[3] == 4);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestArrayInitializationLimitUnchanged() {\n"
        "    int a[4] = {1, 2, 3};\n"
        "    a[4] = 5;\n"
        "}\n",
        CBotErrOutArray
    );

    ExecuteTest(
        "extern void TestArrayInitializationWithVars() {\n"
        "    int x=1, y=2, z=3;\n"
        "    int i[] = { x, y, z };\n"
        "    ASSERT(i[0] == 1);\n"
        "    ASSERT(i[1] == 2);\n"
        "    ASSERT(i[2] == 3);\n"
        "    i[0] += 1;\n"
        "    ASSERT(i[0] == 2);\n"
        "    ASSERT(x == 1);\n"
        "}\n"
    );
}

TEST_F(CBotUT, TestArrayFunctionReturn)
{
    ExecuteTest(
        "int[] test() {\n"
        "    int[] a = {1, 2, 3};\n"
        "    return a;"
        "}\n"
        "extern void TestArrayFunctionReturn() {\n"
        "    int[] b = test();\n"
        "    ASSERT(sizeof(b) == 3);\n"
        "    ASSERT(b[0] == 1);\n"
        "    ASSERT(b[1] == 2);\n"
        "    ASSERT(b[2] == 3);\n"
        "}\n"
    );
}

TEST_F(CBotUT, AccessMembersInParameters_Issue256)
{
    ExecuteTest(
        "public class Test1 {\n"
        "    int x = 1337;\n"
        "}\n"
        "public class Test2 {\n"
        "    public bool test(int a) {\n"
        "        return a == 1337;\n"
        "    }\n"
        "}\n"
        "public class Test3 {\n"
        "    public Test1 test1 = new Test1();\n"
        "    public Test2 test2 = new Test2();\n"
        "    public void test() {\n"
        "        ASSERT(test2.test(test1.x));\n"
        "    }\n"
        "}\n"
        "extern void AccessMembersInParameters() {\n"
        "    Test3 t();\n"
        "    t.test();\n"
        "}\n"
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVoid)
{
    ExecuteTest(
        "void Test() {}\n"
        "extern void TestAccessMemberVoid() {\n"
        "    Test().x;\n"
        "}\n",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, InstrCallAccessMemberNonObject)
{
    ExecuteTest(
        "int GetInt() {\n"
        "    return 1;\n"
        "}\n"
        "extern void TestAccessMemberNonObject() {\n"
        "    GetInt().x;\n"
        "}\n",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, InstrCallAccessMemberObjectNull)
{
    ExecuteTest(
        "public class TestClass { int x = 1; }\n"
        "TestClass GetObjectNull() {\n"
        "    TestClass t = null;"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberObjectNull() {\n"
        "    GetObjectNull().x;\n"
        "}\n",
        CBotErrNull
    );
}

TEST_F(CBotUT, InstrCallAccessMemberReturnNull)
{
    ExecuteTest(
        "public class TestClass { int x = 1; }\n"
        "TestClass GetReturnNull() {\n"
        "    return null;\n"
        "}\n"
        "extern void TestAccessMemberReturnNull() {\n"
        "    GetReturnNull().x;\n"
        "}\n",
        CBotErrNull
    );
}

TEST_F(CBotUT, InstrCallAccessMemberNotVar)
{
    ExecuteTest(
        "public class TestClass {}\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberNotVar() {\n"
        "    TestClass tc();\n"
        "    GetObject(tc).123;\n"
        "}\n",
        CBotErrUndefClass
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarNonMember)
{
    ExecuteTest(
        "public class TestClass { int x = 1; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarNonMember() {\n"
        "    TestClass tc();\n"
        "    GetObject(tc).y;\n"
        "}\n",
        CBotErrUndefItem
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarUndefined)
{
    ExecuteTest(
        "public class TestClass { int x; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarUndefined() {\n"
        "    TestClass tc();\n"
        "    GetObject(tc).x;\n"
        "}\n",
        CBotErrNotInit
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarPrivate)
{
    ExecuteTest(
        "public class TestClass { private int x = 123; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarPrivate() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetObject(tc).x);\n"
        "}\n",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVar)
{
    ExecuteTest(
        "public class TestClass { int x = 123; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVar() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetObject(tc).x);\n"
        "}\n"
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarArrayBadIndex)
{
    ExecuteTest(
        "public class TestClass { int[] a; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarArrayEmpty() {\n"
        "    TestClass tc();\n"
        "    int i = GetObject(tc).a[4.7];\n"
        "}\n",
        CBotErrBadIndex
    );
}
TEST_F(CBotUT, InstrCallAccessMemberVarArrayCloseIndex)
{
    ExecuteTest(
        "public class TestClass { int[] a = {123}; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarArrayEmpty() {\n"
        "    TestClass tc();\n"
        "    int i = GetObject(tc).a[0;\n"
        "}\n",
        CBotErrCloseIndex
    );
}
TEST_F(CBotUT, InstrCallAccessMemberVarArrayEmpty)
{
    ExecuteTest(
        "public class TestClass { int[] a; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarArrayEmpty() {\n"
        "    TestClass tc();\n"
        "    int i = GetObject(tc).a[0];\n"
        "}\n",
        CBotErrOutArray
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarArrayOutOfRange)
{
    ExecuteTest(
        "public class TestClass { int a[] = {123}; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarArrayOut() {\n"
        "    TestClass tc();\n"
        "    int i = GetObject(tc).a[1];\n"
        "}\n",
        CBotErrOutArray
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarArray)
{
    ExecuteTest(
        "public class TestClass { int a[] = {123}; }\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberVarArray() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetObject(tc).a[0]);\n"
        "}\n"
    );
}

TEST_F(CBotUT, InstrCallAccessMemberMethod)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int x = 123;\n"
        "    int testGetX() { return x; }\n"
        "}\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberMethod() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetObject(tc).testGetX());\n"
        "}\n"
    );
}

TEST_F(CBotUT, InstrCallAccessMemberMethodChain)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int x = 123;\n"
        "    TestClass testGetThis() { return this; }\n"
        "    int testGetX() { return x; }\n"
        "}\n"
        "TestClass GetObject(TestClass t) {\n"
        "    return t;\n"
        "}\n"
        "extern void TestAccessMemberMethodChain() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetObject(tc).testGetThis().testGetX());\n"
        "}\n"
    );
}

TEST_F(CBotUT, InstrCallAccessMemberNewObjectDestructor)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int x = 123;\n"
        "    static bool b = false;\n"
        "    void ~TestClass() { b = true; }\n"
        "}\n"
        "TestClass GetNewObject() { return new TestClass(); }\n"
        "extern void TestAccessMemberNewObject() {\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == GetNewObject().x);\n"
        "    ASSERT(tc.b == true);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassConstructorMethodChain)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int a = 123;\n"
        "    int b = 246;\n"
        "    TestClass testSetA(int x) { a = x; return this; }\n"
        "    TestClass testSetB(int y) { b = y; return this; }\n"
        "}\n"
        "extern void ConstructorMethodChain() {\n"
        "    TestClass tc().testSetA(111).testSetB(222);\n"
        "    ASSERT(tc.a == 111);\n"
        "    ASSERT(tc.b == 222);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassNewConstructorMethodChain)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    int a = 123;\n"
        "    int b = 246;\n"
        "    TestClass testSetA(int x) { a = x; return this; }\n"
        "    TestClass testSetB(int y) { b = y; return this; }\n"
        "}\n"
        "extern void NewConstructorMethodChain() {\n"
        "    TestClass tc;\n"
        "    tc = new TestClass().testSetA(111).testSetB(222);\n"
        "    ASSERT(tc.a == 111);\n"
        "    ASSERT(tc.b == 222);\n"
        "}\n"
    );
}

TEST_F(CBotUT, PassNullAsArgument)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass {}\n"
        "public class SubClass extends BaseClass {}\n"
    );

    ExecuteTest(
        "bool Test(BaseClass b) {\n"
        "    return (b == null);\n"
        "}\n"
        "extern void PassNullAsArgument() {\n"
        "    ASSERT(true == Test(null));\n"
        "}\n"
    );

    ExecuteTest(
        "void Test(BaseClass b) {}\n"
        "void Test(SubClass s) {}\n"
        "\n"
        "extern void AmbiguousCallArgumentNull() {\n"
        "    Test(null);\n"
        "}\n",
        CBotErrAmbiguousCall
    );
}

TEST_F(CBotUT, ClassImplicitCastArguments)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass { int a = 360; }\n"
        "public class SubClass extends BaseClass {}\n"
    );

    ExecuteTest(
        "bool Test(BaseClass b) {\n"
        "    SubClass s = b;\n"
        "    return (360 == s.a);\n"
        "}\n"
        "extern void UpcastPassingArguments() {\n"
        "    ASSERT(true == Test(new SubClass()));\n"
        "}\n"
    );

    ExecuteTest(
        "void Test(BaseClass b, SubClass s) {}\n"
        "void Test(SubClass s, BaseClass b) {}\n"
        "\n"
        "extern void UpcastAmbiguousCall() {\n"
        "    Test(new SubClass(), new SubClass());\n"
        "}\n",
        CBotErrAmbiguousCall
    );

    ExecuteTest(
        "bool Test(BaseClass b, SubClass s) { return false; }\n"
        "bool Test(SubClass s, BaseClass b) { return false; }\n"
        "bool Test(SubClass s, SubClass s2) { return true; }\n"
        "\n"
        "extern void NoErrorMoreSpecific() {\n"
        "    ASSERT(true == Test(new SubClass(), new SubClass()));\n"
        "}\n"
    );
}

TEST_F(CBotUT, AmbiguousCallWithNumbers)
{
    ExecuteTest(
        "void Test(int i, float f) {}\n"
        "void Test(float f, int i) {}\n"
        "\n"
        "extern void AmbiguousCallNumbers() {\n"
        "    Test(1, 2);\n"
        "}\n",
        CBotErrAmbiguousCall
    );

    ExecuteTest(
        "bool Test(int i, float f) { return false; }\n"
        "bool Test(float f, int i) { return false; }\n"
        "bool Test(int i, int ii)  { return true; }\n"
        "\n"
        "extern void NoErrorMoreSpecific() {\n"
        "    ASSERT(true == Test(1, 2));\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassMethodWithPublicKeyword)
{
    auto publicProgram = ExecuteTest(
        "public class TestClass {\n"
        "    public int Test() { return 1; }\n"
        "}\n"
    );

    ExecuteTest(
        "int Test() { return 2; }\n"
        "\n"
        "extern void DontCallMethodInTestClass()\n"
        "{\n"
        "    ASSERT(2 == Test());\n"
        "}\n"
    );

    ExecuteTest(
        "int Test() { return 2; }\n"
        "\n"
        "public class OtherClass {}\n"
        "\n"
        "extern void OtherClass::TestCallWithThis()\n"
        "{\n"
        "    this.Test();\n"
        "}\n",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassTestProtectedMember)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass {\n"
        "    protected int a_protected = 1;\n"
        "    bool test() {\n"
        "        a_protected = 1;\n"
        "        int a = a_protected;\n"
        "        return true;\n"
        "    }\n"
        "}\n"
        "extern void Test() {\n"
        "    BaseClass b();\n"
        "    ASSERT(true == b.test());\n"
        "}\n"
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    bool testProtected() {\n"
        "        a_protected = 1;\n"
        "        int a = a_protected;\n"
        "        return true;\n"
        "    }\n"
        "}\n"
        "extern void TestSubClassAccessProtected() {\n"
        "    SubClass  s();\n"
        "    ASSERT(true == s.test());\n"
        "    ASSERT(true == s.testProtected());\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestErrorProtected() {\n"
        "    BaseClass b();\n"
        "    int i = b.a_protected;\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "extern void ErrorProtectedAssignment() {\n"
        "    BaseClass b();\n"
        "    b.a_protected = 1;\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void testErrorProtected() {\n"
        "        BaseClass b();\n"
        "        int i = b.a_protected;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void testErrorProtectedAssignment() {\n"
        "        BaseClass b();\n"
        "        b.a_protected = 1;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestPrivateMember)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass {\n"
        "    private int a_private = 2;\n"
        "\n"
        "    bool test() {\n"
        "        a_private = 2;\n"
        "        int a = a_private;\n"
        "        return true;\n"
        "    }\n"
        "    bool NoErrorPrivateSameClass() {\n"
        "        BaseClass b = new BaseClass();\n"
        "        int a = b.a_private;\n"
        "        b.a_private = 2;\n"
        "        return true;\n"
        "    }\n"
        "}\n"
        "extern void Test() {\n"
        "    BaseClass b();\n"
        "    ASSERT(true == b.test());\n"
        "    ASSERT(true == b.NoErrorPrivateSameClass());\n"
        "}\n"
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    void testErrorPrivate() {\n"
        "        int a = a_private;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    void testErrorPrivateAssignment() {\n"
        "        a_private = 2;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "extern void TestErrorPrivate() {\n"
        "    BaseClass b();\n"
        "    int i = b.a_private;\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "extern void ErrorPrivateAssignment() {\n"
        "    BaseClass b();\n"
        "    b.a_private = 2;\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void testErrorPrivate() {\n"
        "        BaseClass b();\n"
        "        int i = b.a_private;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void testErrorPrivateAssignment() {\n"
        "        BaseClass b();\n"
        "        b.a_private = 1;\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, IncrementDecrementSyntax)
{
    auto publicProgram = ExecuteTest(
        "public class TestClass {\n"
        "    int GetInt() { return 1; }\n"
        "}\n"
        "extern void TestIncrementDecrement()\n"
        "{\n"
        "    int i = 1;\n"
        "    ASSERT(2 == ++i);\n"
        "    ASSERT(2 == i++);\n"
        "    ASSERT(3 ==  i );\n"
        "    ASSERT(2 == --i);\n"
        "    ASSERT(2 == i--);\n"
        "    ASSERT(1 ==  i );\n"
        "}\n"
    );

    ExecuteTest(
        "extern void PreIncrementMethodCall()\n"
        "{\n"
        "    TestClass tc();\n"
        "    ++tc.GetInt();\n"
        "}\n",
        CBotErrBadType1
    );

    ExecuteTest(
        "extern void PostIncrementMethodCall()\n"
        "{\n"
        "    TestClass tc();\n"
        "    tc.GetInt()++;\n"
        "}\n",
        CBotErrBadType1
    );

    ExecuteTest(
        "extern void BadPreIncrementEmpty()\n"
        "{\n"
        "    ++;\n"
        "}\n",
        CBotErrBadType1
    );

    ExecuteTest(
        "extern void BadPreIncrementNotAVar()\n"
        "{\n"
        "    ++123;\n"
        "}\n",
        CBotErrBadType1
    );
}

TEST_F(CBotUT, ParametersWithDefaultValues)
{
    ExecuteTest(
        "extern void ParametersWithDefaultValues() {\n"
        "    ASSERT(true == Test());\n"
        "    ASSERT(true == Test(1));\n"
        "    ASSERT(true == Test(1, 2));\n"
        "}\n"
        "bool Test(int i = 1, float f = 2.0) {\n"
        "    return (i == 1) && (f == 2.0);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void NotUsingDefaultValues() {\n"
        "    ASSERT(true == Test(2, 4.0));\n"
        "}\n"
        "bool Test(int i = 1, float f = 2.0) {\n"
        "    return (i == 2) && (f == 4.0);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void NextParamNeedsDefaultValue() {\n"
        "}\n"
        "void Test(int i = 1, float f) {}\n"
        "\n",
        CBotErrDefaultValue
    );

    ExecuteTest(
        "extern void ParamMissingExpression() {\n"
        "}\n"
        "void Test(int i = 1, float f = ) {}\n"
        "\n",
        CBotErrNoExpression
    );

    ExecuteTest(
        "extern void ParamDefaultBadType() {\n"
        "}\n"
        "void Test(int i = 1, float f = null) {}\n"
        "\n",
        CBotErrBadType1
    );

    ExecuteTest(
        "extern void DefaultValuesAmbiguousCall() {\n"
        "    Test();\n"
        "}\n"
        "void Test(int i = 1) {}\n"
        "void Test(float f = 2.0) {}\n"
        "\n",
        CBotErrAmbiguousCall
    );

    ExecuteTest(
        "extern void AmbiguousCallOneDefault() {\n"
        "    Test(1);\n"
        "}\n"
        "void Test(int i, float f = 2) {}\n"
        "void Test(int i) {}\n"
        "\n",
        CBotErrAmbiguousCall
    );

    ExecuteTest(
        "extern void DifferentNumberOfDefaultValues() {\n"
        "    Test(1, 2.0);\n"
        "}\n"
        "void Test(int i, float f = 2.0) {}\n"
        "void Test(int i, float f = 2.0, int ii = 1) {}\n"
        "\n",
        CBotErrAmbiguousCall
    );

    ExecuteTest(
        "extern void DefaultValueUnaryExpression() {\n"
        "    TestNumbers();\n"
        "    TestBool();\n"
        "}\n"
        "void TestNumbers(int i = -1, float f = -1, int ii = ~1) {\n"
        "    ASSERT(i == -1);\n"
        "    ASSERT(f == -1.0);\n"
        "    ASSERT(ii == ~1);\n"
        "}\n"
        "void TestBool(bool b = !false, bool b2 = not false) {\n"
        "    ASSERT(true == b);\n"
        "    ASSERT(true == b2);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassMethodsOutOfClass_Issue207)
{
    auto publicProgram = ExecuteTest(
        "public class OtherClass {}\n"
        "public class TestClass {}\n"
    );

    ExecuteTest(
        "extern void TestCallWithoutObject()\n"
        "{\n"
        "    TestMethod();\n"
        "}\n"
        "public void TestClass::TestMethod() {}\n",
        CBotErrUndefCall
    );

    ExecuteTest(
        "extern void TestCallWithWrongObject()\n"
        "{\n"
        "    OtherClass oc();\n"
        "    oc.TestMethod();\n"
        "}\n"
        "public void TestClass::TestMethod() {}\n",
        CBotErrUndefCall
    );

    ExecuteTest(
        "extern void OtherClass::TestCallWithWrongThis()\n"
        "{\n"
        "    this.TestMethod();\n"
        "}\n"
        "public void TestClass::TestMethod() {}\n",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassMethodsOutOfClass)
{
    auto publicProgram = ExecuteTest(
        "public class TestClass\n"
        "{\n"
        "    int i = 123, j = 456, k = 789;\n"
        "    int InsideClass()\n"
        "    {\n"
        "        ASSERT(456 == PublicMethod());\n"
        "        ASSERT(789 == NotPublicMethod());\n"
        "        return this.i;\n"
        "    }\n"
        "}\n"
        "extern void TestMethodsOutOfClass()\n"
        "{\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == tc.InsideClass());\n"
        "    ASSERT(456 == tc.PublicMethod());\n"
        "    ASSERT(789 == tc.NotPublicMethod());\n"
        "}\n"
        "public int TestClass::PublicMethod()\n"
        "{\n"
        "    return this.j;\n"
        "}\n"
        "int TestClass::NotPublicMethod()\n"
        "{\n"
        "    return k;\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestFromOtherProgram()\n"
        "{\n"
        "    TestClass tc();\n"
        "    ASSERT(123 == tc.InsideClass());\n"
        "    ASSERT(456 == tc.PublicMethod());\n"
        "    ASSERT(789 == tc.MethodInThisProgram());\n"
        "}\n"
        "int TestClass::MethodInThisProgram()\n"
        "{\n"
        "    ASSERT(123 == InsideClass());\n"
        "    ASSERT(456 == PublicMethod());\n"
        "    ASSERT(123 == this.InsideClass());\n"
        "    ASSERT(456 == this.PublicMethod());\n"
        "    ASSERT(i == 123 && this.j == 456);\n"
        "    return this.k;\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceMethodsOutOfClass)
{
    ExecuteTest(
        "public class BaseClass {\n"
        "    int a = 123;\n"
        "    int b = 456;\n"
        "    int c = 789;\n"
        "}\n"
        "int BaseClass::testOverride() { return 123; }\n"
        "int BaseClass::testNoOverride() { return 456; }\n"
        "int BaseClass::testInsideBaseClass() {\n"
        "    ASSERT(a == 123);\n"
        "    ASSERT(b == 456);\n"
        "    ASSERT(c == 789);\n"
        "    ASSERT(456 == testNoOverride());\n"
        "    return c;\n"
        "}\n"
        "int BaseClass::testInsideBaseOverride() { return testOverride(); }\n"
        "\n"
        "public class MidClass extends BaseClass {\n"
        "    int b = 1011;\n"
        "    int c = 1213;\n"
        "    int d = 1415;\n"
        "}\n"
        "int MidClass::testOverride() { return 1011; }\n"
        "int MidClass::testInsideMidClass() {\n"
        "    ASSERT(a == 123);\n"
        "    ASSERT(b == 1011);\n"
        "    ASSERT(c == 1213);\n"
        "    ASSERT(d == 1415);\n"
        "    ASSERT(456 == testNoOverride());\n"
        "    ASSERT(789 == testInsideBaseClass());\n"
        "    return c;\n"
        "}\n"
        "int MidClass::testSuper() {\n"
        "    ASSERT(super.a == 123);\n"
        "    ASSERT(super.b == 456);\n"
        "    ASSERT(super.c == 789);\n"
        "    ASSERT(123 == super.testOverride());\n"
        "    ASSERT(789 == super.testInsideBaseClass());\n"
        "    return super.testInsideBaseOverride();\n"
        "}\n"
        "int MidClass::testInsideMidOverride() { return testOverride(); }\n"
        "\n"
        "public class SubClass extends MidClass {\n"
        "    int c = 1617;\n"
        "    int d = 1819;\n"
        "    int e = 2021;\n"
        "}\n"
        "int SubClass::testOverride() { return 1617; }\n"
        "int SubClass::testInsideSubClass() {\n"
        "    ASSERT(a == 123);\n"
        "    ASSERT(b == 1011);\n"
        "    ASSERT(c == 1617);\n"
        "    ASSERT(d == 1819);\n"
        "    ASSERT(e == 2021);\n"
        "    ASSERT(456  == testNoOverride());\n"
        "    ASSERT(789  == testInsideBaseClass());\n"
        "    ASSERT(1213 == testInsideMidClass());\n"
        "    return c;\n"
        "}\n"
        "int SubClass::testSuper() {\n"
        "    ASSERT(super.a == 123);\n"
        "    ASSERT(super.b == 1011);\n"
        "    ASSERT(super.c == 1213);\n"
        "    ASSERT(super.d == 1415);\n"
        "    ASSERT(1011 == super.testOverride());\n"
        "    ASSERT(789  == super.testInsideBaseClass());\n"
        "    ASSERT(1213 == super.testInsideMidClass());\n"
        "    return super.testSuper();\n"
        "}\n"
        "int SubClass::testInsideSubOverride() { return testOverride(); }\n"
        "\n"
        "extern void InheritanceMethodsOutOfClass()\n"
        "{\n"
        "    BaseClass bc();\n"
        "    ASSERT(123 == bc.testOverride());\n"
        "    ASSERT(456 == bc.testNoOverride());\n"
        "    ASSERT(789 == bc.testInsideBaseClass());\n"
        "    ASSERT(123 == bc.testInsideBaseOverride());\n"
        "    MidClass  mc();\n"
        "    ASSERT(1011 == mc.testSuper());\n"
        "    ASSERT(1011 == mc.testOverride());\n"
        "    ASSERT(456  == mc.testNoOverride());\n"
        "    ASSERT(789  == mc.testInsideBaseClass());\n"
        "    ASSERT(1213 == mc.testInsideMidClass());\n"
        "    ASSERT(1011 == mc.testInsideBaseOverride());\n"
        "    ASSERT(1011 == mc.testInsideMidOverride());\n"
        "    SubClass  sc();\n"
        "    ASSERT(1617 == sc.testSuper());\n"
        "    ASSERT(1617 == sc.testOverride());\n"
        "    ASSERT(456  == sc.testNoOverride());\n"
        "    ASSERT(789  == sc.testInsideBaseClass());\n"
        "    ASSERT(1213 == sc.testInsideMidClass());\n"
        "    ASSERT(1617 == sc.testInsideSubClass());\n"
        "    ASSERT(1617 == sc.testInsideBaseOverride());\n"
        "    ASSERT(1617 == sc.testInsideMidOverride());\n"
        "    ASSERT(1617 == sc.testInsideSubOverride());\n"
        // Test polymorphism
        "    bc = mc;\n"
        "    ASSERT(1011 == bc.testOverride());\n"
        "    ASSERT(789  == bc.testInsideBaseClass());\n"
        "    ASSERT(1011 == bc.testInsideBaseOverride());\n"
        "    bc = sc;\n"
        "    ASSERT(1617 == bc.testOverride());\n"
        "    ASSERT(789  == bc.testInsideBaseClass());\n"
        "    ASSERT(1617 == bc.testInsideBaseOverride());\n"
        "    mc = sc;\n"
        "    ASSERT(1617 == mc.testSuper());\n"
        "    ASSERT(1617 == mc.testOverride());\n"
        "    ASSERT(789  == mc.testInsideBaseClass());\n"
        "    ASSERT(1213 == mc.testInsideMidClass());\n"
        "    ASSERT(1617 == mc.testInsideBaseOverride());\n"
        "    ASSERT(1617 == mc.testInsideMidOverride());\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassInheritanceTestThisOutOfClass)
{
    ExecuteTest(
        "public class BaseClass {\n"
        "    int a = 123;\n"
        "    int b = 456;\n"
        "    int c = 789;\n"
        "}\n"
        "void BaseClass::testBaseMembersAndParams(int a, int b, int c) {\n"
        "    ASSERT(a != 123);\n"
        "    ASSERT(b != 456);\n"
        "    ASSERT(c != 789);\n"
        "    ASSERT(this.a == 123);\n"
        "    ASSERT(this.b == 456);\n"
        "    ASSERT(this.c == 789);\n"
        "}\n"
        "BaseClass BaseClass::testSuperReturnThis(){ return this; }\n"
        "BaseClass BaseClass::testReturnThisFromBaseClass() { return this; }\n"
        "\n"
        "public class MidClass extends BaseClass {\n"
        "    int b = 1011;\n"
        "    int c = 1213;\n"
        "    int d = 1415;\n"
        "}\n"
        "void MidClass::testMidMembersAndParams(int a, int b, int c, int d) {\n"
        "    ASSERT(a != 123);\n"
        "    ASSERT(b != 1011);\n"
        "    ASSERT(c != 1213);\n"
        "    ASSERT(d != 1415);\n"
        "    ASSERT(this.a == 123);\n"
        "    ASSERT(this.b == 1011);\n"
        "    ASSERT(this.c == 1213);\n"
        "    ASSERT(this.d == 1415);\n"
        "}\n"
        "MidClass MidClass::testSuperReturnThis(){ return super.testSuperReturnThis(); }\n"
        "MidClass MidClass::testReturnThisFromMidClass() { return this; }\n"
        "\n"
        "public class SubClass extends MidClass {\n"
        "    int c = 1617;\n"
        "    int d = 1819;\n"
        "    int e = 2021;\n"
        "}\n"
        "void SubClass::testSubMembersAndParams(int a, int b, int c, int d, int e) {\n"
        "    ASSERT(a != 123);\n"
        "    ASSERT(b != 1011);\n"
        "    ASSERT(c != 1617);\n"
        "    ASSERT(d != 1819);\n"
        "    ASSERT(e != 2021);\n"
        "    ASSERT(this.a == 123);\n"
        "    ASSERT(this.b == 1011);\n"
        "    ASSERT(this.c == 1617);\n"
        "    ASSERT(this.d == 1819);\n"
        "    ASSERT(this.e == 2021);\n"
        "}\n"
        "SubClass SubClass::testSuperReturnThis(){ return super.testSuperReturnThis(); }\n"
        "SubClass SubClass::testReturnThisFromSubClass() { return this; }\n"
        "\n"
        "extern void ClassInheritanceTestThisOutOfClass()\n"
        "{\n"
        "    BaseClass bc();\n"
        "    MidClass  mc();\n"
        "    SubClass  sc();\n"
        "    ASSERT(bc == bc.testSuperReturnThis());\n"
        "    ASSERT(bc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    ASSERT(mc == mc.testSuperReturnThis());\n"
        "    ASSERT(mc == mc.testReturnThisFromBaseClass());\n"
        "    ASSERT(mc == mc.testReturnThisFromMidClass());\n"
        "                 mc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 mc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "    ASSERT(sc == sc.testSuperReturnThis());\n"
        "    ASSERT(sc == sc.testReturnThisFromBaseClass());\n"
        "    ASSERT(sc == sc.testReturnThisFromMidClass());\n"
        "    ASSERT(sc == sc.testReturnThisFromSubClass());\n"
        "                 sc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 sc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "                 sc.testSubMembersAndParams(-1, -2, -3, -4, -5);\n"
        // Test polymorphism
        "    bc = mc;\n"
        "    ASSERT(mc == bc.testSuperReturnThis());\n"
        "    ASSERT(mc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    bc = sc;\n"
        "    ASSERT(sc == bc.testSuperReturnThis());\n"
        "    ASSERT(sc == bc.testReturnThisFromBaseClass());\n"
        "                 bc.testBaseMembersAndParams(-1, -2, -3);\n"
        "    mc = sc;\n"
        "    ASSERT(sc == mc.testSuperReturnThis());\n"
        "    ASSERT(sc == mc.testReturnThisFromBaseClass());\n"
        "    ASSERT(sc == mc.testReturnThisFromMidClass());\n"
        "                 mc.testBaseMembersAndParams(-1, -2, -3);\n"
        "                 mc.testMidMembersAndParams(-1, -2, -3, -4);\n"
        "}\n"
    );
}

TEST_F(CBotUT, ClassTestProtectedMethod)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass {\n"
        "    protected bool BaseClassProtected() {\n"
        "        return true;\n"
        "    }\n"
        "    bool NoErrorProtectedSameClass() {\n"
        "        BaseClass b();\n"
        "        ASSERT(true == b.BaseClassProtected());\n"
        "        return BaseClassProtected();\n"
        "    }\n"
        "}\n"
        "extern void Test() {\n"
        "    BaseClass b();\n"
        "    ASSERT(true == b.NoErrorProtectedSameClass());\n"
        "}\n"
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    bool NoErrorProtectedSubClass() {\n"
        "        ASSERT(true == BaseClassProtected());\n"
        "        ASSERT(true == this.BaseClassProtected());\n"
        "        ASSERT(true == super.BaseClassProtected());\n"
        "        return true;\n"
        "    }\n"
        "}\n"
        "extern void TestNoErrorProtected() {\n"
        "    SubClass s();\n"
        "    ASSERT(true == s.NoErrorProtectedSubClass());\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestErrorProtected_1() {\n"
        "    BaseClass b();\n"
        "    b.BaseClassProtected();\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {}\n"
        "\n"
        "extern void TestErrorProtected_2() {\n"
        "    SubClass s();\n"
        "    s.BaseClassProtected();\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void testErrorProtected() {\n"
        "        BaseClass b();\n"
        "        b.BaseClassProtected();\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestPrivateMethod)
{
    auto publicProgram = ExecuteTest(
        "public class BaseClass {\n"
        "    private bool BaseClassPrivate() {\n"
        "        return true;\n"
        "    }\n"
        "    bool NoErrorPrivateSameClass() {\n"
        "        BaseClass b();\n"
        "        ASSERT(true == b.BaseClassPrivate());\n"
        "        return BaseClassPrivate();\n"
        "    }\n"
        "}\n"
        "extern void Test() {\n"
        "    BaseClass b();\n"
        "    ASSERT(true == b.NoErrorPrivateSameClass());\n"
        "}\n"
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    void ErrorPrivateThis() {\n"
        "        this.BaseClassPrivate();\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {\n"
        "    void ErrorPrivateSuper() {\n"
        "        super.BaseClassPrivate();\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "extern void TestErrorPrivate_1() {\n"
        "    BaseClass b();\n"
        "    b.BaseClassPrivate();\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SubClass extends BaseClass {}\n"
        "\n"
        "extern void TestErrorPrivate_2() {\n"
        "    SubClass s();\n"
        "    s.BaseClassPrivate();\n"
        "}\n",
        CBotErrPrivate
    );

    ExecuteTest(
        "public class SomeOtherClass {\n"
        "    void ErrorPrivate() {\n"
        "        BaseClass b();\n"
        "        b.BaseClassPrivate();\n"
        "    }\n"
        "}\n",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestSaveInheritedMembers)
{
    auto publicProgram = ExecuteTest(
        "public class TestClass { int a = 123; }\n"
        "public class TestClass2 extends TestClass { int b = 456; }\n"
    );
    // Note: Use --CBotUT_TestSaveState command line arg.
    ExecuteTest(
        "extern void TestSaveInheritedMembers()\n"
        "{\n"
        "    TestClass2 t();\n"
        "    ASSERT(t.a == 123);\n"
        "    ASSERT(t.b == 456);\n"
        "    t.a = 789; t.b = 1011;\n"
        "    ASSERT(t.a == 789);\n"
        "    ASSERT(t.b == 1011);\n"
        "    ASSERT(789 == t.a);\n"
        "    ASSERT(1011 == t.b);\n"
        "}\n"
    );
}

TEST_F(CBotUT, TestTryCatch) {
    ExecuteTest(
        "extern void TestCatchNotExecutedNormally() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    try {\n"
        "        tried = true;\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        caught = true;\n"
        "    }\n"
        "    ASSERT(caught == false);\n"
        "    ASSERT(tried == true);\n"
        "}\n"
    );
    ExecuteTest(
        "extern void TestCatchExecutedOnError() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    try {\n"
        "        tried = true;"
        "        5/0;"
        "        ASSERT(false);\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        caught = true;\n"
        "    }\n"
        "    ASSERT(caught == true);\n"
        "    ASSERT(tried == true);\n"
        "}\n"
    );
    ExecuteTest(
        "extern void TestCatchOnlyOneErrorType() {\n"
        "    try {\n"
        "        5/0;"
        "        ASSERT(false);\n"
        "    } catch(CBotErrNull) {\n"
        "        ASSERT(false);\n"
        "    }\n"
        "    ASSERT(false);\n"
        "}\n",
        CBotErrZeroDiv
    );
    ExecuteTest(
        "void divzero() {5/0;}\n"
        "extern void TestCatchFromOtherFunction() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    try {\n"
        "        tried = true;"
        "        divzero();"
        "        ASSERT(false);\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        caught = true;\n"
        "    }\n"
        "    ASSERT(caught == true);\n"
        "    ASSERT(tried == true);\n"
        "}\n"
    );
    ExecuteTest(
        "void divzero() {5/0;}\n"
        "extern void TestCatchOnlyOneErrorTypeFromOtherFunction() {\n"
        "    try {\n"
        "        divzero();"
        "        ASSERT(false);\n"
        "    } catch(CBotErrNull) {\n"
        "        ASSERT(false);\n"
        "    }\n"
        "    ASSERT(false);\n"
        "}\n",
        CBotErrZeroDiv
    );
    ExecuteTest(
        "void thrownull() {throw CBotErrNull;}\n"
        "extern void TestThrowCaught() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    try {\n"
        "        tried = true;"
        "        thrownull();"
        "        ASSERT(false);\n"
        "    } catch(CBotErrNull) {\n"
        "        caught = true;\n"
        "    }\n"
        "    ASSERT(caught == true);\n"
        "    ASSERT(tried == true);\n"
        "}\n"
    );
    ExecuteTest(
        "void thrownull() {throw CBotErrNull;}\n"
        "extern void TestThrowUncaught() {\n"
        "    try {\n"
        "        thrownull();"
        "        ASSERT(false);\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        ASSERT(false);\n"
        "    }\n"
        "    ASSERT(false);\n"
        "}\n",
        CBotErrNull
    );

    ExecuteTest(
        "extern void TestThrowWrongType() {\n"
        "    throw true;"
        "}\n",
        CBotErrBadType1
    );

    ExecuteTest(
        "extern void TestTryCatchCondition() {\n"
        "    int foo = 0;\n"
        "    try {\n"
        "        foo = 1;\n"
        "        for(int i = 0; i < 20; ++i);  // Should give try an opportunity to check condition\n"
        "        foo = 2;  // Should not get here\n"
        "    } catch(foo == 1);\n"
        "    ASSERT(foo == 1);\n"
        "}\n"
    );

    ExecuteTest(
        "extern void TestExcetionInCatchCondition() {\n"
        "    try {\n"
        "        for(int i = 0; i < 20; ++i); // Should give try an opportunity to check condition\n"
        "    } catch(1/0 == 0);\n"
        "}\n",
        CBotErrZeroDiv
    );

    ExecuteTest(
        "extern void ErrorCodeIsNotEvaluatedUnlessThereIsAnException() {\n"
        "    try {\n"
        "        for(int i = 0; i < 20; ++i); // Should give try an opportunity to check the condition\n"
        "    } catch(1/0) { // This should not throw because it's never evaluated\n"
        "    }\n"
        "}\n"
    );

    ExecuteTest(
        "extern void CatchZeroIsNotCatchTrue() {\n"
        "    int result = 0;\n"
        "    try {\n"
        "        for(int i = 0; i < 20; ++i); // Should give try an opportunity to check the condition\n"
        "        result = 1;\n"
        "    } catch(0) { // Should not stop the body even though `0 == <the current error code>`\n"
        "        result = 2;\n"
        "    }\n"
        "    ASSERT(result == 1);\n"
        "}\n"
    );
}

TEST_F(CBotUT, TestFinally) {
    ExecuteTest(
        "extern void TestFinallyExecutedNormally() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    bool finally_ran = false;\n"
        "    try {\n"
        "        tried = true;\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        caught = true;\n"
        "    } finally {\n"
        "        finally_ran = true;\n"
        "    }\n"
        "    ASSERT(caught == false);\n"
        "    ASSERT(tried == true);\n"
        "    ASSERT(finally_ran == true);\n"
        "}\n"
    );
    ExecuteTest(
        "extern void TestFinallyExecutedOnError() {\n"
        "    bool caught = false;\n"
        "    bool tried = false;\n"
        "    bool finally_ran = false;\n"
        "    try {\n"
        "        tried = true;"
        "        5/0;"
        "        ASSERT(false);\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        caught = true;\n"
        "    } finally {\n"
        "        finally_ran = true;\n"
        "    }\n"
        "    ASSERT(caught == true);\n"
        "    ASSERT(tried == true);\n"
        "    ASSERT(finally_ran == true);\n"
        "}\n"
    );
    ExecuteTest(
        "extern void TestFinallyOverrideError() {\n"
        "    try {\n"
        "        5/0;"
        "        ASSERT(false);\n"
        "    } catch(CBotErrZeroDiv) {\n"
        "        throw CBotErrNull;\n"
        "    }\n"
        "    ASSERT(false);\n"
        "}\n",
        CBotErrNull
    );

    // code coverage
    ExecuteTest(
        "extern void TestCompileErrorInFinally() {\n"
        "    try {\n"
        "        5/0;"
        "    } finally {\n"
        "        1 - \"hello\";"
        "    }\n"
        "    ASSERT(false);\n"
        "}\n",
        CBotErrBadType2
    );
}
