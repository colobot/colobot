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

        auto program = m_namespace.AddProgram(nullptr);
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
    CBotNamespace m_namespace;
};

TEST_F(CBotUT, EmptyTest)
{
    ExecuteTest(R"(
        extern void EmptyTest()
        {
        }
    )");
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

    ExecuteTest(R"(
        extern void ParamTrailingComma(int i, ) {
        }
        )",
        CBotErrNoType
    );

   ExecuteTest(
        "extern void MissingOpenBlock(int i)",
        CBotErrOpenBlock
    );

    ExecuteTest(R"(
        extern void MissingCloseBlock()
        {
        )",
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

    ExecuteTest(R"(
        public class TestClass
        {
        )",
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
    ExecuteTest(R"(
        extern void DivideByZero()
        {
            float a = 5/0;
        }
        )",
        CBotErrZeroDiv
    );
}

TEST_F(CBotUT, MissingSemicolon)
{
    ExecuteTest(R"(
        extern void MissingSemicolon()
        {
            string a = "hello"
        }
        )",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, UndefinedFunction)
{
    ExecuteTest(R"(
        extern void UndefinedFunction()
        {
            foo();
        }
        )",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, BasicOperations)
{
    ExecuteTest(R"(
        extern void Comparations()
        {
            ASSERT(1 != 0);
            ASSERT(1 == 1);
            ASSERT(1 > 0);
            ASSERT(1 >= 0);
            ASSERT(1 >= 1);
            ASSERT(0 < 1);
            ASSERT(0 <= 1);
            ASSERT(1 <= 1);
        }
        
        extern void BasicMath()
        {
            ASSERT(2+2 == 4);
            ASSERT(4-2 == 2);
            ASSERT(2*2 == 4);
            ASSERT(2/2 == 1);
            ASSERT(5%2 == 1);
            ASSERT(5**3 == 125);
        }
        
        extern void BitwiseMath()
        {
            ASSERT((1 << 5) == 32);
            ASSERT((32 >> 5) == 1);
            ASSERT((3 & 2) == 2);
            ASSERT((1 & 2) == 0);
            ASSERT((1 | 2) == 3);
            ASSERT((2 | 2) == 2);
            ASSERT((5 ^ 3) == 6);
            ASSERT((~1024) == -1025);
        }
        
        extern void BooleanLogic()
        {
            ASSERT(true);
            ASSERT(!false);
            ASSERT(true && true);
            ASSERT(!(true && false));
            ASSERT(!(false && true));
            ASSERT(!(false && false));
            ASSERT(true || true);
            ASSERT(true || false);
            ASSERT(false || true);
            ASSERT(!(false || false));
            ASSERT(!(true ^ true));
            ASSERT(true ^ false);
            ASSERT(false ^ true);
            ASSERT(!(false ^ false));
        }
        
        extern void NumberFormats()
        {
            ASSERT(2.0 == 2);
            ASSERT(2.00000 == 2);
            ASSERT(2.50000 == 2.5);
            ASSERT(-2.0 == -2);
            ASSERT(2e3 == 2000);
            ASSERT(-2e3 == -2000);
            ASSERT(2e-3 == 0.002);
            ASSERT(-2e-3 == -0.002);
            ASSERT(0xFF == 255);
            ASSERT(0xAB == 171);
        }
    )");
}

TEST_F(CBotUT, VarBasic)
{
    ExecuteTest(R"(
        extern void VarBasic()
        {
            int a = 5;
            ASSERT(a == 5);
            int b = 3;
            ASSERT(b == 3);
            b = a;
            ASSERT(b == 5);
            ASSERT(b == a);
            b = a + b;
            ASSERT(b == 10);
            ASSERT(b == 2*a);
        }
    )");
}

TEST_F(CBotUT, VarDefinitions)
{
    ExecuteTest(R"(
        extern void TestUndefined()
        {
            ASSERT(a == 0);
        }
        )",
        CBotErrUndefVar
    );
    ExecuteTest(R"(
        extern void TestRedefined()
        {
            int a = 5;
            int a = 3;
        }
        )",
        CBotErrRedefVar
    );
}

// TODO: I don't actually know what the exact rules should be, but it looks a bit wrong
TEST_F(CBotUT, VarImplicitCast)
{
    ExecuteTest(R"(
        extern void ImplicitCast()
        {
            int a = 5;
        //"    ASSERT(a == "5");
            string b = a;
            ASSERT(b == "5");
        //"    ASSERT(b == a);
            
            string c = "2.5";
        //"    ASSERT(c == 2.5);
        //"    float d = c;
        //"    ASSERT(d == c);
        //"    ASSERT(d == 2.5);
        }
        
        extern void AssignImplicitCast()
        {
            string a = 2;
            ASSERT(a == "2");
            a = 3;
            ASSERT(a == "3");
            string b = 2.5;
            ASSERT(b == "2.5");
            b = 3.5;
            ASSERT(b == "3.5");
        }
    )");

    ExecuteTest(R"(
        string func()
        {
            return 5;
        }
        extern void ImplicitCastOnReturn()
        {
            string a = func();
            ASSERT(a == "5");
        }
    )");
}

TEST_F(CBotUT, IntegerMathNearLimits_Issue993)
{
    ExecuteTest(R"(
        extern void Test_Issue993() {
            ASSERT(-2147483600 *  1 == -2147483600);
            ASSERT( 2147483600 *  1 ==  2147483600);
            ASSERT( 2147483646 *  1 ==  2147483646);
            ASSERT( 2147483646 * -1 == -2147483646);
            ASSERT( 2147483000 * -1 == -2147483000);
            ASSERT( 2147483000 *  1 ==  2147483000);
        }
    )");
}

TEST_F(CBotUT, BinaryLiterals)
{
    ExecuteTest(R"(
        extern void TestBinaryLiterals() {
            ASSERT(  8 ==   0b00001000);
            ASSERT( 12 ==   0b00001100);
            ASSERT( 16 ==   0b00010000);
            ASSERT( 24 ==   0b00011000);
            ASSERT( 32 ==   0b00100000);
            ASSERT( 48 ==   0b00110000);
            ASSERT( 64 ==   0b01000000);
            ASSERT( 96 ==   0b01100000);
            ASSERT(128 ==   0b10000000);
            ASSERT(192 ==   0b11000000);
            ASSERT(256 ==  0b100000000);
            ASSERT(384 ==  0b110000000);
            ASSERT(512 == 0b1000000000);
        }
    )");
}

TEST_F(CBotUT, TestSwitchCase)
{
    ExecuteTest(R"(
        extern void Test_Switch_Case() {
            int n = 0, c = 0;
            for (int i = -9; i < 11; ++i) {
                switch (i) {
                    case -9: n = -9; ++c; break;
                    case -8: n = -8; ++c; break;
                    case -7: n = -7; ++c; break;
                    case -6: n = -6; ++c; break;
                    case -5: n = -5; ++c; break;
                    case -4: n = -4; ++c; break;
                    case -3: n = -3; ++c; break;
                    case -2: n = -2; ++c; break;
                    case -1: n = -1; ++c; break;
                    case 0: n = 0; ++c; break;
                    case 1: n = 1; ++c; break;
                    case 2: n = 2; ++c; break;
                    case 3: n = 3; ++c; break;
                    case 4: n = 4; ++c; break;
                    case 5: n = 5; ++c; break;
                    case 6: n = 6; ++c; break;
                    case 7: n = 7; ++c; break;
                    case 8: n = 8; ++c; break;
                    case 9: n = 9; ++c; break;
                    default: n = 10; ++c; break;
                }
                ASSERT(n == i);
            }
            ASSERT(n == 10);
            ASSERT(c == 20);
        }
        extern void Test_Case_With_Math() {
            int n = 0, c = 0;
            for (int i = -9; i < 11; ++i) {
                switch (i * 10) {
                    case -9*10: n = -90; ++c; break;
                    case -8*10: n = -80; ++c; break;
                    case -7*10: n = -70; ++c; break;
                    case -6*10: n = -60; ++c; break;
                    case -5*10: n = -50; ++c; break;
                    case -4*10: n = -40; ++c; break;
                    case -3*10: n = -30; ++c; break;
                    case -2*10: n = -20; ++c; break;
                    case -1*10: n = -10; ++c; break;
                    case 0*10: n = 0; ++c; break;
                    case 1*10: n = 10; ++c; break;
                    case 2*10: n = 20; ++c; break;
                    case 3*10: n = 30; ++c; break;
                    case 4*10: n = 40; ++c; break;
                    case 5*10: n = 50; ++c; break;
                    case 6*10: n = 60; ++c; break;
                    case 7*10: n = 70; ++c; break;
                    case 8*10: n = 80; ++c; break;
                    case 9*10: n = 90; ++c; break;
                    default: n = 100; ++c; break;
                }
                ASSERT(n == i * 10);
            }
            ASSERT(n == 100);
            ASSERT(c == 20);
        }
    )");

    ExecuteTest(R"(
        extern void Duplicate_Case() {
            switch(0) {
                case 1000:
                case 10*100:
            }
        }
        )",
        CBotErrRedefCase
    );

    ExecuteTest(R"(
        extern void Duplicate_Default() {
            switch(0) {
                default:
                default:
            }
        }
        )",
        CBotErrRedefCase
    );
}

