#include "common/profile.h"
#include "common/logger.h"
#include "app/system.h"

#include <iostream>
#include <string>
#include <vector>
#include <gtest/gtest.h>


class CProfileTest : public testing::Test
{
protected:
    CProfile m_profile;

};

TEST_F(CProfileTest, ReadTest)
{
    m_profile.SetUseCurrentDirectory(true);

    ASSERT_TRUE(m_profile.Init()); // load colobot.ini file

    std::string result;
    ASSERT_TRUE(m_profile.GetStringProperty("test_string", "string_value", result));
    ASSERT_STREQ("Hello world", result.c_str());

    int int_value;
    ASSERT_TRUE(m_profile.GetIntProperty("test_int", "int_value", int_value));
    ASSERT_EQ(42, int_value);

    float float_value;
    ASSERT_TRUE(m_profile.GetFloatProperty("test_float", "float_value", float_value));
    ASSERT_FLOAT_EQ(1.5, float_value);

    std::vector<std::string> list;
    list = m_profile.GetSection("test_multi", "entry");
    ASSERT_EQ(5u, list.size());
}
