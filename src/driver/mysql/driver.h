#pragma once

#include <map>

#include "sqlcc/driver/driver.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class MySQLDriver : public driver::Driver {
public:
    MySQLDriver();
    std::shared_ptr<driver::Conn> Open(const std::string& name) override;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
