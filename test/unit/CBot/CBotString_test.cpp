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

// Modules inlcude
#include "CBot/CBotDll.h"

// Local include

// Global include
#include <gtest/gtest.h>

/**
 * @brief CBotString_Test.Empty Check Empty and IsEmpty functions of class
 *                              CBotString_Test.
 */
TEST(CBotString_Test, Empty)
{
    CBotString str1;
    EXPECT_EQ(true, str1.IsEmpty());

    CBotString str2("qsexrdctfvygbuh");
    EXPECT_EQ(false, str2.IsEmpty());
    str2.Empty();
    EXPECT_EQ(true, str2.IsEmpty());
}

/**
 * @brief CBotString_Test.Length Check GetLength function of class
 *                               CBotString_Test.
 */
TEST(CBotString_Test, Length)
{
    CBotString str1;
    EXPECT_EQ(0, str1.GetLength());

    CBotString str2("a");
    EXPECT_EQ(1, str2.GetLength());

    CBotString str3("aaaaa");
    EXPECT_EQ(5, str3.GetLength());
}

/**
 * @brief CBotString_Test.Find Check find functions of class CBotString_Test
 */
TEST(CBotString_Test, Find)
{
    CBotString str1("a");
    EXPECT_EQ(0, str1.Find('a'));

    CBotString str2("b");
    EXPECT_EQ(-1, str2.Find('a'));

    CBotString str3("bbabb");
    EXPECT_EQ(2, str3.Find('a'));

    CBotString str4("bbAbb");
    EXPECT_EQ(-1, str4.Find('a'));

    CBotString str5("bbAbb");
    EXPECT_EQ(2, str5.Find('A'));

    CBotString str6("test");
    EXPECT_EQ(0, str6.Find("test"));

    CBotString str7("azazdazd");
    EXPECT_EQ(-1, str7.Find("test"));

    CBotString str8("bbtestbb");
    EXPECT_EQ(2, str8.Find("test"));

    CBotString str9("bbTestbb");
    EXPECT_EQ(-1, str9.Find("test"));

    CBotString str10("bbTestbb");
    EXPECT_EQ(2, str10.Find("Test"));
}

/**
 * @brief CBotString_Test.ReverseFind Check find functions of class
 *                                    CBotString_Test
 */
TEST(CBotString_Test, ReverseFind)
{
    CBotString str1("a");
    EXPECT_EQ(0, str1.ReverseFind('a'));

    CBotString str2("b");
    EXPECT_EQ(-1, str2.ReverseFind('a'));

    CBotString str3("bbabb");
    EXPECT_EQ(2, str3.ReverseFind('a'));

    CBotString str4("bbAbb");
    EXPECT_EQ(-1, str4.ReverseFind('a'));

    CBotString str5("bbAbb");
    EXPECT_EQ(2, str5.ReverseFind('A'));

    CBotString str6("test");
    EXPECT_EQ(0, str6.ReverseFind("test"));

    CBotString str7("azazdazd");
    EXPECT_EQ(-1, str7.ReverseFind("test"));

    CBotString str8("bbtestbb");
    EXPECT_EQ(2, str8.ReverseFind("test"));

    CBotString str9("bbTestbb");
    EXPECT_EQ(-1, str9.ReverseFind("test"));

    CBotString str10("bbTestbb");
    EXPECT_EQ(2, str10.ReverseFind("Test"));
}

/**
 * @brief CBotString_Test.Mid Check Mid functions of class CBotString_Test
 */
TEST(CBotString_Test, Mid)
{
    CBotString str1("aaaColobot");
    EXPECT_STREQ(str1.Mid(3).CStr(), "Colobot");

    CBotString str2("aaaColobotaa");
    EXPECT_STREQ(str2.Mid(3,7).CStr(), "Colobot");
}

/**
 * @brief CBotString_Test.Left Check Left function of class CBotString_Test
 */
TEST(CBotString_Test, Left)
{
    CBotString str1("Colobotaaa");
    EXPECT_STREQ(str1.Left(7).CStr(), "Colobot");
}

