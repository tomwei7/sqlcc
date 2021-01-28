#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <iostream>

#include "driver/mysql/driver.h"
#include "driver/mysql/dsn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

TEST(DSN, Basic) {
    std::string dsn = "root:toor@tcp(10.10.2.3:4455)/testdb";
    Config cfg = ParseDSN(dsn);
    EXPECT_EQ("root", cfg.user);
    EXPECT_EQ("toor", cfg.passwd);
    EXPECT_EQ("10.10.2.3", cfg.host);
    EXPECT_EQ(4455, cfg.port);
    EXPECT_EQ("testdb", cfg.dbname);
}

TEST(DSN, WithQuery) {
    std::string dsn = "root:toor@tcp(10.10.2.3:4455)/testdb?timeout=10&read_timeout=6&write_timeout=8&reconnect=0&charset=utf-8";
    Config cfg = ParseDSN(dsn);
    EXPECT_EQ("root", cfg.user);
    EXPECT_EQ("toor", cfg.passwd);
    EXPECT_EQ("10.10.2.3", cfg.host);
    EXPECT_EQ(4455, cfg.port);
    EXPECT_EQ("testdb", cfg.dbname);
    EXPECT_EQ(10, cfg.timeout);
    EXPECT_EQ(6, cfg.read_timeout);
    EXPECT_EQ(8, cfg.write_timeout);
    EXPECT_EQ(0, cfg.reconnect);
    EXPECT_EQ("utf-8", cfg.charset);
}

TEST(DSN, ToString) {
    std::string dsn = "root:toor@tcp(10.10.2.3:4455)/testdb";
    Config cfg = ParseDSN(dsn);
    std::stringstream buf;
    buf << cfg;
    EXPECT_EQ(dsn, buf.str());
}

class MySQLDriverTest: public testing::Test {
protected:
    Driver driver;
};

TEST_F(MySQLDriverTest, Exec) {
    //std::time_t t = std::time(nullptr);
    //std::tm tm = *std::localtime(&t);
    //Conn conn = driver.open("root:toor@tcp(127.0.0.1:3306)/testdb");
    //Stmt stmt = conn->prepare("insert into table1 (username, age, ctime, ok) values(?, ?, ?, ?)");
    //Result result = stmt->exec({"test", 12, tm, 1});
}

TEST_F(MySQLDriverTest, Query) {
    Conn conn = driver.Open("root:toor@tcp(127.0.0.1:3306)/testdb");
    Stmt stmt = conn->Prepare("select * from table1");
    Rows rows = stmt->Query({});
    std::vector<std::string> columns = rows->Columns();
    while(rows->Next()) {
        int64_t id;
        std::string username;
        NullInt64 age;
        std::tm ctime;
        int64_t ok;
        std::vector<Value> values = {id, username, age, ctime, ok};
        rows->Scan(values);
        std::cout << values << std::endl;
    }
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
