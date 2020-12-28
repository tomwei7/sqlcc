#pragma once

#include "sqlcc/driver/driver.h"

#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class Statement : public driver::Statement {
public:
    Statement(Connection* conn, const std::string& query);
    ~Statement();
    std::size_t num_input() override;
    Result exec(const std::vector<Value>& args) override;
    Rows query(const std::vector<Value>& args) override;
private:
    Connection* conn_;
    std::string query_;
    MYSQL_STMT* stmt_;
};

class SQLResult: public driver::SQLResult {
public:
    SQLResult(int64_t last_insert_id, int64_t rows_affected);
    int64_t last_insert_id() override;
    int64_t rows_affected() override;
private:
    int64_t last_insert_id_;
    int64_t rows_affected_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
