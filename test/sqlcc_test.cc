#include <gtest/gtest.h>

#include "sqlcc/sqlcc.h"

namespace sqlcc {

TEST(sqlccTest, sqlccTest) {
    DB db = sqlcc::open("mysql", "root:toor@tcp(127.0.0.1:3306)/testdb");
    Result result  = db->exec("insert into table2 (username, age) values(?, ?)", "hello", 123);
    std::cerr << "last_insert_id: " << result->last_insert_id() << " rows: " << result->rows_affected() << std::endl;
    Rows rows = db->query("select id, username, age from table2");
    std::cerr << "query done" << std::endl;
    for (auto column: rows->columns()) {
        std::cerr << "c: " << column << std::endl;
    }
    while(rows->next()) {
        int id;
        std::string username;
        int64_t age;
        rows->scan(&id, &username, &age);
        std::cerr << "id: " << id << " " << username << " " << age << std::endl;
    }
}

} // namespace sqlcc
