#include "pq_utils.hpp"
#include "pq_connection.hpp"
#include <gtest/gtest.h>

constexpr auto connection_string = "postgresql://kerl@/test_pq";

namespace {

void clearTestDB()
{
    PQConnection connection{connection_string};
    connection.connect();
    connection.exec("DELETE FROM only_int;");
    connection.exec("DELETE FROM only_string;");
    connection.exec("DELETE FROM int_string;");
}

bool isDBClean()
{
    PQConnection connection{connection_string};
    connection.connect();
    ResultWrapper int_count = connection.exec("SELECT COUNT(*) FROM only_int;");
    ResultWrapper string_count = connection.exec("SELECT COUNT(*) FROM only_string;");
    ResultWrapper combo_count = connection.exec("SELECT COUNT(*) FROM int_string;");
    return int_count.getOnlyOne<int>() == 0 && string_count.getOnlyOne<int>() == 0 &&
           combo_count.getOnlyOne<int>() == 0;
}

}  // namespace

class DBCleaner : public ::testing::Environment
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(isDBClean());
    }
    void TearDown() override
    {
        clearTestDB();
        ASSERT_TRUE(isDBClean());
    }
};

class TestUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        connection.connect();
        ASSERT_TRUE(connection.connected());
    }

    void TearDown() override
    {
        clearTestDB();
    }

    PQConnection connection{connection_string};
};

