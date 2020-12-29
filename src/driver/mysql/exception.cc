#include "driver/mysql/exception.h"

#include <iostream>
#include <sstream>
#include <cassert>

namespace sqlcc {
namespace driver {
namespace mysql {

static std::string bind_error_msg(int code, const char* sqlstate, const char* error) {
    std::stringstream buf;
    buf << "Error(" << code << ")  [" << sqlstate << "] \'" << error << "\'";
    return buf.str();
}

sqlcc::Exception exception_from_stmt(MYSQL_STMT* stmt) {
    int code = (int)mysql_stmt_errno(stmt);
    assert(code);

    return sqlcc::Exception(code, bind_error_msg(code, mysql_stmt_sqlstate(stmt), mysql_stmt_error(stmt)));
}

sqlcc::Exception exception_from_mysql(MYSQL* mysql) {
    int code = (int)mysql_errno(mysql);
    assert(code);

    return sqlcc::Exception(code, bind_error_msg(code, mysql_sqlstate(mysql), mysql_error(mysql)));
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
