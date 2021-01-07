#pragma once

#include <sqlcc/driver/driver.h>

#include <iostream>
#include <utility>

namespace sqlcc {

template <typename... Args>
std::vector<driver::Value> merge_const_values(const Args&... args) {
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

template <typename... Args>
std::vector<driver::Value> merge_pointer_values(Args*... args) {
    std::vector<driver::Value> values;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
    std::initializer_list<int>{(
        [&values](auto&& args) {
            values.push_back(*args);
        }(std::forward<Args*>(args)),
        0)...};
#pragma clang diagnostic pop
    return values;
}

template <typename... Args>
std::vector<driver::Value> split_dest_values(std::vector<driver::Value>& values, Args*... args) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
    std::size_t idx = 0;
    std::initializer_list<int>{(
        [&values, &idx](auto args) {
        //std::visit([&args](auto&& value){
        //        //using T = std::remove_pointer_t<decltype(args)>;
        //        //*args = static_cast<T>(value);
        //        *args = value;
        //    }, values[idx]);
            using T = std::remove_pointer_t<decltype(args)>;
            *args = std::get<T>(values[idx]);
            idx++;
        }(std::forward<Args*>(args)),
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

class SQLRows {
   public:
    virtual ~SQLRows() {}
    virtual const std::vector<std::string>& columns() const = 0;
    virtual bool next() = 0;
    template <typename... Args>
    void scan(Args*... args) {
        std::vector<driver::Value> dest = merge_pointer_values(args...);
        do_scan(dest);
        split_dest_values(dest, args...);
    }
   protected:
    virtual void do_scan(std::vector<driver::Value>& dest) = 0;
};

class Statement {
   public:
    virtual ~Statement() {}
    template <typename... Args>
    Result exec(const Args&... args) {
        std::vector<driver::Value> args_values = merge_const_values(args...);
        return do_exec(args_values);
    }
    template <typename... Args>
    Rows query(const Args&... args) {
        std::vector<driver::Value> args_values = merge_const_values(args...);
        return do_query(args_values);
    }
   protected:
    virtual Result do_exec(const std::vector<driver::Value>& args) = 0;
    virtual Rows do_query(const std::vector<driver::Value>& args) = 0;
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
        std::vector<driver::Value> args_values = merge_const_values(args...);
        return do_exec(query, args_values);
    }
    template <typename... Args>
    Rows query(const std::string& query, const Args&... args) {
        std::vector<driver::Value> args_values = merge_const_values(args...);
        return do_query(query, args_values);
    }
    virtual Stmt prepare(const std::string& query) = 0;
   protected:
    virtual Result do_exec(const std::string& query,
                           const std::vector<driver::Value>& args) = 0;
    virtual Rows do_query(const std::string& query, const std::vector<driver::Value>& args) = 0;
};

DB open(const std::string& driver_name, const std::string& dsn);

}  // namespace sqlcc
