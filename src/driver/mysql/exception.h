#pragma once

#include "sqlcc/exception.h"

#include <mysql.h>

namespace sqlcc {
namespace driver {
namespace mysql {

// create exception from stmt handler
sqlcc::Exception exception_from_stmt(MYSQL_STMT* stmt);

// create exception from mysql handler
sqlcc::Exception exception_from_mysql(MYSQL* mysql);

} // namespace mysql
} // namespace driver
} // namespace sqlcc
