#pragma once

#include "sqlcc/driver/driver.h"
#include "driver/mysql/dsn.h"

#include <mysql.h>

namespace sqlcc {
namespace driver {
namespace mysql {

class MySQLConn : public driver::Conn {
public:
    MySQLConn(const Config& cfg);
    ~MySQLConn();
    std::shared_ptr<driver::Stmt> Prepare(const std::string& query) override;
    std::shared_ptr<driver::Tx> Begin() override;
    void EnterThread() override;
    void LeaveThread() override;
private:
    friend class MySQLStmt;
    Config cfg_;
    MYSQL mysql_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
