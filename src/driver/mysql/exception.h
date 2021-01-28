#pragma once

#include "sqlcc/exception.h"

#include <mysql.h>

namespace sqlcc {
namespace driver {
namespace mysql {

// create exception from stmt handler
sqlcc::Exception ExceptionFromStmt(MYSQL_STMT* stmt);

// create exception from mysql handler
sqlcc::Exception ExceptionFromMySQL(MYSQL* mysql);

} // namespace mysql
} // namespace driver
} // namespace sqlcc