TEST_F(CBotUT, TestRepeatInstruction)
{
    ExecuteTest(R"(
        extern void TestRepeat() {
            int c = 0;
            for (int i = 1; i < 11; ++i)
            {
                repeat (i) ++c;
            }
            ASSERT(c == 55);
        }
        extern void TestRepeatBreakAndContinue() {
            int c = 0;
            repeat (10)
            {
                if (++c == 5) break;
                continue;
                FAIL();
            }
            ASSERT(c == 5);
            label:repeat (10)
            {
                if (++c == 10) break label;
                continue label;
                FAIL();
            }
            ASSERT(c == 10);
        }
        extern void NoRepeatNumberLessThanOne() {
            repeat (0) FAIL();
            repeat (-1) FAIL();
            repeat (-2) FAIL();
        }
        extern void EvaluateExpressionOnlyOnce() {
            int c = 0;
            repeat (c + 5) ASSERT(++c < 6);
            ASSERT(c == 5);
        }
    )");
    ExecuteTest(R"(
        extern void MissingOpenParen() {
            repeat ;
        }
        )",
        CBotErrOpenPar
    );
    ExecuteTest(R"(
        extern void MissingNumber() {
            repeat (;
        }
        )",
        CBotErrBadNum
    );
    ExecuteTest(R"(
        extern void WrongType() {
            repeat ("not number");
        }
        )",
        CBotErrBadType1
    );
    ExecuteTest(R"(
        extern void MissingCloseParen() {
            repeat (2;
        }
        )",
        CBotErrClosePar
    );
}

TEST_F(CBotUT, ToString)
{
    ExecuteTest(R"(
        extern void ArrayToString()
        {
            int[] array = {2, 4, 6};
            string arrayStr = ""+array;
            ASSERT(arrayStr == "{ 2, 4, 6 }");
        }
    )");

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

    ExecuteTest(R"(
        extern void LimitedArrayTest()
        {
            int a[5];
            ASSERT(sizeof(a) == 0);
            a[0] = 1;
            ASSERT(sizeof(a) == 1);
            a[4] = 1;
            ASSERT(sizeof(a) == 5);
            a[5] = 1;
        }
        )",
        CBotErrOutArray
    );

    ExecuteTest(R"(
        extern void BadArrayDeclarationTest()
        {
            int[5] a;
        }
        )",
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
    ExecuteTest(R"(
        public class TestClass {
            public int i = 0;
        }
        extern void ArraysInClasses()
        {
            TestClass test[];
            test[0] = new TestClass();
            test[0].i = 5;
            ASSERT(test[0].i == 5);
            
            TestClass[] test2;
        }
    )");
}

TEST_F(CBotUT, Functions)
{
    ExecuteTest(R"(
        bool notThisOne()
        {
            return false;
        }
        bool testFunction()
        {
            return true;
        }
        
        extern void Functions()
        {
            ASSERT(testFunction());
        }
    )");
}


TEST_F(CBotUT, FunctionRecursion)
{
    ExecuteTest(R"(
        int fact(int x)
        {
            if(x == 0) return 1;
            return fact(x-1) * x;
        }
        
        extern void FunctionRecursion()
        {
            ASSERT(fact(10) == 3628800);
        }
    )");
}

TEST_F(CBotUT, FunctionRecursionStackOverflow)
{
    ExecuteTest(R"(
        extern void StackOverflow()
        {
            StackOverflow();
        }
        )",
        CBotErrStackOver
    );
}

TEST_F(CBotUT, FunctionOverloading)
{
    ExecuteTest(R"(
        int func(string test)
        {
            return 1;
        }
        int func(int test)
        {
            return 2;
        }
        
        extern void FunctionOverloading()
        {
            ASSERT(func("5") == 1);
            ASSERT(func(5) == 2);
        }
    )");
}

TEST_F(CBotUT, FunctionRedefined)
{
    ExecuteTest(R"(
        int func(int test)
        {
            return 1;
        }
        int func(int test)
        {
            return 2;
        }
        )",
        CBotErrRedefFunc
    );

    ExecuteTest(R"(
        int func(int[] test)
        {
            return 1;
        }
        int func(int[] test)
        {
            return 2;
        }
        )",
        CBotErrRedefFunc
    );
}

TEST_F(CBotUT, FunctionBadReturn)
{
    ExecuteTest(R"(
        int func()
        {
            return "test";
        }
        extern void FunctionBadReturn()
        {
            int a = func();
        }
        )",
        CBotErrBadType1
    );
}

TEST_F(CBotUT, FunctionNoReturn)
{
    ExecuteTest(R"(
        int func()
        {
        }
        extern void FunctionNoReturn()
        {
            func();
        }
        )",
        CBotErrNoReturn
    );

    ExecuteTest(R"(
        int FuncDoesNotReturnAValue()
        {
            if (false) return 1;
            while (false) return 1;
            if (true) ; else return 1;
            do { break; return 1; } while (false);
            do { continue; return 1; } while (false);
        }
        )",
        CBotErrNoReturn
    );

    ExecuteTest(R"(
        int FuncHasReturn()
        {
            return 1;
        }
        int BlockHasReturn()
        {
            {
                {
                }
                return 2;
            }
        }
        int IfElseHasReturn()
        {
            if (false) {
                return 3;
            } else {
                if (false) return 3;
                else return 3;
            }
        }
        extern void Test()
        {
            ASSERT(1 == FuncHasReturn());
            ASSERT(2 == BlockHasReturn());
            ASSERT(3 == IfElseHasReturn());
        }
    )");
}

TEST_F(CBotUT, PublicFunctions)
{
    // Keep the program, so that the function continues to exist after ExecuteTest finishes
    auto publicProgram = ExecuteTest(R"(
        public int test()
        {
            return 1337;
        }
    )");

    ExecuteTest(R"(
        extern void TestPublic()
        {
            ASSERT(test() == 1337);
        }
    )");

    publicProgram.reset(); // Now remove

    ExecuteTest(R"(
        extern void TestPublicRemoved()
        {
            ASSERT(test() == 1337);
        }
        )",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassConstructor)
{
    ExecuteTest(R"(
        public class TestClass {
            public static int instanceCounter = 0;
            public void TestClass() {
                instanceCounter++;
            }
        }
        
        extern void ClassConstructor()
        {
            TestClass t1();
            ASSERT(t1.instanceCounter == 1);
            ASSERT(t1 != null);
            TestClass t2; // not calling the constructor!
            ASSERT(t1.instanceCounter == 1);
        //"    ASSERT(t2 == null);\n" // TODO: I was pretty sure that's how it worked, but apparently not...
            TestClass t3 = new TestClass();
            ASSERT(t1.instanceCounter == 2);
            ASSERT(t3.instanceCounter == 2);
            ASSERT(t3 != null);
            ASSERT(t3 != t1);
        }
    )");
}

TEST_F(CBotUT, ClassDestructor)
{
    ExecuteTest(R"(
        public class TestClass {
            public static int instanceCounter = 0;
            public void TestClass() {
                instanceCounter++;
            }
            public void ~TestClass() {
                instanceCounter--;
            }
        }
        
        extern void ClassDestructor()
        {
            TestClass t1();
            ASSERT(t1.instanceCounter == 1);
            {
                TestClass t2();
                ASSERT(t2.instanceCounter == 2);
                ASSERT(t1.instanceCounter == 2);
            }
            ASSERT(t1.instanceCounter == 1);
        }
    )");
}

TEST_F(CBotUT, ClassBadNew)
{
    ExecuteTest(R"(
        public class AClass {};
        extern void ClassBadNew()
        {
            AClass a = new "abc";
        }
        )",
        CBotErrBadNew
    );
}

TEST_F(CBotUT, ClassCallOnNull)
{
    ExecuteTest(R"(
        public class AClass {
            public void test() {}
        };
        extern void ClassCallOnNull()
        {
            AClass a = null;
            a.test();
        }
        )",
        CBotErrNull
    );
}

TEST_F(CBotUT, ClassNullPointer)
{
    ExecuteTest(R"(
        public class TestClass {
            public void TestClass() {
                FAIL();
            }
        }
        extern void TestClassNullPointer()
        {
            TestClass t;
        //"    ASSERT(t == null);\n" // TODO: OH REALLY?
            TestClass t2 = null;
            ASSERT(t2 == null);
        }
    )");
    ExecuteTest(R"(
        public class TestClass {
            public int x = 0;
            public void TestClass() {
                FAIL();
            }
        }
        extern void TestClassNullPointerAccess()
        {
            TestClass t;
            int y = t.x;
        }
        )",
        CBotErrNull
    );
}

TEST_F(CBotUT, ClassDestructorNaming)
{
    ExecuteTest(R"(
        public class TestClass {
            public void ~SomethingElse() {}
        }
        )",
        CBotErrNoFunc
    );
    ExecuteTest(R"(
        public class SomethingElse {
        }
        public class TestClass2 {
            public void ~SomethingElse() {}
        }
        )",
        CBotErrNoFunc
    );
}

TEST_F(CBotUT, ClassDestructorSyntax)
{
    ExecuteTest(R"(
        public class TestClass {
            void ~TestClass(int i) {}
        }
        extern void DestructorNoParams() {}
        )",
        CBotErrClosePar
    );

    ExecuteTest(R"(
        public class TestClass {
            int ~TestClass() {}
        }
        extern void DestructorReturnTypeVoid() {}
        )",
        CBotErrFuncNotVoid
    );
}

TEST_F(CBotUT, ClassMethodOverloading)
{
    ExecuteTest(R"(
        public class TestClass {
            public int test(string test) {
                return 1;
            }
            public int test(int test) {
                return 2;
            }
        }
        extern void ClassMethodOverloading() {
            TestClass t();
            ASSERT(t.test("5") == 1);
            ASSERT(t.test(5) == 2);
        }
    )");
}

