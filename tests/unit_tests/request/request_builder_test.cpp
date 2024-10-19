#include "request_builder.hpp"
#include <gtest/gtest.h>

TEST(RequestBuilderTest, buildCorrectRequest)
{
    constexpr std::string_view get_request =
        "GET /test_url HTTP/1.1\n"
        "Host: example.com\n"
        "Accept: application/json\n\n";

    RequestBuilder builder;
    builder.complete(get_request);

    ASSERT_TRUE(builder.isComplete());
    HttpRequest result = builder.getRequest();
    EXPECT_EQ(result.getMethod(), HttpMethod::GET);
    EXPECT_EQ(result.getTarget(), "/test_url");
    EXPECT_EQ(result.getProtocol(), "HTTP/1.1");
    HttpRequest::headers_t expected_headers = {
        {"Host",   "example.com"     },
        {"Accept", "application/json"}
    };
    EXPECT_EQ(result.getHeaders(), expected_headers);
}

TEST(RequestBuilderTest, endlessRequest)
{
    constexpr std::string_view endless_request =
        "GET /test_url HTTP/1.1\n"
        "Host: example.com\n"
        "Accept: application/json\n";

    RequestBuilder builder;
    builder.complete(endless_request);

    ASSERT_FALSE(builder.isComplete());
}

TEST(RequestBuilderTest, batchLoading)
{
    constexpr std::string_view first_part =
        "GET /test_url HTTP/1.1\n"
        "Host: example.com\n";
    constexpr std::string_view second_part = "Accept: application/json\n\n";

    RequestBuilder builder;
    builder.complete(first_part);
    ASSERT_FALSE(builder.isComplete());
    builder.complete(second_part);
    ASSERT_TRUE(builder.isComplete());
    HttpRequest result = builder.getRequest();
    EXPECT_EQ(result.getMethod(), HttpMethod::GET);
    EXPECT_EQ(result.getTarget(), "/test_url");
    EXPECT_EQ(result.getProtocol(), "HTTP/1.1");
    HttpRequest::headers_t expected_headers = {
        {"Host",   "example.com"     },
        {"Accept", "application/json"}
    };
    EXPECT_EQ(result.getHeaders(), expected_headers);
}
