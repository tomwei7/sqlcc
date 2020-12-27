#include "driver/mysql/driver.h"

#include "driver/mysql/dsn.h"
#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

Driver::Driver() {}

driver::Conn Driver::open(const std::string& dsn) {
    Config cfg = parse_dsn(dsn);
    return std::make_unique<Connection>(cfg);
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