TEST_F(CBotUT, ClassMethodRedefined)
{
    ExecuteTest(R"(
        public class TestClass {
            public int test(string test) {
                return 1;
            }
            public int test(string test) {
                return 2;
            }
        }
        )",
        CBotErrRedefFunc
    );

    ExecuteTest(R"(
        public class TestClass {
            public int test(int[] test) {
                return 1;
            }
            public int test(int[] test) {
                return 2;
            }
        }
        )",
        CBotErrRedefFunc
    );

    ExecuteTest(R"(
        public class TestClass {
            void ~TestClass() {}
            void ~TestClass() {}
        }
        )",
        CBotErrRedefFunc
    );
}

TEST_F(CBotUT, ClassFieldNaming)
{
    ExecuteTest(R"(
        public class TestClass {
            int ~i = 1;
        }
        )",
        CBotErrNoVar
    );

    ExecuteTest(R"(
        public class TestClass {
            int TestClass = 1;
        }
        )",
        CBotErrNoVar
    );

    ExecuteTest(R"(
        public class TestClass {
            int i = 1;
            int i = 2;
        }
        )",
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
    ExecuteTest(R"(
        public class TestClass {}
        public class TestClass {}
        )",
        CBotErrRedefClass
    );
}

TEST_F(CBotUT, ClassMissingCloseBlock)
{
    ExecuteTest(R"(
        public class Something
        {
        )",
        CBotErrCloseBlock
    );
}

// TODO: NOOOOOO!!! Nononononono :/
TEST_F(CBotUT, DISABLED_PublicClasses)
{
    // Keep the program, so that the class continues to exist after ExecuteTest finishes
    auto publicProgram = ExecuteTest(R"(
        public class TestClass
        {
        }
    )");

    ExecuteTest(R"(
        extern void TestPublic()
        {
            TestClass t();
        }
    )");

    publicProgram.reset(); // Now remove

    ExecuteTest(R"(
        extern void TestPublicRemoved()
        {
            TestClass t();
        }
        )",
        CBotErrUndefClass
    );
}

TEST_F(CBotUT, ThisEarlyContextSwitch_Issue436)
{
    ExecuteTest(R"(
        public class Something {
            public int a = 7;
            void test2(int i, int expected) {
                ASSERT(i == expected);
            }
        }
        public class TestClass {
            public int i = 5;
            public void test(Something s) {
                s.test2(this.i, 5);
            }
        }
        extern void WeirdClassThisAsParamThing()
        {
            Something s();
            TestClass t();
            t.test(s);
        }
    )");
}

