#pragma once

#include "sqlcc/driver/driver.h"
#include "driver/mysql/dsn.h"

#include <mysql.h>

namespace sqlcc {
namespace driver {
namespace mysql {

class Connection : public driver::Connection {
public:
    Connection(const Config& cfg);
    ~Connection();
    Stmt Prepare(const std::string& query) override;
    Tx Begin() override;
    void EnterThread() override;
    void LeaveThread() override;
private:
    friend class Statement;
    Config cfg_;
    MYSQL mysql_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
