#include "driver/mysql/driver.h"

#include "driver/mysql/dsn.h"
#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

MySQLDriver::MySQLDriver() {}

std::shared_ptr<driver::Conn> MySQLDriver::Open(const std::string& dsn) {
    Config cfg = ParseDSN(dsn);
    return std::make_shared<MySQLConn>(cfg);
}

std::shared_ptr<MySQLDriver> CreateMySQLDriver() {
    return std::make_shared<MySQLDriver>();
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
