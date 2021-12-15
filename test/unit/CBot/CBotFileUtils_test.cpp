#include "CBot/CBotFileUtils.h"

#include <gtest/gtest.h>

namespace
{
std::string createTestStringWithAllPossibleCharacters()
{
    std::string testString;
    for (char c = std::numeric_limits<char>::min(); ; ++c)
    {
        testString.push_back(c);
        if (c == std::numeric_limits<char>::max()) break;
    }
    return testString;
}
}

namespace CBot
{

struct CBotFileUtilsTest : public testing::Test
{
    std::stringstream stream;
};

struct CBotFileUtilsReadWriteByteTest : public CBotFileUtilsTest, public testing::WithParamInterface<char>
{
};

TEST_P(CBotFileUtilsReadWriteByteTest, ReadByteValueShouldMatchWrittenValue)
{
    char expectedValue{GetParam()};
    ASSERT_TRUE(WriteByte(stream, expectedValue));
    char value{1};
    ASSERT_TRUE(ReadByte(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteByteTest,
    testing::Values(
        '\0',
        static_cast<char>(42),
        std::numeric_limits<char>::min(),
        std::numeric_limits<char>::max()));

struct CBotFileUtilsReadWriteWordTest : public CBotFileUtilsTest, public testing::WithParamInterface<unsigned short>
{
};

TEST_P(CBotFileUtilsReadWriteWordTest, ReadWordValueShouldMatchWrittenValue)
{
    unsigned short expectedValue{GetParam()};
    ASSERT_TRUE(WriteWord(stream, expectedValue));
    unsigned short value{1};
    ASSERT_TRUE(ReadWord(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteWordTest,
    testing::Values(
        static_cast<unsigned short>(0),
        static_cast<unsigned short>(42),
        std::numeric_limits<unsigned short>::min(),
        std::numeric_limits<unsigned short>::max() / 2,
        std::numeric_limits<unsigned short>::max()));

struct CBotFileUtilsReadWriteShortTest : public CBotFileUtilsTest, public testing::WithParamInterface<short>
{
};

TEST_P(CBotFileUtilsReadWriteShortTest, ReadShortValueShouldMatchWrittenValue)
{
    short expectedValue{GetParam()};
    ASSERT_TRUE(WriteShort(stream, expectedValue));
    short value{1};
    ASSERT_TRUE(ReadShort(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteShortTest,
    testing::Values(
        static_cast<short>(-7),
        static_cast<short>(-1),
        static_cast<short>(0),
        static_cast<short>(42),
        std::numeric_limits<short>::min(),
        std::numeric_limits<short>::min() / 2,
        std::numeric_limits<short>::max() / 2,
        std::numeric_limits<short>::max()));

struct CBotFileUtilsReadWriteUInt32Test : public CBotFileUtilsTest, public testing::WithParamInterface<uint32_t>
{
};

TEST_P(CBotFileUtilsReadWriteUInt32Test, ReadUInt32ValueShouldMatchWrittenValue)
{
    uint32_t expectedValue{GetParam()};
    ASSERT_TRUE(WriteUInt32(stream, expectedValue));
    uint32_t value{1};
    ASSERT_TRUE(ReadUInt32(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteUInt32Test,
    testing::Values(
        static_cast<uint32_t>(0),
        static_cast<uint32_t>(42),
        std::numeric_limits<uint32_t>::max() / 2,
        std::numeric_limits<uint32_t>::max()));

struct CBotFileUtilsReadWriteIntTest : public CBotFileUtilsTest, public testing::WithParamInterface<int>
{
};

TEST_P(CBotFileUtilsReadWriteIntTest, ReadIntValueShouldMatchWrittenValue)
{
    int expectedValue{GetParam()};
    ASSERT_TRUE(WriteInt(stream, expectedValue));
    int value{1};
    ASSERT_TRUE(ReadInt(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteIntTest,
    testing::Values(
        static_cast<int>(7),
        static_cast<int>(-1),
        static_cast<int>(0),
        static_cast<int>(42),
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min() / 2,
        std::numeric_limits<int>::max() / 2,
        std::numeric_limits<int>::max()));

struct CBotFileUtilsReadWriteLongTest : public CBotFileUtilsTest, public testing::WithParamInterface<long>
{
};

TEST_P(CBotFileUtilsReadWriteLongTest, ReadLongValueShouldMatchWrittenValue)
{
    long value{1};
    long expectedValue{GetParam()};
    ASSERT_TRUE(WriteLong(stream, expectedValue));
    ASSERT_TRUE(ReadLong(stream, value));
    ASSERT_EQ(expectedValue, value);
}

TEST_P(CBotFileUtilsReadWriteLongTest, ReadLongValueShouldMatchWrittenValueWithPadding)
{
    constexpr int padding = 10;
    long expectedValue{GetParam()};
    ASSERT_TRUE(WriteLong(stream, expectedValue, padding));
    long value{1};
    ASSERT_TRUE(ReadLong(stream, value));
    ASSERT_EQ(expectedValue, value);
}

TEST_P(CBotFileUtilsReadWriteLongTest, ReadLongValueShouldMatchWrittenValueWithPaddingAndMultipleValues)
{
    constexpr int padding = 10;
    long value{1};
    long expectedValue{GetParam()};
    int anotherValue{1};
    int anotherExpectedValue{2};
    ASSERT_TRUE(WriteLong(stream, expectedValue, padding));
    ASSERT_TRUE(WriteInt(stream, anotherExpectedValue));
    ASSERT_TRUE(ReadLong(stream, value));
    ASSERT_TRUE(ReadInt(stream, anotherValue));
    ASSERT_EQ(expectedValue, value);
    ASSERT_EQ(anotherExpectedValue, anotherValue);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteLongTest,
    testing::Values(
        static_cast<long>(7),
        static_cast<long>(-1),
        static_cast<long>(0),
        static_cast<long>(42),
        std::numeric_limits<long>::min(),
        std::numeric_limits<long>::min() / 2,
        std::numeric_limits<long>::max() / 2,
        std::numeric_limits<long>::max()));

struct CBotFileUtilsReadWriteFloatTest : public CBotFileUtilsTest, public testing::WithParamInterface<float>
{
};

TEST_P(CBotFileUtilsReadWriteFloatTest, ReadFloatValueShouldMatchWrittenValue)
{
    float expectedValue{GetParam()};
    ASSERT_TRUE(WriteFloat(stream, expectedValue));
    float value{1.0f};
    ASSERT_TRUE(ReadFloat(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteFloatTest,
    testing::Values(
        7.0f,
        -1.0f,
        0.0f,
        42.0f,
        3.14f,
        -2.73f,
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min() / 2.0f,
        std::numeric_limits<float>::max() / 2.0f,
        std::numeric_limits<float>::max()));

struct CBotFileUtilsReadWriteDoubleTest : public CBotFileUtilsTest, public testing::WithParamInterface<double>
{
};

TEST_P(CBotFileUtilsReadWriteDoubleTest, ReadDoubleValueShouldMatchWrittenValue)
{
    double expectedValue{GetParam()};
    ASSERT_TRUE(WriteDouble(stream, expectedValue));
    double value{1.0};
    ASSERT_TRUE(ReadDouble(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteDoubleTest,
    testing::Values(
        7.0,
        -1.0,
        0.0,
        42.0,
        3.14,
        -2.73,
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min() / 2.0,
        std::numeric_limits<double>::max() / 2.0,
        std::numeric_limits<double>::max()));

struct CBotFileUtilsReadWriteStringTest : public CBotFileUtilsTest, public testing::WithParamInterface<std::string>
{
};

TEST_P(CBotFileUtilsReadWriteStringTest, ReadStringValueShouldMatchWrittenValue)
{
    std::string expectedValue{GetParam()};
    ASSERT_TRUE(WriteString(stream, expectedValue));
    std::string value{"test"};
    ASSERT_TRUE(ReadString(stream, value));
    ASSERT_EQ(expectedValue, value);
}

INSTANTIATE_TEST_SUITE_P(
    CBotIoReadWriteTest,
    CBotFileUtilsReadWriteStringTest,
    testing::Values(
        "",
        "123",
        "abc",
        createTestStringWithAllPossibleCharacters()));

TEST_F(CBotFileUtilsTest, ReadStreamShouldMatchWrittenStream)
{
    std::string expectedValue{"Lorem ipsum dolor sit amet"};
    std::stringstream initialStream{expectedValue};
    ASSERT_TRUE(WriteStream(stream, initialStream));
    std::stringstream newStream{};
    ASSERT_TRUE(ReadStream(stream, newStream));
    ASSERT_EQ(expectedValue, newStream.str());
}

}