TEST_F(CBotUT, ClassStringAdd_Issue535)
{
    ExecuteTest(R"(
        public class TestClass {}
        extern void ClassStringAdd()
        {
            TestClass t();
            string s = t + "!";
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceAssignment)
{
    ExecuteTest(R"(
        public class BaseClass {}
        public class MidClass extends BaseClass {}
        public class SubClass extends MidClass {}
        extern void ClassInheritanceAssignment()
        {
            BaseClass bc = new MidClass();
            MidClass  mc = bc;
            mc = new SubClass();
            SubClass  sc = mc;
            bc = mc;
            bc = new MidClass();
            bc = new SubClass();
            sc = bc;
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceVars)
{
    ExecuteTest(R"(
        public class BaseClass {
            int a = 123;
            int b = 456;
            int c = 789;
        }
        public class MidClass extends BaseClass {
            int b = 1011;
            int c = 1213;
            int d = 1415;
        }
        public class SubClass extends MidClass {
            int c = 1617;
            int d = 1819;
            int e = 2021;
        }
        extern void ClassInheritanceVars()
        {
            BaseClass bc();
            ASSERT(bc.a == 123);
            ASSERT(bc.b == 456);
            ASSERT(bc.c == 789);
            MidClass  mc();
            ASSERT(mc.a == 123);
            ASSERT(mc.b == 1011);
            ASSERT(mc.c == 1213);
            ASSERT(mc.d == 1415);
            SubClass  sc();
            ASSERT(sc.a == 123);
            ASSERT(sc.b == 1011);
            ASSERT(sc.c == 1617);
            ASSERT(sc.d == 1819);
            ASSERT(sc.e == 2021);
        // Test polymorphism
            bc = mc;
            ASSERT(bc.a == 123);
            ASSERT(bc.b == 456);
            ASSERT(bc.c == 789);
            mc = sc;
            ASSERT(mc.a == 123);
            ASSERT(mc.b == 1011);
            ASSERT(mc.c == 1213);
            ASSERT(mc.d == 1415);
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceMethods)
{
    ExecuteTest(R"(
        public class BaseClass {
            int a = 123;
            int b = 456;
            int c = 789;
            int testOverride() { return 123; }
            int testNoOverride() { return 456; }
            int testInsideBaseClass() {
                ASSERT(a == 123);
                ASSERT(b == 456);
                ASSERT(c == 789);
                ASSERT(456 == testNoOverride());
                return c;
            }
            int testInsideBaseOverride() { return testOverride(); }
        }
        public class MidClass extends BaseClass {
            int b = 1011;
            int c = 1213;
            int d = 1415;
            int testOverride() { return 1011; }
            int testInsideMidClass() {
                ASSERT(a == 123);
                ASSERT(b == 1011);
                ASSERT(c == 1213);
                ASSERT(d == 1415);
                ASSERT(456  == testNoOverride());
                ASSERT(789  == testInsideBaseClass());
                return c;
            }
            int testSuper() {
                ASSERT(super.a == 123);
                ASSERT(super.b == 456);
                ASSERT(super.c == 789);
                ASSERT(123 == super.testOverride());
                ASSERT(789 == super.testInsideBaseClass());
                return super.testInsideBaseOverride();
            }
            int testInsideMidOverride() { return testOverride(); }
        }
        public class SubClass extends MidClass {
            int c = 1617;
            int d = 1819;
            int e = 2021;
            int testOverride() { return 1617; }
            int testInsideSubClass() {
                ASSERT(a == 123);
                ASSERT(b == 1011);
                ASSERT(c == 1617);
                ASSERT(d == 1819);
                ASSERT(e == 2021);
                ASSERT(456  == testNoOverride());
                ASSERT(789  == testInsideBaseClass());
                ASSERT(1213 == testInsideMidClass());
                return c;
            }
            int testSuper() {
                ASSERT(super.a == 123);
                ASSERT(super.b == 1011);
                ASSERT(super.c == 1213);
                ASSERT(super.d == 1415);
                ASSERT(1011 == super.testOverride());
                ASSERT(789  == super.testInsideBaseClass());
                ASSERT(1213 == super.testInsideMidClass());
                return super.testSuper();
            }
            int testInsideSubOverride() { return testOverride(); }
        }
        extern void InheritanceMethods()
        {
            BaseClass bc();
            ASSERT(123 == bc.testOverride());
            ASSERT(456 == bc.testNoOverride());
            ASSERT(789 == bc.testInsideBaseClass());
            ASSERT(123 == bc.testInsideBaseOverride());
            MidClass  mc();
            ASSERT(1011 == mc.testSuper());
            ASSERT(1011 == mc.testOverride());
            ASSERT(456  == mc.testNoOverride());
            ASSERT(789  == mc.testInsideBaseClass());
            ASSERT(1213 == mc.testInsideMidClass());
            ASSERT(1011 == mc.testInsideBaseOverride());
            ASSERT(1011 == mc.testInsideMidOverride());
            SubClass  sc();
            ASSERT(1617 == sc.testSuper());
            ASSERT(1617 == sc.testOverride());
            ASSERT(456  == sc.testNoOverride());
            ASSERT(789  == sc.testInsideBaseClass());
            ASSERT(1213 == sc.testInsideMidClass());
            ASSERT(1617 == sc.testInsideSubClass());
            ASSERT(1617 == sc.testInsideBaseOverride());
            ASSERT(1617 == sc.testInsideMidOverride());
            ASSERT(1617 == sc.testInsideSubOverride());
        // Test polymorphism
            bc = mc;
            ASSERT(1011 == bc.testOverride());
            ASSERT(789  == bc.testInsideBaseClass());
            ASSERT(1011 == bc.testInsideBaseOverride());
            bc = sc;
            ASSERT(1617 == bc.testOverride());
            ASSERT(789  == bc.testInsideBaseClass());
            ASSERT(1617 == bc.testInsideBaseOverride());
            mc = sc;
            ASSERT(1617 == mc.testSuper());
            ASSERT(1617 == mc.testOverride());
            ASSERT(789  == mc.testInsideBaseClass());
            ASSERT(1213 == mc.testInsideMidClass());
            ASSERT(1617 == mc.testInsideBaseOverride());
            ASSERT(1617 == mc.testInsideMidOverride());
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceTestThis)
{
    ExecuteTest(R"(
        public class BaseClass {
            int a = 123;
            int b = 456;
            int c = 789;
            void testBaseMembersAndParams(int a, int b, int c) {
                ASSERT(a != 123);
                ASSERT(b != 456);
                ASSERT(c != 789);
                ASSERT(this.a == 123);
                ASSERT(this.b == 456);
                ASSERT(this.c == 789);
            }
            BaseClass testSuperReturnThis(){ return this; }
            BaseClass testReturnThisFromBaseClass() { return this; }
        }
        public class MidClass extends BaseClass {
            int b = 1011;
            int c = 1213;
            int d = 1415;
            void testMidMembersAndParams(int a, int b, int c, int d) {
                ASSERT(a != 123);
                ASSERT(b != 1011);
                ASSERT(c != 1213);
                ASSERT(d != 1415);
                ASSERT(this.a == 123);
                ASSERT(this.b == 1011);
                ASSERT(this.c == 1213);
                ASSERT(this.d == 1415);
            }
            MidClass testSuperReturnThis(){ return super.testSuperReturnThis(); }
            MidClass testReturnThisFromMidClass() { return this; }
        }
        public class SubClass extends MidClass {
            int c = 1617;
            int d = 1819;
            int e = 2021;
            void testSubMembersAndParams(int a, int b, int c, int d, int e) {
                ASSERT(a != 123);
                ASSERT(b != 1011);
                ASSERT(c != 1617);
                ASSERT(d != 1819);
                ASSERT(e != 2021);
                ASSERT(this.a == 123);
                ASSERT(this.b == 1011);
                ASSERT(this.c == 1617);
                ASSERT(this.d == 1819);
                ASSERT(this.e == 2021);
            }
            SubClass testSuperReturnThis(){ return super.testSuperReturnThis(); }
            SubClass testReturnThisFromSubClass() { return this; }
        }
        extern void ClassInheritanceTestThis()
        {
            BaseClass bc();
            MidClass  mc();
            SubClass  sc();
            ASSERT(bc == bc.testSuperReturnThis());
            ASSERT(bc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            ASSERT(mc == mc.testSuperReturnThis());
            ASSERT(mc == mc.testReturnThisFromBaseClass());
            ASSERT(mc == mc.testReturnThisFromMidClass());
                         mc.testBaseMembersAndParams(-1, -2, -3);
                         mc.testMidMembersAndParams(-1, -2, -3, -4);
            ASSERT(sc == sc.testSuperReturnThis());
            ASSERT(sc == sc.testReturnThisFromBaseClass());
            ASSERT(sc == sc.testReturnThisFromMidClass());
            ASSERT(sc == sc.testReturnThisFromSubClass());
                         sc.testBaseMembersAndParams(-1, -2, -3);
                         sc.testMidMembersAndParams(-1, -2, -3, -4);
                         sc.testSubMembersAndParams(-1, -2, -3, -4, -5);
        // Test polymorphism
            bc = mc;
            ASSERT(mc == bc.testSuperReturnThis());
            ASSERT(mc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            bc = sc;
            ASSERT(sc == bc.testSuperReturnThis());
            ASSERT(sc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            mc = sc;
            ASSERT(sc == mc.testSuperReturnThis());
            ASSERT(sc == mc.testReturnThisFromBaseClass());
            ASSERT(sc == mc.testReturnThisFromMidClass());
                         mc.testBaseMembersAndParams(-1, -2, -3);
                         mc.testMidMembersAndParams(-1, -2, -3, -4);
        }
    )");
}

TEST_F(CBotUT, ClassCompileCircularReference_Issue433)
{
    ExecuteTest(R"(
        public class OtherClass {
            TestClass testclass;
        }
        public class TestClass {
            int test;
            OtherClass otherclass;
        }
        extern void TestCompileCircularReference()
        {
            TestClass t();
        }
    )");
}

TEST_F(CBotUT, ClassTestClassDefinedAfterReference)
{
    ExecuteTest(R"(
        public class OtherClass {
            TestClass testclass = new TestClass();
        }
        public class TestClass {
            int test = 246;
        }
        extern void TestDefinedAfterReference()
        {
            OtherClass o();
            TestClass t = o.testclass;
            ASSERT(t.test == 246);
        }
    )");
}

TEST_F(CBotUT, String)
{
    ExecuteTest(R"(
        extern void StringTest()
        {
            string a = "Colo";
            string b = "bot";
            string c = a + b + "!";
            ASSERT(a == "Colo");
            ASSERT(b == "bot");
            ASSERT(c == "Colobot!");
        }
    )");

    ExecuteTest(R"(
        extern void MissingEndQuote()
        {
            "Colobot...
        }
        )",
        CBotErrEndQuote
    );
}

TEST_F(CBotUT, StringEscapeCodes)
{
    ExecuteTest(R"(
        extern void HexEscapeCodes()
        {
            ASSERT("  \x07  " == "  \a  ");
            ASSERT("  \x08  " == "  \b  ");
            ASSERT("  \x09  " == "  \t  ");
            ASSERT("  \x0A  " == "  \n  ");
            ASSERT("  \x0B  " == "  \v  ");
            ASSERT("  \x0C  " == "  \f  ");
            ASSERT("  \x0D  " == "  \r  ");
            ASSERT("  \x22  " == "  \"  ");
            ASSERT("  \x27  " == "  \'  ");
            ASSERT("  \x5C  " == "  \\  ");
            string test = "\x31 \x32 \x33";
            ASSERT(test == "1 2 3");
        }
        extern void OctalEscapeCodes()
        {
            ASSERT("  \000  " == "  \x00  ");
            ASSERT("  \007  " == "  \x07  ");
            ASSERT("  \010  " == "  \x08  ");
            ASSERT("  \011  " == "  \x09  ");
            ASSERT("  \012  " == "  \x0A  ");
            ASSERT("  \013  " == "  \x0B  ");
            ASSERT("  \014  " == "  \x0C  ");
            ASSERT("  \015  " == "  \x0D  ");
            ASSERT("  \042  " == "  \x22  ");
            ASSERT("  \047  " == "  \x27  ");
            ASSERT("  \134  " == "  \x5C  ");
            string test = "\101 \102 \103";
            ASSERT(test == "A B C");
        }
        extern void UnicodeEscapeCodesToUTF_8()
        {
            ASSERT("  \u0000  " == "  \0  ");
            ASSERT("  \u0007  " == "  \a  ");
            ASSERT("  \u0008  " == "  \b  ");
            ASSERT("  \u0009  " == "  \t  ");
            ASSERT("  \u000A  " == "  \n  ");
            ASSERT("  \u000B  " == "  \v  ");
            ASSERT("  \u000C  " == "  \f  ");
            ASSERT("  \u000D  " == "  \r  ");
            ASSERT("  \u0022  " == "  \"  ");
            ASSERT("  \u0027  " == "  \'  ");
            ASSERT("  \u005C  " == "  \\  ");
        
            ASSERT("\u00A9" == "\xC2\xA9");
            ASSERT("\u00AE" == "\xC2\xAE");
            ASSERT("\u262E" == "\xE2\x98\xAE");
            ASSERT("\u262F" == "\xE2\x98\xAF");
            ASSERT("\U0001F60E" == "\xF0\x9F\x98\x8E");
            ASSERT("\U0001F61C" == "\xF0\x9F\x98\x9C");
            ASSERT("\U0001F6E0" == "\xF0\x9F\x9B\xA0");
        }
        extern void UnicodeMaxCharacterNameToUTF_8()
        {
            ASSERT("\U0010FFFF" == "\xF4\x8F\xBF\xBF");
        }
    )");
}

TEST_F(CBotUT, StringEscapeCodeErrors)
{
    ExecuteTest(R"(
        extern void UnknownEscapeSequence()
        {
            "Unknown: \p ";
        }
        )",
        CBotErrBadEscape
    );

    ExecuteTest(R"(
        extern void MissingHexDigits()
        {
            "  \x  ";
        }
        )",
        CBotErrHexDigits
    );

    ExecuteTest(R"(
        extern void HexValueOutOfRange()
        {
            "  \x100  ";
        }
        )",
        CBotErrHexRange
    );

    ExecuteTest(R"(
        extern void OctalValueOutOfRange()
        {
            "  \400  ";
        }
        )",
        CBotErrOctalRange
    );

    ExecuteTest(R"(
        extern void BadUnicodeCharacterName()
        {
            "  \U00110000  ";
        }
        )",
        CBotErrUnicodeName
    );
}

// TODO: not implemented, see issue #694
TEST_F(CBotUT, DISABLED_StringAsArray)
{
    ExecuteTest(R"(
        extern void StringAsArray()
        {
            string s = "Colobot";
            ASSERT(s[0] == "C");
            ASSERT(s[3] == "o");
            s[2] = "L"; s[4] = "B"; s[6] = "T";
            ASSERT(s == "CoLoBoT");
        }
    )");
}

TEST_F(CBotUT, ArraysOfStrings)
{
    ExecuteTest(R"(
        extern void ArraysOfStrings()
        {
            string[] a;
            string b[];
            string c[5];
            string d[] = {"test"};
            d[1] = "test2";
            ASSERT(d[0] == "test");
            ASSERT(d[1] == "test2");
        }
    )");
}

TEST_F(CBotUT, StringFunctions)
{
    ExecuteTest(R"(
        extern void StringFunctions()
        {
            string s = "Colobot";
            ASSERT(strlen(s) == 7);
            ASSERT(strlower(s) == "colobot");
            ASSERT(strupper(s) == "COLOBOT");
            ASSERT(strleft(s, 3) == "Col");
            ASSERT(strright(s, 3) == "bot");
            ASSERT(strmid(s, 1, 3) == "olo");
            ASSERT(strfind(s, "o") == 1);
            ASSERT(strval("2.5") == 2.5);
        }
        extern void StringFunctionsOutOfRange()
        {
            ASSERT(strmid("asdf", 5, 1) == "");
            ASSERT(strmid("asdf", 0, 100) == "asdf");
            ASSERT(strmid("asdf", -500, 100) == "asdf");
            ASSERT(strleft("asdf", 15) == "asdf");
            ASSERT(strleft("asdf", -15) == "");
            ASSERT(strright("asdf", 15) == "asdf");
            ASSERT(strright("asdf", -15) == "");
        }
    )");
}

TEST_F(CBotUT, LiteralCharacters)
{
    ExecuteTest(R"(
        extern void TestCharValue()
        {
            ASSERT('A' == 65);
            ASSERT('B' == 66);
            ASSERT('C' == 67);
            ASSERT('\a' == 0x07);
            ASSERT('\b' == 0x08);
            ASSERT('\t' == 0x09);
            ASSERT('\n' == 0x0A);
            ASSERT('\v' == 0x0B);
            ASSERT('\f' == 0x0C);
            ASSERT('\r' == 0x0D);
            ASSERT('\"' == 0x22);
            ASSERT('\'' == 0x27);
            ASSERT('\\' == 0x5C);
        }
        extern void TestCharUnicodeEscape()
        {
            ASSERT('\u0007' == '\a');
            ASSERT('\u0008' == '\b');
            ASSERT('\u0009' == '\t');
            ASSERT('\u000A' == '\n');
            ASSERT('\u000B' == '\v');
            ASSERT('\u000C' == '\f');
            ASSERT('\u000D' == '\r');
            ASSERT('\u0022' == '\"');
            ASSERT('\u0027' == '\'');
            ASSERT('\u005C' == '\\');
        }
        extern void AssignCharToString_ToUTF_8()
        {
            string test = '\u00A9';
            test += '\u00AE';
            ASSERT(test == "\xC2\xA9\xC2\xAE");
        }
        extern void AddCharToString_ToUTF_8()
        {
            ASSERT("" + 'A' + 'B' + 'C' == "ABC");
            ASSERT("" + '\u00A9' == "\xC2\xA9");
            ASSERT("" + '\u00AE' == "\xC2\xAE");
            ASSERT("" + '\u262E' == "\xE2\x98\xAE");
            ASSERT("" + '\u262F' == "\xE2\x98\xAF");
            ASSERT("" + '\U0001F60E' == "\xF0\x9F\x98\x8E");
            ASSERT("" + '\U0001F61C' == "\xF0\x9F\x98\x9C");
            ASSERT("" + '\U0001F6E0' == "\xF0\x9F\x9B\xA0");
            ASSERT("" + '\U0010FFFF' == "\xF4\x8F\xBF\xBF");
        }
    )");

    ExecuteTest(R"(
        extern void MissingEndQuote()
        {
            '
        }
        )",
        CBotErrEndQuote
    );

    ExecuteTest(R"(
        extern void MissingEndQuote()
        {
            'a
        }
        )",
        CBotErrEndQuote
    );

    ExecuteTest(R"(
        extern void EmptyQuotes()
        {
            '';
        }
        )",
        CBotErrCharEmpty
    );

    ExecuteTest(R"(
        extern void UnknownEscapeSequence()
        {
            '\p';
        }
        )",
        CBotErrBadEscape
    );

    ExecuteTest(R"(
        extern void MissingHexDigits()
        {
            '\u';
        }
        )",
        CBotErrHexDigits
    );

    ExecuteTest(R"(
        extern void BadUnicodeCharacterName()
        {
            '\U00110000';
        }
        )",
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
    ExecuteTest(R"(
        extern void TestArrayInitialization() {
            int[] a = {1, 2, 3};
            ASSERT(sizeof(a) == 3);
            ASSERT(a[0] == 1);
            ASSERT(a[1] == 2);
            ASSERT(a[2] == 3);
        }
    )");

    ExecuteTest(R"(
        extern void TestArrayInitializationOutOfRange() {
            int a[2] = {1, 2, 3};
        }
        )",
        CBotErrOutArray
    );

    ExecuteTest(R"(
        extern void TestArrayInitializationSmallerThanRange() {
            int a[4] = {1, 2, 3};
            ASSERT(sizeof(a) == 3);
            ASSERT(a[0] == 1);
            ASSERT(a[1] == 2);
            ASSERT(a[2] == 3);
            a[3] = 4;
            ASSERT(sizeof(a) == 4);
            ASSERT(a[3] == 4);
        }
    )");

    ExecuteTest(R"(
        extern void TestArrayInitializationLimitUnchanged() {
            int a[4] = {1, 2, 3};
            a[4] = 5;
        }
        )",
        CBotErrOutArray
    );

    ExecuteTest(R"(
        extern void TestArrayInitializationWithVars() {
            int x=1, y=2, z=3;
            int i[] = { x, y, z };
            ASSERT(i[0] == 1);
            ASSERT(i[1] == 2);
            ASSERT(i[2] == 3);
            i[0] += 1;
            ASSERT(i[0] == 2);
            ASSERT(x == 1);
        }
    )");
}

