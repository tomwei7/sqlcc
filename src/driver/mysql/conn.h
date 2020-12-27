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
    Stmt prepare(const std::string& query) override;
    Tx begin() override;
    void enter_thread() override;
    void leave_thread() override;
private:
    Config cfg_;
    MYSQL mysql_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
