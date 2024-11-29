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

TEST(FirstLineParserTest, complexTarget)
{
    constexpr std::string_view complex_line =
        "GET /over/there;name=kerl/here;user=kirill?name=kerl&user=kirill#end HTTP/1.1\r\n";
    FirstLineParser parser;
    const bool line_complete = parser.parse(complex_line);
    EXPECT_TRUE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getTarget(), "/over/there;name/here;user");
    EXPECT_EQ(parser.getProtocol(), "HTTP/1.1");
    EXPECT_EQ(parser.getLineEnd(), complex_line.size());
    EXPECT_EQ(parser.getParameters()["user"], "kirill");
    EXPECT_EQ(parser.getParameters()["name"], "kerl");
    EXPECT_EQ(parser.getQueries()["user"], "kirill");
    EXPECT_EQ(parser.getQueries()["name"], "kerl");
    EXPECT_EQ(parser.getFragment(), "end");
}

TEST(FirstLineParserTest, withoutFragment)
{
    constexpr std::string_view complex_line =
        "GET /over/there;name=kerl/here;user=kirill?name=kerl&user=kirill HTTP/1.1\r\n";
    FirstLineParser parser;
    const bool line_complete = parser.parse(complex_line);
    EXPECT_TRUE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getTarget(), "/over/there;name/here;user");
    EXPECT_EQ(parser.getProtocol(), "HTTP/1.1");
    EXPECT_EQ(parser.getLineEnd(), complex_line.size());
    EXPECT_EQ(parser.getParameters()["user"], "kirill");
    EXPECT_EQ(parser.getParameters()["name"], "kerl");
    EXPECT_EQ(parser.getQueries()["user"], "kirill");
    EXPECT_EQ(parser.getQueries()["name"], "kerl");
    EXPECT_TRUE(parser.getFragment().empty());
}

TEST(FirstLineParserTest, withoutQueries)
{
    constexpr std::string_view complex_line = "GET /over/there;name=kerl/here;user=kirill HTTP/1.1\r\n";
    FirstLineParser parser;
    const bool line_complete = parser.parse(complex_line);
    EXPECT_TRUE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getTarget(), "/over/there;name/here;user");
    EXPECT_EQ(parser.getProtocol(), "HTTP/1.1");
    EXPECT_EQ(parser.getLineEnd(), complex_line.size());
    EXPECT_EQ(parser.getParameters()["user"], "kirill");
    EXPECT_EQ(parser.getParameters()["name"], "kerl");
    EXPECT_TRUE(parser.getQueries().empty());
    EXPECT_TRUE(parser.getFragment().empty());
}

TEST(FirstLineParserTest, withoutParameters)
{
    constexpr std::string_view complex_line = "GET /over/there/here?name=kerl&user=kirill HTTP/1.1\r\n";
    FirstLineParser parser;
    const bool line_complete = parser.parse(complex_line);
    EXPECT_TRUE(line_complete);
    EXPECT_EQ(parser.getMethod(), HttpMethod::GET);
    EXPECT_EQ(parser.getTarget(), "/over/there/here");
    EXPECT_EQ(parser.getProtocol(), "HTTP/1.1");
    EXPECT_EQ(parser.getLineEnd(), complex_line.size());
    EXPECT_EQ(parser.getQueries()["user"], "kirill");
    EXPECT_EQ(parser.getQueries()["name"], "kerl");
    EXPECT_TRUE(parser.getParameters().empty());
    EXPECT_TRUE(parser.getFragment().empty());
}