TEST_F(CBotUT, TestArrayFunctionReturn)
{
    ExecuteTest(R"(
        int[] test() {
            int[] a = {1, 2, 3};
            return a;
        }
        extern void TestArrayFunctionReturn() {
            int[] b = test();
            ASSERT(sizeof(b) == 3);
            ASSERT(b[0] == 1);
            ASSERT(b[1] == 2);
            ASSERT(b[2] == 3);
        }
    )");
}

TEST_F(CBotUT, AccessMembersInParameters_Issue256)
{
    ExecuteTest(R"(
        public class Test1 {
            int x = 1337;
        }
        public class Test2 {
            public bool test(int a) {
                return a == 1337;
            }
        }
        public class Test3 {
            public Test1 test1 = new Test1();
            public Test2 test2 = new Test2();
            public void test() {
                ASSERT(test2.test(test1.x));
            }
        }
        extern void AccessMembersInParameters() {
            Test3 t();
            t.test();
        }
    )");
}

TEST_F(CBotUT, InstrCallAccessMemberVoid)
{
    ExecuteTest(R"(
        void Test() {}
        extern void TestAccessMemberVoid() {
            Test().x;
        }
        )",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, InstrCallAccessMemberNonObject)
{
    ExecuteTest(R"(
        int GetInt() {
            return 1;
        }
        extern void TestAccessMemberNonObject() {
            GetInt().x;
        }
        )",
        CBotErrNoTerminator
    );
}

TEST_F(CBotUT, InstrCallAccessMemberObjectNull)
{
    ExecuteTest(R"(
        public class TestClass { int x = 1; }
        TestClass GetObjectNull() {
            TestClass t = null;
            return t;
        }
        extern void TestAccessMemberObjectNull() {
            GetObjectNull().x;
        }
        )",
        CBotErrNull
    );
}

TEST_F(CBotUT, InstrCallAccessMemberReturnNull)
{
    ExecuteTest(R"(
        public class TestClass { int x = 1; }
        TestClass GetReturnNull() {
            return null;
        }
        extern void TestAccessMemberReturnNull() {
            GetReturnNull().x;
        }
        )",
        CBotErrNull
    );
}

