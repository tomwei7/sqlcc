#pragma once

#include <map>

#include "sqlcc/driver/driver.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class Driver : public driver::Driver {
public:
    Driver();
    driver::Conn Open(const std::string& name) override;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
