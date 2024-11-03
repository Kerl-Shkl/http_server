#include "first_line_parser.hpp"
#include <gtest/gtest.h>

TEST(FirstLineParserTest, parseCorrectLine)
{
    constexpr std::string_view correct_line = "GET / HTTP/1.1\r\n";
    FirstLineParser parser;
    const bool line_complete = parser.parse(correct_line);
    ASSERT_TRUE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getTarget(), "/");
    EXPECT_EQ(parser.getProtocol(), "HTTP/1.1");
    EXPECT_EQ(parser.getLineEnd(), correct_line.size());
}

TEST(FirstLineParserTest, endlessLine)
{
    constexpr std::string_view incorrect_first_line = "GET /init/ HTTP/1.1\r";
    FirstLineParser parser;
    const bool line_complete = parser.parse(incorrect_first_line);
    EXPECT_FALSE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::INCORRECT);
    EXPECT_EQ(parser.getTarget(), "");
    EXPECT_EQ(parser.getProtocol(), "");
    EXPECT_EQ(parser.getLineEnd(), std::string_view::npos);
}

TEST(FirstLineParserTest, incorrectMethod)
{
    constexpr std::string_view incorrect_first_line = "SET / HTTP/1.1\r\n";
    FirstLineParser parser;
    EXPECT_THROW(parser.parse(incorrect_first_line), IncorrectMethod);
}

TEST(FirstLineParserTest, withoutPostTargetSpace)
{
    constexpr std::string_view incorrect_first_line = "GET /init\r\n";
    FirstLineParser parser;
    EXPECT_THROW(parser.parse(incorrect_first_line), IncorrectFirstLine);
}

TEST(FirstLineParserTest, withoutTarget)
{
    constexpr std::string_view incorrect_first_line = "GET  HTTP/1.1\r\n";
    FirstLineParser parser;
    EXPECT_THROW(parser.parse(incorrect_first_line), IncorrectFirstLine);
}

TEST(FirstLineParserTest, withoutProtocol)
{
    constexpr std::string_view incorrect_first_line = "GET /init/ \r\n";
    FirstLineParser parser;
    EXPECT_THROW(parser.parse(incorrect_first_line), IncorrectFirstLine);
}