TEST_F(CBotUT, InstrCallAccessMemberNotVar)
{
    ExecuteTest(R"(
        public class TestClass {}
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberNotVar() {
            TestClass tc();
            GetObject(tc).123;
        }
        )",
        CBotErrUndefClass
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarNonMember)
{
    ExecuteTest(R"(
        public class TestClass { int x = 1; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarNonMember() {
            TestClass tc();
            GetObject(tc).y;
        }
        )",
        CBotErrUndefItem
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarUndefined)
{
    ExecuteTest(R"(
        public class TestClass { int x; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarUndefined() {
            TestClass tc();
            GetObject(tc).x;
        }
        )",
        CBotErrNotInit
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarPrivate)
{
    ExecuteTest(R"(
        public class TestClass { private int x = 123; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarPrivate() {
            TestClass tc();
            ASSERT(123 == GetObject(tc).x);
        }
        )",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVar)
{
    ExecuteTest(R"(
        public class TestClass { int x = 123; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVar() {
            TestClass tc();
            ASSERT(123 == GetObject(tc).x);
        }
    )");
}

TEST_F(CBotUT, InstrCallAccessMemberVarArrayBadIndex)
{
    ExecuteTest(R"(
        public class TestClass { int[] a; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarArrayEmpty() {
            TestClass tc();
            int i = GetObject(tc).a[4.7];
        }
        )",
        CBotErrBadIndex
    );
}
TEST_F(CBotUT, InstrCallAccessMemberVarArrayCloseIndex)
{
    ExecuteTest(R"(
        public class TestClass { int[] a = {123}; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarArrayEmpty() {
            TestClass tc();
            int i = GetObject(tc).a[0;
        }
        )",
        CBotErrCloseIndex
    );
}
TEST_F(CBotUT, InstrCallAccessMemberVarArrayEmpty)
{
    ExecuteTest(R"(
        public class TestClass { int[] a; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarArrayEmpty() {
            TestClass tc();
            int i = GetObject(tc).a[0];
        }
        )",
        CBotErrOutArray
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarArrayOutOfRange)
{
    ExecuteTest(R"(
        public class TestClass { int a[] = {123}; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarArrayOut() {
            TestClass tc();
            int i = GetObject(tc).a[1];
        }
        )",
        CBotErrOutArray
    );
}

TEST_F(CBotUT, InstrCallAccessMemberVarArray)
{
    ExecuteTest(R"(
        public class TestClass { int a[] = {123}; }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberVarArray() {
            TestClass tc();
            ASSERT(123 == GetObject(tc).a[0]);
        }
    )");
}

TEST_F(CBotUT, InstrCallAccessMemberMethod)
{
    ExecuteTest(R"(
        public class TestClass {
            int x = 123;
            int testGetX() { return x; }
        }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberMethod() {
            TestClass tc();
            ASSERT(123 == GetObject(tc).testGetX());
        }
    )");
}

TEST_F(CBotUT, InstrCallAccessMemberMethodChain)
{
    ExecuteTest(R"(
        public class TestClass {
            int x = 123;
            TestClass testGetThis() { return this; }
            int testGetX() { return x; }
        }
        TestClass GetObject(TestClass t) {
            return t;
        }
        extern void TestAccessMemberMethodChain() {
            TestClass tc();
            ASSERT(123 == GetObject(tc).testGetThis().testGetX());
        }
    )");
}

TEST_F(CBotUT, InstrCallAccessMemberNewObjectDestructor)
{
    ExecuteTest(R"(
        public class TestClass {
            int x = 123;
            static bool b = false;
            void ~TestClass() { b = true; }
        }
        TestClass GetNewObject() { return new TestClass(); }
        extern void TestAccessMemberNewObject() {
            TestClass tc();
            ASSERT(123 == GetNewObject().x);
            ASSERT(tc.b == true);
        }
    )");
}

TEST_F(CBotUT, ClassConstructorMethodChain)
{
    ExecuteTest(R"(
        public class TestClass {
            int a = 123;
            int b = 246;
            TestClass testSetA(int x) { a = x; return this; }
            TestClass testSetB(int y) { b = y; return this; }
        }
        extern void ConstructorMethodChain() {
            TestClass tc().testSetA(111).testSetB(222);
            ASSERT(tc.a == 111);
            ASSERT(tc.b == 222);
        }
    )");
}

TEST_F(CBotUT, ClassNewConstructorMethodChain)
{
    ExecuteTest(R"(
        public class TestClass {
            int a = 123;
            int b = 246;
            TestClass testSetA(int x) { a = x; return this; }
            TestClass testSetB(int y) { b = y; return this; }
        }
        extern void NewConstructorMethodChain() {
            TestClass tc;
            tc = new TestClass().testSetA(111).testSetB(222);
            ASSERT(tc.a == 111);
            ASSERT(tc.b == 222);
        }
    )");
}

TEST_F(CBotUT, PassNullAsArgument)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass {}
        public class SubClass extends BaseClass {}
    )");

    ExecuteTest(R"(
        bool Test(BaseClass b) {
            return (b == null);
        }
        extern void PassNullAsArgument() {
            ASSERT(true == Test(null));
        }
    )");

    ExecuteTest(R"(
        void Test(BaseClass b) {}
        void Test(SubClass s) {}
        
        extern void AmbiguousCallArgumentNull() {
            Test(null);
        }
        )",
        CBotErrAmbiguousCall
    );
}

TEST_F(CBotUT, ClassImplicitCastArguments)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass { int a = 360; }
        public class SubClass extends BaseClass {}
    )");

    ExecuteTest(R"(
        bool Test(BaseClass b) {
            SubClass s = b;
            return (360 == s.a);
        }
        extern void UpcastPassingArguments() {
            ASSERT(true == Test(new SubClass()));
        }
    )");

    ExecuteTest(R"(
        void Test(BaseClass b, SubClass s) {}
        void Test(SubClass s, BaseClass b) {}
        
        extern void UpcastAmbiguousCall() {
            Test(new SubClass(), new SubClass());
        }
        )",
        CBotErrAmbiguousCall
    );

    ExecuteTest(R"(
        bool Test(BaseClass b, SubClass s) { return false; }
        bool Test(SubClass s, BaseClass b) { return false; }
        bool Test(SubClass s, SubClass s2) { return true; }
        
        extern void NoErrorMoreSpecific() {
            ASSERT(true == Test(new SubClass(), new SubClass()));
        }
    )");
}

TEST_F(CBotUT, AmbiguousCallWithNumbers)
{
    ExecuteTest(R"(
        void Test(int i, float f) {}
        void Test(float f, int i) {}
        
        extern void AmbiguousCallNumbers() {
            Test(1, 2);
        }
        )",
        CBotErrAmbiguousCall
    );

    ExecuteTest(R"(
        bool Test(int i, float f) { return false; }
        bool Test(float f, int i) { return false; }
        bool Test(int i, int ii)  { return true; }
        
        extern void NoErrorMoreSpecific() {
            ASSERT(true == Test(1, 2));
        }
    )");
}

TEST_F(CBotUT, ClassMethodWithPublicKeyword)
{
    auto publicProgram = ExecuteTest(R"(
        public class TestClass {
            public int Test() { return 1; }
        }
    )");

    ExecuteTest(R"(
        int Test() { return 2; }
        
        extern void DontCallMethodInTestClass()
        {
            ASSERT(2 == Test());
        }
    )");

    ExecuteTest(R"(
        int Test() { return 2; }
        
        public class OtherClass {}
        
        extern void OtherClass::TestCallWithThis()
        {
            this.Test();
        }
        )",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassTestProtectedMember)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass {
            protected int a_protected = 1;
            bool test() {
                a_protected = 1;
                int a = a_protected;
                return true;
            }
        }
        extern void Test() {
            BaseClass b();
            ASSERT(true == b.test());
        }
    )");

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            bool testProtected() {
                a_protected = 1;
                int a = a_protected;
                return true;
            }
        }
        extern void TestSubClassAccessProtected() {
            SubClass  s();
            ASSERT(true == s.test());
            ASSERT(true == s.testProtected());
        }
    )");

    ExecuteTest(R"(
        extern void TestErrorProtected() {
            BaseClass b();
            int i = b.a_protected;
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        extern void ErrorProtectedAssignment() {
            BaseClass b();
            b.a_protected = 1;
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void testErrorProtected() {
                BaseClass b();
                int i = b.a_protected;
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void testErrorProtectedAssignment() {
                BaseClass b();
                b.a_protected = 1;
            }
        }
        )",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestPrivateMember)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass {
            private int a_private = 2;
        
            bool test() {
                a_private = 2;
                int a = a_private;
                return true;
            }
            bool NoErrorPrivateSameClass() {
                BaseClass b = new BaseClass();
                int a = b.a_private;
                b.a_private = 2;
                return true;
            }
        }
        extern void Test() {
            BaseClass b();
            ASSERT(true == b.test());
            ASSERT(true == b.NoErrorPrivateSameClass());
        }
    )");

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            void testErrorPrivate() {
                int a = a_private;
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            void testErrorPrivateAssignment() {
                a_private = 2;
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        extern void TestErrorPrivate() {
            BaseClass b();
            int i = b.a_private;
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        extern void ErrorPrivateAssignment() {
            BaseClass b();
            b.a_private = 2;
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void testErrorPrivate() {
                BaseClass b();
                int i = b.a_private;
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void testErrorPrivateAssignment() {
                BaseClass b();
                b.a_private = 1;
            }
        }
        )",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, IncrementDecrementSyntax)
{
    auto publicProgram = ExecuteTest(R"(
        public class TestClass {
            int GetInt() { return 1; }
        }
        extern void TestIncrementDecrement()
        {
            int i = 1;
            ASSERT(2 == ++i);
            ASSERT(2 == i++);
            ASSERT(3 ==  i );
            ASSERT(2 == --i);
            ASSERT(2 == i--);
            ASSERT(1 ==  i );
        }
    )");

    ExecuteTest(R"(
        extern void PreIncrementMethodCall()
        {
            TestClass tc();
            ++tc.GetInt();
        }
        )",
        CBotErrBadType1
    );

    ExecuteTest(R"(
        extern void PostIncrementMethodCall()
        {
            TestClass tc();
            tc.GetInt()++;
        }
        )",
        CBotErrBadType1
    );

    ExecuteTest(R"(
        extern void BadPreIncrementEmpty()
        {
            ++;
        }
        )",
        CBotErrBadType1
    );

    ExecuteTest(R"(
        extern void BadPreIncrementNotAVar()
        {
            ++123;
        }
        )",
        CBotErrBadType1
    );
}

TEST_F(CBotUT, ParametersWithDefaultValues)
{
    ExecuteTest(R"(
        extern void ParametersWithDefaultValues() {
            ASSERT(true == Test());
            ASSERT(true == Test(1));
            ASSERT(true == Test(1, 2));
        }
        bool Test(int i = 1, float f = 2.0) {
            return (i == 1) && (f == 2.0);
        }
    )");

    ExecuteTest(R"(
        extern void NotUsingDefaultValues() {
            ASSERT(true == Test(2, 4.0));
        }
        bool Test(int i = 1, float f = 2.0) {
            return (i == 2) && (f == 4.0);
        }
    )");

    ExecuteTest(R"(
        extern void NextParamNeedsDefaultValue() {
        }
        void Test(int i = 1, float f) {}
        
        )",
        CBotErrDefaultValue
    );

    ExecuteTest(R"(
        extern void ParamMissingExpression() {
        }
        void Test(int i = 1, float f = ) {}
        
        )",
        CBotErrNoExpression
    );

    ExecuteTest(R"(
        extern void ParamDefaultBadType() {
        }
        void Test(int i = 1, float f = null) {}
        
        )",
        CBotErrBadType1
    );

    ExecuteTest(R"(
        extern void DefaultValuesAmbiguousCall() {
            Test();
        }
        void Test(int i = 1) {}
        void Test(float f = 2.0) {}
        
        )",
        CBotErrAmbiguousCall
    );

    ExecuteTest(R"(
        extern void AmbiguousCallOneDefault() {
            Test(1);
        }
        void Test(int i, float f = 2) {}
        void Test(int i) {}
        
        )",
        CBotErrAmbiguousCall
    );

    ExecuteTest(R"(
        extern void DifferentNumberOfDefaultValues() {
            Test(1, 2.0);
        }
        void Test(int i, float f = 2.0) {}
        void Test(int i, float f = 2.0, int ii = 1) {}
        
        )",
        CBotErrAmbiguousCall
    );

    ExecuteTest(R"(
        extern void DefaultValueUnaryExpression() {
            TestNumbers();
            TestBool();
        }
        void TestNumbers(int i = -1, float f = -1, int ii = ~1) {
            ASSERT(i == -1);
            ASSERT(f == -1.0);
            ASSERT(ii == ~1);
        }
        void TestBool(bool b = !false, bool b2 = not false) {
            ASSERT(true == b);
            ASSERT(true == b2);
        }
    )");
}

