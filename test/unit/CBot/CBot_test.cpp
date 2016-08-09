/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include <stdexcept>

using namespace CBot;

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

protected:
    std::unique_ptr<CBotProgram> ExecuteTest(const std::string& code, CBotError expectedError = CBotNoErr)
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
                ADD_FAILURE() << "Compile error - " << error << " (" << cursor1 << "-" << (cursor2 >= 0 ? cursor2 : cursor1) << ")" << std::endl << GetFormattedLineInfo(code, cursor1); // TODO: Error messages are on Colobot side
                return std::move(program);
            }
            else
            {
                ADD_FAILURE() << "No compile error, expected " << expectedCompileError; // TODO: Error messages are on Colobot side
                return std::move(program);
            }
        }
        if (expectedCompileError != CBotNoErr) return std::move(program);

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
        return std::move(program); // Take it if you want, destroy on exit otherwise
    }
};

TEST_F(CBotUT, EmptyTest)
{
    ExecuteTest(
        "extern void EmptyTest()\n"
        "{\n"
        "}\n"
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

// TODO: Doesn't work
TEST_F(CBotUT, DISABLED_FunctionNoReturn)
{
    ExecuteTest(
        "int func()\n"
        "{\n"
        "}\n"
        "extern void FunctionNoReturn()\n"
        "{\n"
        "    func();\n"
        "}\n",
        static_cast<CBotError>(-1) // TODO: no error for that
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

// TODO: This doesn't work
TEST_F(CBotUT, DISABLED_ClassDestructorNaming)
{
    ExecuteTest(
        "public class TestClass {\n"
        "    public void ~SomethingElse() {}\n"
        "}\n",
        static_cast<CBotError>(-1) // TODO: no error for that
    );
    ExecuteTest(
        "public class SomethingElse {\n"
        "}\n"
        "public class TestClass2 {\n"
        "    public void ~SomethingElse() {}\n"
        "}\n",
        static_cast<CBotError>(-1) // TODO: no error for that
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
}

// TODO: Not only doesn't work but segfaults
TEST_F(CBotUT, DISABLED_ClassRedefined)
{
    ExecuteTest(
        "public class TestClass {}\n"
        "public class TestClass {}\n",
        CBotErrRedefClass
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

TEST_F(CBotUT, DISABLED_TestNANParam_Issue642)
{
    ExecuteTest(
        "float test(float x) {\n"
        "     return x;\n"
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

// The this pointer should always point to the current instance, even if we are
// using the 'super' meta instance that points to the parent.
TEST_F(CBotUT, ThisPointerInsideSuperCall)
{
    ExecuteTest(
        "public class Base {\n"
        "    Base GetThis() { return this; }\n"
        "}\n"
        "public class Child extends Base {\n"
        "    Base GetThis() { return super.GetThis(); }\n"
        "}\n"
        "extern void test() {\n"
        "    Child c();\n"
        "    ASSERT(c == c.GetThis());\n"
        "}\n"
    );
}

TEST_F(CBotUT, ThisPointerInsideSuperCallGrandparent)
{
    ExecuteTest(
        "public class Base {\n"
        "    Base GetThis() { return this; }\n"
        "}\n"
        "public class Child extends Base {\n"
        "    Base GetThis() { return super.GetThis(); }\n"
        "}\n"
        "public class Grandchild extends Child {\n"
        "    Base GetThis() { return super.GetThis(); }\n"
        "}\n"
        "extern void test() {\n"
        "    Grandchild g();\n"
        "    ASSERT(g == g.GetThis());\n"
        "}"
    );
}

TEST_F(CBotUT, DynamicDispatch)
{
    ExecuteTest(
        "public class Base {\n"
        "    int foo() { return 1; }\n"
        "}\n"
        "public class Child extends Base {\n"
        "    int foo() { return 2; }\n"
        "}\n"
        "extern void test() {\n"
        "    Base b();\n"
        "    ASSERT(b.foo() == 1);\n"
        "    Child c();\n"
        "    ASSERT(c.foo() == 2);\n"
        "    b = c;\n"
        "    ASSERT(b.foo() == 2);\n"
        "}"
    );
}

TEST_F(CBotUT, WrappedSuperMethod)
{
    ExecuteTest(
        "public class Base {\n"
        "    int wrapped() { this.foo() +  1; }\n"
        "    int foo() { throw 1; }\n"
        "}\n"
        "public class Child extends Base {\n"
        "    int foo() { return 2; }\n"
        "}\n"
        "extern void test() {\n"
        "    Child c();\n"
        "    ASSERT(c.wrapped() == 3);\n"
        "}"
    );
}
