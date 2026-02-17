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

class TestDB : public testing::Test
{
protected:
    void SetUp() override
    {
        database = std::make_unique<PQDatabase>(connection_string);
    }

    void TearDown() override
    {
        clearTestDB();
    }

    std::unique_ptr<IDataBase> database;
};

TEST(Test, addInt)
{
    PGconn *connection = PQconnectdb(connection_string);
    ASSERT_NE(connection, nullptr);
    ASSERT_EQ(PQstatus(connection), CONNECTION_OK);

    ResultWrapper res = execParams(connection, "INSERT INTO only_int VALUES ($1);", 7);
    EXPECT_TRUE(res.valid());

    // Validate
    pqxx::connection xxconnection{connection_string};
    pqxx::nontransaction action{xxconnection};
    pqxx::result i = action.exec("SELECT * FROM only_int;");
    EXPECT_EQ(i.size(), 1);
    EXPECT_EQ(i.columns(), 1);
    EXPECT_EQ(i.one_field().as<int>(), 7);

    clearTestDB();
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new DBCleaner);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
