#include "pq_utils.hpp"
#include "pq_database.hpp"
#include <gtest/gtest.h>
#include <pqxx/pqxx>

constexpr auto connection_string = "postgresql://kerl@/test_pq";

namespace {

void clearTestDB()
{
    pqxx::connection connection{connection_string};
    pqxx::work transaction{connection};
    transaction.exec("DELETE FROM only_int;");
    transaction.exec("DELETE FROM only_string;");
    transaction.exec("DELETE FROM int_string;");
    transaction.commit();
}

bool isDBClean()
{
    pqxx::connection connection{connection_string};
    pqxx::work transaction{connection};
    pqxx::row int_count = transaction.exec("SELECT COUNT(*) FROM only_int;").one_row();
    pqxx::row string_count = transaction.exec("SELECT COUNT(*) FROM only_string;").one_row();
    pqxx::row combo_count = transaction.exec("SELECT COUNT(*) FROM int_string;").one_row();
    return int_count[0].as<int>() == 0 && string_count[0].as<int>() == 0 && combo_count[0].as<int>() == 0;
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
        connection = PQconnectdb(connection_string);
        ASSERT_NE(connection, nullptr);
        ASSERT_EQ(PQstatus(connection), CONNECTION_OK);
    }

    void TearDown() override
    {
        clearTestDB();
        PQfinish(connection);
    }

    PGconn *connection{nullptr};
};

TEST_F(TestUtils, addInt)
{
    ResultWrapper res = execParams(connection, "INSERT INTO only_int VALUES ($1);", 7);
    EXPECT_TRUE(res.valid());

    // Validate
    pqxx::connection xxconnection{connection_string};
    pqxx::nontransaction action{xxconnection};
    pqxx::result i = action.exec("SELECT * FROM only_int;");
    EXPECT_EQ(i.size(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.one_field().as<int>(), 7);
}

TEST_F(TestUtils, addCharPtr)
{
    const char *test_str = "string literal";

    ResultWrapper res = execParams(connection, "INSERT INTO only_string VALUES ($1);", test_str);
    EXPECT_TRUE(res.valid());

    // Validate
    pqxx::connection xxconnection{connection_string};
    pqxx::nontransaction action{xxconnection};
    pqxx::result i = action.exec("SELECT * FROM only_string;");
    EXPECT_EQ(i.size(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.one_field().as<std::string>(), std::string{test_str});
}

TEST_F(TestUtils, addStr)
{
    std::string test_str = "std string";

    ResultWrapper res =
        execParams(connection, "INSERT INTO only_string VALUES ($1);", test_str + " prvalue cast");
    EXPECT_TRUE(res.valid());

    // Validate
    pqxx::connection xxconnection{connection_string};
    pqxx::nontransaction action{xxconnection};
    pqxx::result i = action.exec("SELECT * FROM only_string;");
    EXPECT_EQ(i.size(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.one_field().as<std::string>(), test_str + " prvalue cast");
}

TEST_F(TestUtils, addBoth)
{
    std::string test_str = "std string";
    int test_int = 42;

    ResultWrapper res =
        execParams(connection, "INSERT INTO int_string(i, s) VALUES ($1, $2);", test_int, test_str);
    EXPECT_TRUE(res.valid());

    // Validate
    pqxx::connection xxconnection{connection_string};
    pqxx::nontransaction action{xxconnection};
    pqxx::result r = action.exec("SELECT * FROM int_string;");
    pqxx::row row = r.one_row();
    EXPECT_EQ(row.size(), 2);
    EXPECT_EQ(row.front().as<int>(), test_int);
    EXPECT_EQ(row.back().as<std::string>(), test_str);
}

TEST_F(TestUtils, getIntFromResult)
{
    ResultWrapper insert_result = execParams(connection, "INSERT INTO only_int VALUES ($1);", 7);
    ASSERT_TRUE(insert_result.valid());

    ResultWrapper select_res = execParams(connection, "SELECT i FROM only_int WHERE i = $1;", 7);
    ASSERT_TRUE(select_res.valid());
    EXPECT_EQ(select_res.rows(), 1);
    EXPECT_EQ(select_res.columns(), 1);
    EXPECT_EQ(select_res.getOnlyOne<int>(), 7);
    EXPECT_EQ(select_res.get<int>(0, 0), 7);
}

TEST_F(TestUtils, getStringFromResult)
{
    ResultWrapper insert_result =
        execParams(connection, "INSERT INTO only_string VALUES ($1);", "Hello world");
    ASSERT_TRUE(insert_result.valid());

    ResultWrapper select_res =
        execParams(connection, "SELECT s FROM only_string WHERE s = $1;", "Hello world");
    ASSERT_TRUE(select_res.valid());
    EXPECT_EQ(select_res.rows(), 1);
    EXPECT_EQ(select_res.columns(), 1);
    EXPECT_EQ(select_res.getOnlyOne<std::string>(), std::string{"Hello world"});
    EXPECT_EQ(select_res.get<std::string>(0, 0), std::string{"Hello world"});
}

TEST_F(TestUtils, incorrectQuery)
{
    ResultWrapper insert_result = execParams(connection, "INSERT INTO only_string VALUES ($1);", 1);
    ASSERT_FALSE(insert_result.valid());
}

TEST_F(TestUtils, selectEmpty)
{
    ResultWrapper insert_result = execParams(connection, "SELECT * FROM only_string;");
    EXPECT_TRUE(insert_result.valid());
    EXPECT_EQ(insert_result.rows(), 0);
    EXPECT_TRUE(insert_result.empty());
}

TEST_F(TestUtils, selectTwoColumn)
{
    std::string test_str = "std string";
    int test_int = 42;
    ResultWrapper insert_res =
        execParams(connection, "INSERT INTO int_string(i, s) VALUES ($1, $2);", test_int, test_str);
    ASSERT_TRUE(insert_res.valid());

    ResultWrapper select_result = execParams(connection, "SELECT * FROM int_string;");
    EXPECT_TRUE(select_result.valid());
    EXPECT_EQ(select_result.rows(), 1);
    EXPECT_EQ(select_result.columns(), 2);
    EXPECT_EQ(select_result.get<int>(0, 0), test_int);
    EXPECT_EQ(select_result.get<std::string>(0, 1), test_str);
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