TEST_F(CBotUT, ClassMethodsOutOfClass_Issue207)
{
    auto publicProgram = ExecuteTest(R"(
        public class OtherClass {}
        public class TestClass {}
    )");

    ExecuteTest(R"(
        extern void TestCallWithoutObject()
        {
            TestMethod();
        }
        public void TestClass::TestMethod() {}
        )",
        CBotErrUndefCall
    );

    ExecuteTest(R"(
        extern void TestCallWithWrongObject()
        {
            OtherClass oc();
            oc.TestMethod();
        }
        public void TestClass::TestMethod() {}
        )",
        CBotErrUndefCall
    );

    ExecuteTest(R"(
        extern void OtherClass::TestCallWithWrongThis()
        {
            this.TestMethod();
        }
        public void TestClass::TestMethod() {}
        )",
        CBotErrUndefCall
    );
}

TEST_F(CBotUT, ClassMethodsOutOfClass)
{
    auto publicProgram = ExecuteTest(R"(
        public class TestClass
        {
            int i = 123, j = 456, k = 789;
            int InsideClass()
            {
                ASSERT(456 == PublicMethod());
                ASSERT(789 == NotPublicMethod());
                return this.i;
            }
        }
        extern void TestMethodsOutOfClass()
        {
            TestClass tc();
            ASSERT(123 == tc.InsideClass());
            ASSERT(456 == tc.PublicMethod());
            ASSERT(789 == tc.NotPublicMethod());
        }
        public int TestClass::PublicMethod()
        {
            return this.j;
        }
        int TestClass::NotPublicMethod()
        {
            return k;
        }
    )");

    ExecuteTest(R"(
        extern void TestFromOtherProgram()
        {
            TestClass tc();
            ASSERT(123 == tc.InsideClass());
            ASSERT(456 == tc.PublicMethod());
            ASSERT(789 == tc.MethodInThisProgram());
        }
        int TestClass::MethodInThisProgram()
        {
            ASSERT(123 == InsideClass());
            ASSERT(456 == PublicMethod());
            ASSERT(123 == this.InsideClass());
            ASSERT(456 == this.PublicMethod());
            ASSERT(i == 123 && this.j == 456);
            return this.k;
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceMethodsOutOfClass)
{
    ExecuteTest(R"(
        public class BaseClass {
            int a = 123;
            int b = 456;
            int c = 789;
        }
        int BaseClass::testOverride() { return 123; }
        int BaseClass::testNoOverride() { return 456; }
        int BaseClass::testInsideBaseClass() {
            ASSERT(a == 123);
            ASSERT(b == 456);
            ASSERT(c == 789);
            ASSERT(456 == testNoOverride());
            return c;
        }
        int BaseClass::testInsideBaseOverride() { return testOverride(); }
        
        public class MidClass extends BaseClass {
            int b = 1011;
            int c = 1213;
            int d = 1415;
        }
        int MidClass::testOverride() { return 1011; }
        int MidClass::testInsideMidClass() {
            ASSERT(a == 123);
            ASSERT(b == 1011);
            ASSERT(c == 1213);
            ASSERT(d == 1415);
            ASSERT(456 == testNoOverride());
            ASSERT(789 == testInsideBaseClass());
            return c;
        }
        int MidClass::testSuper() {
            ASSERT(super.a == 123);
            ASSERT(super.b == 456);
            ASSERT(super.c == 789);
            ASSERT(123 == super.testOverride());
            ASSERT(789 == super.testInsideBaseClass());
            return super.testInsideBaseOverride();
        }
        int MidClass::testInsideMidOverride() { return testOverride(); }
        
        public class SubClass extends MidClass {
            int c = 1617;
            int d = 1819;
            int e = 2021;
        }
        int SubClass::testOverride() { return 1617; }
        int SubClass::testInsideSubClass() {
            ASSERT(a == 123);
            ASSERT(b == 1011);
            ASSERT(c == 1617);
            ASSERT(d == 1819);
            ASSERT(e == 2021);
            ASSERT(456  == testNoOverride());
            ASSERT(789  == testInsideBaseClass());
            ASSERT(1213 == testInsideMidClass());
            return c;
        }
        int SubClass::testSuper() {
            ASSERT(super.a == 123);
            ASSERT(super.b == 1011);
            ASSERT(super.c == 1213);
            ASSERT(super.d == 1415);
            ASSERT(1011 == super.testOverride());
            ASSERT(789  == super.testInsideBaseClass());
            ASSERT(1213 == super.testInsideMidClass());
            return super.testSuper();
        }
        int SubClass::testInsideSubOverride() { return testOverride(); }
        
        extern void InheritanceMethodsOutOfClass()
        {
            BaseClass bc();
            ASSERT(123 == bc.testOverride());
            ASSERT(456 == bc.testNoOverride());
            ASSERT(789 == bc.testInsideBaseClass());
            ASSERT(123 == bc.testInsideBaseOverride());
            MidClass  mc();
            ASSERT(1011 == mc.testSuper());
            ASSERT(1011 == mc.testOverride());
            ASSERT(456  == mc.testNoOverride());
            ASSERT(789  == mc.testInsideBaseClass());
            ASSERT(1213 == mc.testInsideMidClass());
            ASSERT(1011 == mc.testInsideBaseOverride());
            ASSERT(1011 == mc.testInsideMidOverride());
            SubClass  sc();
            ASSERT(1617 == sc.testSuper());
            ASSERT(1617 == sc.testOverride());
            ASSERT(456  == sc.testNoOverride());
            ASSERT(789  == sc.testInsideBaseClass());
            ASSERT(1213 == sc.testInsideMidClass());
            ASSERT(1617 == sc.testInsideSubClass());
            ASSERT(1617 == sc.testInsideBaseOverride());
            ASSERT(1617 == sc.testInsideMidOverride());
            ASSERT(1617 == sc.testInsideSubOverride());
        // Test polymorphism
            bc = mc;
            ASSERT(1011 == bc.testOverride());
            ASSERT(789  == bc.testInsideBaseClass());
            ASSERT(1011 == bc.testInsideBaseOverride());
            bc = sc;
            ASSERT(1617 == bc.testOverride());
            ASSERT(789  == bc.testInsideBaseClass());
            ASSERT(1617 == bc.testInsideBaseOverride());
            mc = sc;
            ASSERT(1617 == mc.testSuper());
            ASSERT(1617 == mc.testOverride());
            ASSERT(789  == mc.testInsideBaseClass());
            ASSERT(1213 == mc.testInsideMidClass());
            ASSERT(1617 == mc.testInsideBaseOverride());
            ASSERT(1617 == mc.testInsideMidOverride());
        }
    )");
}

TEST_F(CBotUT, ClassInheritanceTestThisOutOfClass)
{
    ExecuteTest(R"(
        public class BaseClass {
            int a = 123;
            int b = 456;
            int c = 789;
        }
        void BaseClass::testBaseMembersAndParams(int a, int b, int c) {
            ASSERT(a != 123);
            ASSERT(b != 456);
            ASSERT(c != 789);
            ASSERT(this.a == 123);
            ASSERT(this.b == 456);
            ASSERT(this.c == 789);
        }
        BaseClass BaseClass::testSuperReturnThis(){ return this; }
        BaseClass BaseClass::testReturnThisFromBaseClass() { return this; }
        
        public class MidClass extends BaseClass {
            int b = 1011;
            int c = 1213;
            int d = 1415;
        }
        void MidClass::testMidMembersAndParams(int a, int b, int c, int d) {
            ASSERT(a != 123);
            ASSERT(b != 1011);
            ASSERT(c != 1213);
            ASSERT(d != 1415);
            ASSERT(this.a == 123);
            ASSERT(this.b == 1011);
            ASSERT(this.c == 1213);
            ASSERT(this.d == 1415);
        }
        MidClass MidClass::testSuperReturnThis(){ return super.testSuperReturnThis(); }
        MidClass MidClass::testReturnThisFromMidClass() { return this; }
        
        public class SubClass extends MidClass {
            int c = 1617;
            int d = 1819;
            int e = 2021;
        }
        void SubClass::testSubMembersAndParams(int a, int b, int c, int d, int e) {
            ASSERT(a != 123);
            ASSERT(b != 1011);
            ASSERT(c != 1617);
            ASSERT(d != 1819);
            ASSERT(e != 2021);
            ASSERT(this.a == 123);
            ASSERT(this.b == 1011);
            ASSERT(this.c == 1617);
            ASSERT(this.d == 1819);
            ASSERT(this.e == 2021);
        }
        SubClass SubClass::testSuperReturnThis(){ return super.testSuperReturnThis(); }
        SubClass SubClass::testReturnThisFromSubClass() { return this; }
        
        extern void ClassInheritanceTestThisOutOfClass()
        {
            BaseClass bc();
            MidClass  mc();
            SubClass  sc();
            ASSERT(bc == bc.testSuperReturnThis());
            ASSERT(bc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            ASSERT(mc == mc.testSuperReturnThis());
            ASSERT(mc == mc.testReturnThisFromBaseClass());
            ASSERT(mc == mc.testReturnThisFromMidClass());
                         mc.testBaseMembersAndParams(-1, -2, -3);
                         mc.testMidMembersAndParams(-1, -2, -3, -4);
            ASSERT(sc == sc.testSuperReturnThis());
            ASSERT(sc == sc.testReturnThisFromBaseClass());
            ASSERT(sc == sc.testReturnThisFromMidClass());
            ASSERT(sc == sc.testReturnThisFromSubClass());
                         sc.testBaseMembersAndParams(-1, -2, -3);
                         sc.testMidMembersAndParams(-1, -2, -3, -4);
                         sc.testSubMembersAndParams(-1, -2, -3, -4, -5);
        // Test polymorphism
            bc = mc;
            ASSERT(mc == bc.testSuperReturnThis());
            ASSERT(mc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            bc = sc;
            ASSERT(sc == bc.testSuperReturnThis());
            ASSERT(sc == bc.testReturnThisFromBaseClass());
                         bc.testBaseMembersAndParams(-1, -2, -3);
            mc = sc;
            ASSERT(sc == mc.testSuperReturnThis());
            ASSERT(sc == mc.testReturnThisFromBaseClass());
            ASSERT(sc == mc.testReturnThisFromMidClass());
                         mc.testBaseMembersAndParams(-1, -2, -3);
                         mc.testMidMembersAndParams(-1, -2, -3, -4);
        }
    )");
}

TEST_F(CBotUT, ClassTestProtectedMethod)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass {
            protected bool BaseClassProtected() {
                return true;
            }
            bool NoErrorProtectedSameClass() {
                BaseClass b();
                ASSERT(true == b.BaseClassProtected());
                return BaseClassProtected();
            }
        }
        extern void Test() {
            BaseClass b();
            ASSERT(true == b.NoErrorProtectedSameClass());
        }
    )");

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            bool NoErrorProtectedSubClass() {
                ASSERT(true == BaseClassProtected());
                ASSERT(true == this.BaseClassProtected());
                ASSERT(true == super.BaseClassProtected());
                return true;
            }
        }
        extern void TestNoErrorProtected() {
            SubClass s();
            ASSERT(true == s.NoErrorProtectedSubClass());
        }
    )");

    ExecuteTest(R"(
        extern void TestErrorProtected_1() {
            BaseClass b();
            b.BaseClassProtected();
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SubClass extends BaseClass {}
        
        extern void TestErrorProtected_2() {
            SubClass s();
            s.BaseClassProtected();
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void testErrorProtected() {
                BaseClass b();
                b.BaseClassProtected();
            }
        }
        )",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestPrivateMethod)
{
    auto publicProgram = ExecuteTest(R"(
        public class BaseClass {
            private bool BaseClassPrivate() {
                return true;
            }
            bool NoErrorPrivateSameClass() {
                BaseClass b();
                ASSERT(true == b.BaseClassPrivate());
                return BaseClassPrivate();
            }
        }
        extern void Test() {
            BaseClass b();
            ASSERT(true == b.NoErrorPrivateSameClass());
        }
    )");

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            void ErrorPrivateThis() {
                this.BaseClassPrivate();
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SubClass extends BaseClass {
            void ErrorPrivateSuper() {
                super.BaseClassPrivate();
            }
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        extern void TestErrorPrivate_1() {
            BaseClass b();
            b.BaseClassPrivate();
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SubClass extends BaseClass {}
        
        extern void TestErrorPrivate_2() {
            SubClass s();
            s.BaseClassPrivate();
        }
        )",
        CBotErrPrivate
    );

    ExecuteTest(R"(
        public class SomeOtherClass {
            void ErrorPrivate() {
                BaseClass b();
                b.BaseClassPrivate();
            }
        }
        )",
        CBotErrPrivate
    );
}

TEST_F(CBotUT, ClassTestSaveInheritedMembers)
{
    auto publicProgram = ExecuteTest(R"(
        public class TestClass { int a = 123; }
        public class TestClass2 extends TestClass { int b = 456; }
    )");
    // Note: Use --CBotUT_TestSaveState command line arg.
    ExecuteTest(R"(
        extern void TestSaveInheritedMembers()
        {
            TestClass2 t();
            ASSERT(t.a == 123);
            ASSERT(t.b == 456);
            t.a = 789; t.b = 1011;
            ASSERT(t.a == 789);
            ASSERT(t.b == 1011);
            ASSERT(789 == t.a);
            ASSERT(1011 == t.b);
        }
    )");
}

TEST_F(CBotUT, TestTryCatch) {
    ExecuteTest(R"(
        extern void TestCatchNotExecutedNormally() {
            bool caught = false;
            bool tried = false;
            try {
                tried = true;
            } catch(CBotErrZeroDiv) {
                caught = true;
            }
            ASSERT(caught == false);
            ASSERT(tried == true);
        }
    )");
    ExecuteTest(R"(
        extern void TestCatchExecutedOnError() {
            bool caught = false;
            bool tried = false;
            try {
                tried = true;
                5/0;
                ASSERT(false);
            } catch(CBotErrZeroDiv) {
                caught = true;
            }
            ASSERT(caught == true);
            ASSERT(tried == true);
        }
    )");
    ExecuteTest(R"(
        extern void TestCatchOnlyOneErrorType() {
            try {
                5/0;
                ASSERT(false);
            } catch(CBotErrNull) {
                ASSERT(false);
            }
            ASSERT(false);
        }
        )",
        CBotErrZeroDiv
    );
    ExecuteTest(R"(
        void divzero() {5/0;}
        extern void TestCatchFromOtherFunction() {
            bool caught = false;
            bool tried = false;
            try {
                tried = true;
                divzero();
                ASSERT(false);
            } catch(CBotErrZeroDiv) {
                caught = true;
            }
            ASSERT(caught == true);
            ASSERT(tried == true);
        }
    )");
    ExecuteTest(R"(
        void divzero() {5/0;}
        extern void TestCatchOnlyOneErrorTypeFromOtherFunction() {
            try {
                divzero();
                ASSERT(false);
            } catch(CBotErrNull) {
                ASSERT(false);
            }
            ASSERT(false);
        }
        )",
        CBotErrZeroDiv
    );
    ExecuteTest(R"(
        void thrownull() {throw CBotErrNull;}
        extern void TestThrowCaught() {
            bool caught = false;
            bool tried = false;
            try {
                tried = true;
                thrownull();
                ASSERT(false);
            } catch(CBotErrNull) {
                caught = true;
            }
            ASSERT(caught == true);
            ASSERT(tried == true);
        }
    )");
    ExecuteTest(R"(
        void thrownull() {throw CBotErrNull;}
        extern void TestThrowUncaught() {
            try {
                thrownull();
                ASSERT(false);
            } catch(CBotErrZeroDiv) {
                ASSERT(false);
            }
            ASSERT(false);
        }
        )",
        CBotErrNull
    );

    ExecuteTest(R"(
        extern void TestThrowWrongType() {
            throw true;
        }
        )",
        CBotErrBadType1
    );

    ExecuteTest(R"(
        extern void TestTryCatchCondition() {
            int foo = 0;
            try {
                foo = 1;
                for(int i = 0; i < 20; ++i);  // Should give try an opportunity to check condition
                foo = 2;  // Should not get here
            } catch(foo == 1);
            ASSERT(foo == 1);
        }
    )");

    ExecuteTest(R"(
        extern void TestExcetionInCatchCondition() {
            try {
                for(int i = 0; i < 20; ++i); // Should give try an opportunity to check condition
            } catch(1/0 == 0);
        }
        )",
        CBotErrZeroDiv
    );

    ExecuteTest(R"(
        extern void ErrorCodeIsNotEvaluatedUnlessThereIsAnException() {
            try {
                for(int i = 0; i < 20; ++i); // Should give try an opportunity to check the condition
            } catch(1/0) { // This should not throw because it's never evaluated
            }
        }
    )");

    ExecuteTest(R"(
        extern void CatchZeroIsNotCatchTrue() {
            int result = 0;
            try {
                for(int i = 0; i < 20; ++i); // Should give try an opportunity to check the condition
                result = 1;
            } catch(0) { // Should not stop the body even though `0 == <the current error code>`
                result = 2;
            }
            ASSERT(result == 1);
        }
    )");
}

