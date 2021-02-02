#pragma once

#include "sqlcc/driver/driver.h"

#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class MySQLStmt : public driver::Stmt {
public:
    MySQLStmt(MySQLConn* conn, const std::string& query);
    ~MySQLStmt();
    std::size_t NumInput() override;
    std::shared_ptr<driver::SQLResult> Exec(const std::vector<Value>& args) override;
    std::shared_ptr<driver::SQLRows> Query(const std::vector<Value>& args) override;
private:
    void BindValue(const std::vector<Value>& args);
    MySQLConn* conn_;
    std::string query_;
    MYSQL_STMT* stmt_;
    MYSQL_BIND* bind_;
    std::size_t bind_size_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
