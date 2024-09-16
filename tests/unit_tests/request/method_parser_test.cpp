#include "method_parser.hpp"
#include <gtest/gtest.h>

TEST(MethodParser, CorrectMethodReturn)
{
    const std::string method{"GET"};
    auto ansver = MethodParser::parse(method);
    EXPECT_FALSE(ansver.need_more);
    EXPECT_EQ(ansver.method, HttpMethod::GET);
    EXPECT_EQ(ansver.method_end, std::string::npos);
}

TEST(MethodParser, IncorrectMethodReturn)
{
    const std::string method{"GES"};
    auto ansver = MethodParser::parse(method);
    EXPECT_FALSE(ansver.need_more);
    EXPECT_EQ(ansver.method, HttpMethod::INCORRECT);
}

TEST(MethodParser, PrefixMethodReturn)
{
    const std::string method{"GE"};
    auto ansver = MethodParser::parse(method);
    EXPECT_TRUE(ansver.need_more);
}

TEST(MethodParser, EmptyMethodIsPrefix)
{
    const std::string method{""};
    auto ansver = MethodParser::parse(method);
    EXPECT_TRUE(ansver.need_more);
}

TEST(MethodParser, ExtraSymbolsAreError)
{
    const std::string method{"GET!"};
    auto ansver = MethodParser::parse(method);
    EXPECT_FALSE(ansver.need_more);
    EXPECT_EQ(ansver.method, HttpMethod::INCORRECT);
}

TEST(MethodParser, CorrectMethodEndPos)
{
    const std::string method{"GET another coin"};
    auto ansver = MethodParser::parse(method);
    EXPECT_FALSE(ansver.need_more);
    EXPECT_EQ(ansver.method, HttpMethod::GET);
    EXPECT_EQ(ansver.method_end, method.find_first_of(' '));
}
