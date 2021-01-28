#include "driver/mysql/driver.h"

#include "driver/mysql/dsn.h"
#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

Driver::Driver() {}

driver::Conn Driver::Open(const std::string& dsn) {
    Config cfg = ParseDSN(dsn);
    return std::make_shared<Connection>(cfg);
}

std::shared_ptr<Driver> CreateMySQLDriver() {
    return std::make_shared<Driver>();
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