TEST_F(CBotUT, TestFinally) {
    ExecuteTest(R"(
        extern void TestFinallyExecutedNormally() {
            bool caught = false;
            bool tried = false;
            bool finally_ran = false;
            try {
                tried = true;
            } catch(CBotErrZeroDiv) {
                caught = true;
            } finally {
                finally_ran = true;
            }
            ASSERT(caught == false);
            ASSERT(tried == true);
            ASSERT(finally_ran == true);
        }
    )");
    ExecuteTest(R"(
        extern void TestFinallyExecutedOnError() {
            bool caught = false;
            bool tried = false;
            bool finally_ran = false;
            try {
                tried = true;
                5/0;
                ASSERT(false);
            } catch(CBotErrZeroDiv) {
                caught = true;
            } finally {
                finally_ran = true;
            }
            ASSERT(caught == true);
            ASSERT(tried == true);
            ASSERT(finally_ran == true);
        }
    )");
    ExecuteTest(R"(
        extern void TestFinallyOverrideError() {
            try {
                5/0;
                ASSERT(false);
            } catch(CBotErrZeroDiv) {
                throw CBotErrNull;
            }
            ASSERT(false);
        }
        )",
        CBotErrNull
    );

    // code coverage
    ExecuteTest(R"(
        extern void TestCompileErrorInFinally() {
            try {
                5/0;
            } finally {
                1 - "hello";
            }
            ASSERT(false);
        }
        )",
        CBotErrBadType2
    );
}

TEST_F(CBotUT, TestByteToString) {
    ExecuteTest(R"(
        extern void TestByteToString() {
            byte c = 90;
            ASSERT(c + "" == "90");
        }
    )");
}
