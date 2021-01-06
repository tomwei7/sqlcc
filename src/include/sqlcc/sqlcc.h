#pragma once

#include <sqlcc/driver/driver.h>

#include <iostream>
#include <utility>

namespace sqlcc {

template <typename... Args>
std::vector<driver::Value> merge_values(const Args&... args) {
    std::vector<driver::Value> values;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
    std::initializer_list<int>{(
        [&values](const auto&& value) {
            values.push_back(value);
        }(std::forward<const Args>(args)),
        0)...};
#pragma clang diagnostic pop
    return values;
}

class Database;
class Connection;
class Statement;
class Transaction;
class SQLResult;
class SQLRows;

using DB = std::shared_ptr<Database>;
using Conn = std::shared_ptr<Connection>;
using Stmt = std::shared_ptr<Statement>;
using Tx = std::shared_ptr<Transaction>;
using Result = std::shared_ptr<SQLResult>;
using Rows = std::shared_ptr<SQLRows>;

class SQLResult {
   public:
    virtual ~SQLResult(){};
    virtual int64_t last_insert_id() = 0;
    virtual int64_t rows_affected() = 0;
};

class Statement {
   public:
    virtual ~Statement() {}
    virtual Result exec(const std::vector<driver::Value>& args) = 0;
};

class Connection {
   public:
    virtual ~Connection() {}
    virtual Stmt prepare(const std::string& query) = 0;
};

class Database {
   public:
    virtual ~Database(){};
    virtual Conn conn() = 0;
    virtual void ping() = 0;
    virtual std::shared_ptr<driver::Driver> driver() = 0;
    template <typename... Args>
    Result exec(const std::string& query, const Args&... args) {
        std::vector<driver::Value> args_values = merge_values(args...);
        return do_exec(query, args_values);
    }
   protected:
    virtual Result do_exec(const std::string& query,
                           const std::vector<driver::Value>& args) = 0;
};

DB open(const std::string& driver_name, const std::string& dsn);

}  // namespace sqlcc
