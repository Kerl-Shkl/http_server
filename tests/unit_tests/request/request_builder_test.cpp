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

TEST(RequestBuilderTest, buildCorrectRequestWithBody)
{
    constexpr std::string_view get_request =
        "GET /test_url HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Length: 9\n"
        "Accept: application/json\n\n"
        "Body text";

    RequestBuilder builder;
    builder.complete(get_request);

    ASSERT_TRUE(builder.isComplete());
    HttpRequest result = builder.getRequest();
    EXPECT_EQ(result.getMethod(), HttpMethod::GET);
    EXPECT_EQ(result.getTarget(), "/test_url");
    EXPECT_EQ(result.getProtocol(), "HTTP/1.1");
    HttpRequest::headers_t expected_headers = {
        {"Host",           "example.com"     },
        {"Accept",         "application/json"},
        {"Content-Length", "9"               }
    };
    EXPECT_EQ(result.getHeaders(), expected_headers);
    EXPECT_EQ(result.getBody(), "Body text");
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

class BatchLoading : public testing::Test
{
protected:
    void breakMethod()
    {
        break_pos = 2;
    }

    void breakTarget()
    {
        break_pos = 8;
    }

    void breakProtocol()
    {
        constexpr size_t protocol_start = request.find("HTTP");
        ASSERT_NE(protocol_start, std::string_view::npos);
        break_pos = protocol_start + 2;
    }

    void breakHeaders()
    {
        constexpr size_t header_pos = request.find("Content-Length");
        ASSERT_NE(header_pos, std::string_view::npos);
        break_pos = header_pos + 4;
    }

    void breakBody()
    {
        constexpr size_t body_pos = request.find("Body");
        ASSERT_NE(body_pos, std::string_view::npos);
        break_pos = body_pos + 4;
    }

    void runTest()
    {
        RequestBuilder builder;
        std::string_view first_part{request.begin(), request.begin() + break_pos};
        std::string_view second_part{request.begin() + break_pos, request.end()};
        builder.complete(first_part);
        ASSERT_FALSE(builder.isComplete());
        builder.complete(second_part);
        ASSERT_TRUE(builder.isComplete());
        HttpRequest result = builder.getRequest();
        EXPECT_EQ(result.getMethod(), HttpMethod::GET);
        EXPECT_EQ(result.getTarget(), "/test_url");
        EXPECT_EQ(result.getProtocol(), "HTTP/1.1");
        HttpRequest::headers_t expected_headers = {
            {"Host",           "example.com"     },
            {"Accept",         "application/json"},
            {"Content-Length", "9"               }
        };
        EXPECT_EQ(result.getHeaders(), expected_headers);
        EXPECT_EQ(result.getBody(), "Body text");
    }

    size_t break_pos = 0;

    static constexpr std::string_view request =
        "GET /test_url HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Length: 9\n"
        "Accept: application/json\n\n"
        "Body text";
};

TEST_F(BatchLoading, methodBreak)
{
    breakMethod();
    runTest();
}

TEST_F(BatchLoading, targetBreak)
{
    breakTarget();
    runTest();
}

TEST_F(BatchLoading, protocolBreak)
{
    breakProtocol();
    runTest();
}

TEST_F(BatchLoading, headersBreak)
{
    breakHeaders();
    runTest();
}

TEST_F(BatchLoading, bodyBreak)
{
    breakBody();
    runTest();
}