TEST_F(TestUtils, addInt)
{
    ResultWrapper res = connection.exec("INSERT INTO only_int VALUES ($1);", 7);
    EXPECT_TRUE(res.valid());

    // Validate
    ResultWrapper i = connection.exec("SELECT * FROM only_int;");
    EXPECT_EQ(i.rows(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.getOnlyOne<int>(), 7);
}

TEST_F(TestUtils, addCharPtr)
{
    const char *test_str = "string literal";

    ResultWrapper res = connection.exec("INSERT INTO only_string VALUES ($1);", test_str);
    EXPECT_TRUE(res.valid());

    // Validate
    ResultWrapper i = connection.exec("SELECT * FROM only_string;");
    EXPECT_EQ(i.rows(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.getOnlyOne<std::string>(), std::string{test_str});
}

TEST_F(TestUtils, addStr)
{
    std::string test_str = "std string";

    ResultWrapper res = connection.exec("INSERT INTO only_string VALUES ($1);", test_str + " prvalue cast");
    EXPECT_TRUE(res.valid());

    // Validate
    ResultWrapper i = connection.exec("SELECT * FROM only_string;");
    EXPECT_EQ(i.rows(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.getOnlyOne<std::string>(), test_str + " prvalue cast");
}

TEST_F(TestUtils, addBoth)
{
    std::string test_str = "std string";
    int test_int = 42;

    ResultWrapper res = connection.exec("INSERT INTO int_string(i, s) VALUES ($1, $2);", test_int, test_str);
    EXPECT_TRUE(res.valid());

    // Validate
    ResultWrapper r = connection.exec("SELECT * FROM int_string;");
    EXPECT_EQ(r.rows(), 1);
    EXPECT_EQ(r.columns(), 2);
    EXPECT_EQ(r.get<int>(0, 0), test_int);
    EXPECT_EQ(r.get<std::string>(0, 1), test_str);
}

TEST_F(TestUtils, getIntFromResult)
{
    ResultWrapper insert_result = connection.exec("INSERT INTO only_int VALUES ($1);", 7);
    ASSERT_TRUE(insert_result.valid());

    ResultWrapper select_res = connection.exec("SELECT i FROM only_int WHERE i = $1;", 7);
    ASSERT_TRUE(select_res.valid());
    EXPECT_EQ(select_res.rows(), 1);
    EXPECT_EQ(select_res.columns(), 1);
    EXPECT_EQ(select_res.getOnlyOne<int>(), 7);
    EXPECT_EQ(select_res.get<int>(0, 0), 7);
}

TEST_F(TestUtils, getStringFromResult)
{
    ResultWrapper insert_result = connection.exec("INSERT INTO only_string VALUES ($1);", "Hello world");
    ASSERT_TRUE(insert_result.valid());

    ResultWrapper select_res = connection.exec("SELECT s FROM only_string WHERE s = $1;", "Hello world");
    ASSERT_TRUE(select_res.valid());
    EXPECT_EQ(select_res.rows(), 1);
    EXPECT_EQ(select_res.columns(), 1);
    EXPECT_EQ(select_res.getOnlyOne<std::string>(), std::string{"Hello world"});
    EXPECT_EQ(select_res.get<std::string>(0, 0), std::string{"Hello world"});
}

TEST_F(TestUtils, incorrectQuery)
{
    ResultWrapper insert_result = connection.exec("INSERT INTO only_string VALUES ($1);", 1);
    ASSERT_FALSE(insert_result.valid());
}

TEST_F(TestUtils, selectEmpty)
{
    ResultWrapper insert_result = connection.exec("SELECT * FROM only_string;");
    EXPECT_TRUE(insert_result.valid());
    EXPECT_EQ(insert_result.rows(), 0);
    EXPECT_TRUE(insert_result.empty());
}

TEST_F(TestUtils, selectTwoColumn)
{
    std::string test_str = "std string";
    int test_int = 42;
    ResultWrapper insert_res =
        connection.exec("INSERT INTO int_string(i, s) VALUES ($1, $2);", test_int, test_str);
    ASSERT_TRUE(insert_res.valid());

    ResultWrapper select_result = connection.exec("SELECT * FROM int_string;");
    EXPECT_TRUE(select_result.valid());
    EXPECT_EQ(select_result.rows(), 1);
    EXPECT_EQ(select_result.columns(), 2);
    EXPECT_EQ(select_result.get<int>(0, 0), test_int);
    EXPECT_EQ(select_result.get<std::string>(0, 1), test_str);
}

TEST_F(TestUtils, insertNull)
{
    std::string test_str = "string";
    std::optional<std::string> null_opt_str;
    std::string query = "INSERT INTO int_string(i, s) VALUES ($1, $2);";

    ResultWrapper insert_1 = connection.exec(query, 0, std::optional<std::string>{test_str});
    EXPECT_TRUE(insert_1.valid());
    ResultWrapper insert_2 = connection.exec(query, 1, null_opt_str);
    EXPECT_TRUE(insert_2.valid());
    ResultWrapper insert_3 = connection.exec(query, 2, std::nullopt);
    EXPECT_TRUE(insert_3.valid());

    std::string select_query = "SELECT s FROM int_string WHERE i=$1;";
    {
        ResultWrapper select_result = connection.exec(select_query, 0);
        EXPECT_TRUE(select_result.valid());
        EXPECT_EQ(select_result.rows(), 1);
        EXPECT_EQ(select_result.getOnlyOne<std::string>(), test_str);
    }

    {
        ResultWrapper select_result = connection.exec(select_query, 1);
        EXPECT_TRUE(select_result.valid());
        EXPECT_EQ(select_result.rows(), 1);
        EXPECT_EQ(select_result.getOnlyOne<std::string>(), "");
    }

    {
        ResultWrapper select_result = connection.exec(select_query, 2);
        EXPECT_TRUE(select_result.valid());
        EXPECT_EQ(select_result.rows(), 1);
        EXPECT_EQ(select_result.getOnlyOne<std::string>(), "");
    }
}

TEST_F(TestUtils, TransactionCommit)
{
    ASSERT_TRUE(connection.transactionReady());
    connection.beginTransaction();

    EXPECT_FALSE(connection.transactionReady());
    ResultWrapper insert_result = connection.exec("INSERT INTO only_int VALUES ($1);", 7);
    ASSERT_TRUE(insert_result.valid());

    connection.commitTransaction();

    ResultWrapper select_res = connection.exec("SELECT i FROM only_int WHERE i = $1;", 7);
    ASSERT_TRUE(select_res.valid());
    EXPECT_EQ(select_res.getOnlyOne<int>(), 7);
}

TEST_F(TestUtils, TransactionRollback)
{
    ASSERT_TRUE(connection.transactionReady());
    connection.beginTransaction();

    ResultWrapper insert_result = connection.exec("INSERT INTO only_int VALUES ($1);", 7);
    ASSERT_TRUE(insert_result.valid());

    connection.rollbackTransaction();

    ResultWrapper select_res = connection.exec("SELECT i FROM only_int WHERE i = $1;", 7);
    ASSERT_TRUE(select_res.valid());
    EXPECT_TRUE(select_res.empty());
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
