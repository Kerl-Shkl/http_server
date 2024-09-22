#include "headers_builder.hpp"
#include <gtest/gtest.h>

TEST(HeadersBuilderTest, addCorrectHeader)
{
    constexpr std::string_view correct_header = "test_key : test_value\n\n";
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
    constexpr std::string_view correct_header =
        "   test_key   :  test_value  \n\n";
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
    constexpr std::string_view first_part = "   test_key   :  test_value  \n";
    constexpr std::string_view second_part = "\nanother part";
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
    EXPECT_EQ(headers_end, second_part.find_first_not_of('\n'));
}

TEST(HeadersBuilderTest, addFewCorrectHeader)
{
    constexpr std::string_view first_header = "first_key : first_value\n";
    constexpr std::string_view second_header = "second_key : second_value\n\n";
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
    constexpr std::string_view first_part = "first_key : first_value\n";
    constexpr std::string_view second_part = "second";
    constexpr std::string_view third_part = "_key : second_value\n\n";
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
    constexpr std::string_view first_part = "first_key : first_value\n";
    constexpr std::string_view second_part = "second_key : second";
    constexpr std::string_view third_part = "_value\n\n";
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
