#pragma once

#include <sqlcc/driver/driver.h>

#include <iostream>
#include <utility>

namespace sqlcc {

template <typename... Args>
std::vector<driver::Value> MergeConstValues(const Args&... args) {
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
std::vector<driver::Value> MergePointerValues(Args*... args) {
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
std::vector<driver::Value> SplitDestValues(std::vector<driver::Value>& values,
                                             Args*... args) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
    std::size_t idx = 0;
    std::initializer_list<int>{(
        [&values, &idx](auto args) {
            std::visit(
                [args](auto&& value) {
                    using value_t = std::decay_t<decltype(value)>;
                    using args_t = std::remove_pointer_t<Args>;
                    if constexpr (std::is_same_v<value_t, args_t>) {
                        *args = static_cast<args_t>(value);
                    } else if constexpr (std::is_same_v<value_t, int64_t> &&
                                         std::is_integral_v<args_t>) {
                        *args = static_cast<args_t>(value);
                    } else if constexpr (std::is_same_v<value_t, uint64_t> &&
                                         std::is_integral_v<args_t>) {
                        *args = static_cast<args_t>(value);
                    } else if constexpr (std::is_same_v<value_t, double> &&
                                         std::is_floating_point_v<args_t>) {
                        *args = static_cast<args_t>(value);
                    } else {
                        throw std::runtime_error("unable to scan value");
                    }
                },
                values[idx]);
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
using Stmt = std::shared_ptr<Statement>;
using Tx = std::shared_ptr<Transaction>;
using Result = std::shared_ptr<SQLResult>;
using Rows = std::shared_ptr<SQLRows>;

class SQLResult {
   public:
    virtual ~SQLResult(){};
    virtual int64_t LastInsertID() = 0;
    virtual int64_t RowsAffected() = 0;
};

class SQLRows {
   public:
    virtual ~SQLRows() {}
    virtual const std::vector<std::string>& Columns() const = 0;
    virtual bool Next() = 0;
    template <typename... Args>
    void scan(Args*... args) {
        std::vector<driver::Value> dest = MergePointerValues(args...);
        DoScan(dest);
        SplitDestValues(dest, args...);
    }

   protected:
    virtual void DoScan(std::vector<driver::Value>& dest) = 0;
};

class Statement {
   public:
    virtual ~Statement() {}
    template <typename... Args>
    Result Exec(const Args&... args) {
        std::vector<driver::Value> args_values = MergeConstValues(args...);
        return DoExec(args_values);
    }
    template <typename... Args>
    Rows Query(const Args&... args) {
        std::vector<driver::Value> args_values = MergeConstValues(args...);
        return DoQuery(args_values);
    }

   protected:
    virtual Result DoExec(const std::vector<driver::Value>& args) = 0;
    virtual Rows DoQuery(const std::vector<driver::Value>& args) = 0;
};

class Connection {
   public:
    virtual ~Connection() {}
    virtual Stmt Prepare(const std::string& query) = 0;
};

class Database {
   public:
    virtual ~Database(){};
    virtual std::shared_ptr<Connection> Conn() = 0;
    virtual void Ping() = 0;
    virtual std::shared_ptr<driver::Driver> Driver() = 0;
    template <typename... Args>
    Result exec(const std::string& query, const Args&... args) {
        std::vector<driver::Value> args_values = MergeConstValues(args...);
        return DoExec(query, args_values);
    }
    template <typename... Args>
    Rows query(const std::string& query, const Args&... args) {
        std::vector<driver::Value> args_values = MergeConstValues(args...);
        return DoQuery(query, args_values);
    }
    virtual Stmt Prepare(const std::string& query) = 0;

   protected:
    virtual Result DoExec(const std::string& query,
                           const std::vector<driver::Value>& args) = 0;
    virtual Rows DoQuery(const std::string& query,
                          const std::vector<driver::Value>& args) = 0;
};

DB Open(const std::string& driver_name, const std::string& dsn);

}  // namespace sqlcc