/**
 * @brief CBotString_Test.Right Check Right function of class CBotString_Test
 */
TEST(CBotString_Test, Right)
{
    CBotString str1("aaaColobot");
    EXPECT_STREQ(str1.Right(7).CStr(), "Colobot");
}

/**
 * @brief CBotString_Test.Compare Check Compare function of class
 *                                CBotString_Test
 */
TEST(CBotString_Test, Compare)
{
    CBotString str1("colobot");
    EXPECT_EQ(0, str1.Compare("colobot"));

    CBotString str2("a");
    EXPECT_GT(0, str2.Compare("colobot"));

    CBotString str3("z");
    EXPECT_LT(0, str3.Compare("colobot"));
}

/**
 * @brief CBotString_Test.MakeUpper Check MakeUpper function of class
 *                                  CBotString_Test
 */
TEST(CBotString_Test, MakeUpper)
{
    CBotString str1("colobot");
    str1.MakeUpper();
    EXPECT_STREQ(str1.CStr(), "COLOBOT");

    CBotString str2("Colobot");
    str2.MakeUpper();
    EXPECT_STREQ(str2.CStr(), "COLOBOT");
}

/**
 * @brief CBotString_Test.MakeLower Check MakeLower function of class
 *                                  CBotString_Test
 */
TEST(CBotString_Test, MakeLower)
{
    CBotString str1("COLOBOT");
    str1.MakeLower();
    EXPECT_STREQ(str1.CStr(), "colobot");

    CBotString str2("Colobot");
    str2.MakeLower();
    EXPECT_STREQ(str2.CStr(), "colobot");
}

/**
 * @brief CBotString_Test.operatorEqual Check operator equal of class
 *                                      CBotString_Test
 */
TEST(CBotString_Test, operatorEqual)
{
    //-- CBotString
    CBotString botStr1;
    CBotString botStr2("Colobot");

    //-- C Character
    const char cStr1[2] = "C";

    //-- C string
    const char cStr2[8] = "Colobot";

    //-- Compare with CBotString
    botStr1 = botStr2;
    EXPECT_STREQ(botStr1.CStr(), botStr2.CStr());

    //-- Compare with C character
    botStr1 = cStr1[0];
    EXPECT_STREQ(botStr1.CStr(), cStr1);

    //-- Compare with C string
    botStr1 = cStr2;
    EXPECT_STREQ(botStr1.CStr(), cStr2);

}

/**
 * @brief CBotString_Test.operatorIsEqual Check operator is equal of class
 *                                        CBotString_Test
 */
TEST(CBotString_Test, operatorIsEqual)
{
    //-- CBotString
    CBotString botStr1("C");
    CBotString botStr2("C");
    CBotString botStr3("Colobot");

    //-- C string
    const char cStr1[8] = "Colobot";
    const char cStr2[8] = "colobot";

    EXPECT_EQ(true,  (botStr1 == botStr2));
    EXPECT_EQ(false, (botStr1 == botStr3));

    //-- Compare with C string
    EXPECT_EQ(true,  (botStr3 == cStr1));
    EXPECT_EQ(false, (botStr3 == cStr2));
}

/**
 * @brief CBotString_Test.operatorAdd Check operator add of class
 *                                    CBotString_Test.
 */
TEST(CBotString_Test, operatorAdd)
{
    //-- CBotString
    CBotString botStr1;
    CBotString botStr2("C");
    CBotString botStr3("olobot");
    CBotString botStr4("Colobot");

    //-- C string
    const char* cStr1 = "olobot";

    botStr1 = botStr1 + botStr2;
    botStr1 = botStr1 + botStr3;
    EXPECT_STREQ(botStr1.CStr(), botStr4.CStr());

    botStr1.Empty();
    botStr1 = botStr2 + cStr1;
    EXPECT_STREQ(botStr1.CStr(), botStr4.CStr());

    botStr1.Empty();
    botStr1 += botStr2;
    botStr1 += botStr3;
    EXPECT_STREQ(botStr1.CStr(), botStr4.CStr());


    botStr1.Empty();
    botStr1 += botStr2;
    botStr1 += cStr1;
    EXPECT_STREQ(botStr1.CStr(), botStr4.CStr());
}

