#include "common/profile.h"
#include "common/logger.h"

#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>


class CProfileTest : public testing::Test
{
protected:
    CLogger m_logger;
    CProfile m_profile;

};

TEST_F(CProfileTest, ReadTest)
{
    ASSERT_TRUE(m_profile.InitCurrentDirectory()); // load colobot.ini file

    std::string result;
    ASSERT_TRUE(m_profile.GetLocalProfileString("test_string", "string_value", result));
    ASSERT_STREQ("Hello world", result.c_str());

    int int_value;
    ASSERT_TRUE(m_profile.GetLocalProfileInt("test_int", "int_value", int_value));
    ASSERT_EQ(42, int_value);

    float float_value;
    ASSERT_TRUE(m_profile.GetLocalProfileFloat("test_float", "float_value", float_value));
    ASSERT_FLOAT_EQ(1.5, float_value);

    std::vector<std::string> list;
    list = m_profile.GetLocalProfileSection("test_multi", "entry");
    ASSERT_EQ(5u, list.size());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
