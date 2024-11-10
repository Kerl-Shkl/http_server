#include "headers_builder.hpp"
#include <gtest/gtest.h>

TEST(HeadersBuilderTest, addCorrectHeader)
{
    constexpr std::string_view correct_header = "test_key : test_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(correct_header);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_TRUE(headers.contains("test_key"));
    EXPECT_EQ(headers["test_key"], "test_value");
    EXPECT_EQ(headers_end, correct_header.size());
}

TEST(HeadersBuilderTest, extraSpaces)
{
    constexpr std::string_view correct_header = "   test_key   :  test_value  \r\n\r\n";
    HeadersBuilder builder;
    builder.add(correct_header);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_TRUE(headers.contains("test_key"));
    EXPECT_EQ(headers["test_key"], "test_value");
    EXPECT_EQ(headers_end, correct_header.size());
}

TEST(HeadersBuilderTest, separateEnd)
{
    constexpr std::string_view first_part = "   test_key   :  test_value  \r\n";
    constexpr std::string_view second_part = "\r\nanother part";
    HeadersBuilder builder;
    builder.add(first_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(second_part);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_EQ(headers.size(), 1);
    EXPECT_TRUE(headers.contains("test_key"));
    EXPECT_EQ(headers["test_key"], "test_value");
    EXPECT_EQ(headers_end, second_part.find_first_not_of("\n\r"));
}

// TEST(HeadersBuilderTest, fuckingCrutch)
// {
//     constexpr std::string_view first_part = "   test_key   :  test_value  \r\n\r";  // without last \n
//     HeadersBuilder builder;
//     builder.add(first_part);
//     ASSERT_TRUE(builder.isComplete());  // crutch invariant
// }

TEST(HeadersBuilderTest, addFewCorrectHeader)
{
    constexpr std::string_view first_header = "first_key : first_value\r\n";
    constexpr std::string_view second_header = "second_key : second_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(first_header);
    ASSERT_FALSE(builder.isComplete());
    builder.add(second_header);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_TRUE(headers.contains("first_key"));
    EXPECT_TRUE(headers.contains("second_key"));
    EXPECT_EQ(headers["first_key"], "first_value");
    EXPECT_EQ(headers["second_key"], "second_value");
    EXPECT_EQ(headers_end, second_header.size());
}

TEST(HeadersBuilderTest, keyBreak)
{
    constexpr std::string_view first_part = "first_key : first_value\r\n";
    constexpr std::string_view second_part = "second";
    constexpr std::string_view third_part = "_key : second_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(first_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(second_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(third_part);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_TRUE(headers.contains("first_key"));
    EXPECT_TRUE(headers.contains("second_key"));
    EXPECT_EQ(headers["first_key"], "first_value");
    EXPECT_EQ(headers["second_key"], "second_value");
    EXPECT_EQ(headers_end, third_part.size());
}

TEST(HeadersBuilderTest, valueBreak)
{
    constexpr std::string_view first_part = "first_key : first_value\r\n";
    constexpr std::string_view second_part = "second_key : second";
    constexpr std::string_view third_part = "_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(first_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(second_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(third_part);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    EXPECT_TRUE(headers.contains("first_key"));
    EXPECT_TRUE(headers.contains("second_key"));
    EXPECT_EQ(headers["first_key"], "first_value");
    EXPECT_EQ(headers["second_key"], "second_value");
    EXPECT_EQ(headers_end, third_part.size());
}

TEST(HeadersBuilderTest, multilineHeaderValue)
{
    constexpr std::string_view multiline_header =
        "test_key : test_value\r\n"
        "multiline : first line\r\n second_line\r\n"
        "last_key : last_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(multiline_header);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    ASSERT_TRUE(headers.contains("test_key"));
    EXPECT_EQ(headers["test_key"], "test_value");
    ASSERT_TRUE(headers.contains("multiline"));
    EXPECT_EQ(headers["multiline"], "first line\r\n second_line");
    ASSERT_TRUE(headers.contains("last_key"));
    EXPECT_EQ(headers["last_key"], "last_value");
    EXPECT_EQ(headers_end, multiline_header.size());
}

TEST(HeadersBuilderTest, multilineSeparatedValue)
{
    constexpr std::string_view first_part =
        "test_key : test_value\r\n"
        "multiline : first line\r";
    constexpr std::string_view second_part = "\n";
    constexpr std::string_view third_part =
        " second_line\r\n"
        "last_key : last_value\r\n\r\n";
    HeadersBuilder builder;
    builder.add(first_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(second_part);
    ASSERT_FALSE(builder.isComplete());
    builder.add(third_part);
    ASSERT_TRUE(builder.isComplete());
    auto headers = builder.getHeaders();
    auto headers_end = builder.getAfterHeadersPos();
    ASSERT_TRUE(headers.contains("test_key"));
    EXPECT_EQ(headers["test_key"], "test_value");
    ASSERT_TRUE(headers.contains("multiline"));
    EXPECT_EQ(headers["multiline"], "first line\r\n second_line");
    ASSERT_TRUE(headers.contains("last_key"));
    EXPECT_EQ(headers["last_key"], "last_value");
    EXPECT_EQ(headers_end, third_part.size());
}