/**
 * @brief CBotString_Test.operatorDiff Check operator diff of class
 *                                     CBotString_Test.
 */
TEST(CBotString_Test, operatorDiff)
{
    //-- CBotString
    CBotString botStr1("C");
    CBotString botStr2("C");
    CBotString botStr3("Colobot");

    //-- C string
    const char cStr1[8] = "Colobot";
    const char cStr2[8] = "colobot";

    EXPECT_EQ(false, (botStr1 != botStr2));
    EXPECT_EQ(true,  (botStr1 != botStr3));

    //-- Compare with C string
    EXPECT_EQ(false, (botStr3 != cStr1));
    EXPECT_EQ(true,  (botStr3 != cStr2));
}

/**
 * @brief CBotString_Test.operatorSuperior Check operator superior of class
 *                                         CBotString_Test.
 */
TEST(CBotString_Test, operatorSuperior)
{
    //-- CBotString
    CBotString botStr1("a");
    CBotString botStr2("z");
    CBotString botStr3("colobot");

    //-- C string
    const char cStr1[2] = "a";
    const char cStr2[2] = "z";

    EXPECT_EQ(true,  (botStr3 > botStr1));
    EXPECT_EQ(false, (botStr3 > botStr2));

    //-- Compare with C string
    EXPECT_EQ(true,  (botStr3 > cStr1));
    EXPECT_EQ(false, (botStr3 > cStr2));
}

/**
 * @brief CBotString_Test.operatorSuperiorOrEqual Check operator superior or
 *                                                equa of class CBotString_Test.
 */
TEST(CBotString_Test, operatorSuperiorOrEqual)
{
    //-- CBotString
    CBotString botStr1("a");
    CBotString botStr2("z");
    CBotString botStr3("colobot");

    //-- C string
    const char cStr1[2] = "a";
    const char cStr2[2] = "z";
    const char cStr3[8] = "colobot";

    EXPECT_EQ(true,  (botStr3 >= botStr1));
    EXPECT_EQ(false, (botStr3 >= botStr2));
    EXPECT_EQ(true,  (botStr3 >= botStr3));

    //-- Compare with C string
    EXPECT_EQ(true,  (botStr3 >= cStr1));
    EXPECT_EQ(false, (botStr3 >= cStr2));
    EXPECT_EQ(true,  (botStr3 >= cStr3));
}

/**
 * @brief CBotString_Test.operatorInferior Check operator Inferior of class
 *                                         CBotString_Test.
 */
TEST(CBotString_Test, operatorInferior)
{
    //-- CBotString
    CBotString botStr1("a");
    CBotString botStr2("z");
    CBotString botStr3("colobot");

    //-- C string
    const char cStr1[2] = "a";
    const char cStr2[2] = "z";

    EXPECT_EQ(false, (botStr3 < botStr1));
    EXPECT_EQ(true,  (botStr3 < botStr2));

    //-- Compare with C string
    EXPECT_EQ(false, (botStr3 < cStr1));
    EXPECT_EQ(true,  (botStr3 < cStr2));
}

/**
 * @brief CBotString_Test.operatorInferiorOrEqual Check operator Inferior or
 *                                                equa of class CBotString_Test.
 */
TEST(CBotString_Test, operatorInferiorOrEqual)
{
    //-- CBotString
    CBotString botStr1("a");
    CBotString botStr2("z");
    CBotString botStr3("colobot");

    //-- C string
    const char cStr1[2] = "a";
    const char cStr2[2] = "z";
    const char cStr3[8] = "colobot";

    EXPECT_EQ(false, (botStr3 <= botStr1));
    EXPECT_EQ(true,  (botStr3 <= botStr2));
    EXPECT_EQ(true,  (botStr3 <= botStr3));

    //-- Compare with C string
    EXPECT_EQ(false, (botStr3 <= cStr1));
    EXPECT_EQ(true,  (botStr3 <= cStr2));
    EXPECT_EQ(true,  (botStr3 <= cStr3));
}
