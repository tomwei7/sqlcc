#pragma once

#include "sqlcc/driver/driver.h"

#include "driver/mysql/conn.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class Statement : public driver::Statement {
public:
    Statement(Connection* conn, const std::string& query);
    ~Statement();
    std::size_t num_input() override;
    Result exec(const std::vector<Value>& args) override;
    Rows query(const std::vector<Value>& args) override;
private:
    void bind_value(const std::vector<Value>& args);
    Connection* conn_;
    std::string query_;
    MYSQL_STMT* stmt_;
    MYSQL_BIND* bind_;
    std::size_t bind_size_;
};

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
