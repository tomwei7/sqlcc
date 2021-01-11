#pragma once

#include <ctime>
#include <iomanip>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace sqlcc {
namespace driver {

class Connection;
class Statement;
class Transaction;
class SQLResult;
class SQLRows;

template <class>
inline constexpr bool always_false_v = false;

template <typename T>
class NullValue {
   public:
    using reference_t = NullValue<T> &;
    using const_reference_t = const NullValue<T> &;

    NullValue() : is_null_(true) {}
    NullValue(const T &v) : is_null_(false), data_(v) {}

    operator bool() const { return !is_null_; }
    bool operator==(const T &v) { return !is_null_ && data_ == v; }
    reference_t operator=(const T &v) {
        return data_ = v, is_null_ = false, *this;
    }
    T &operator*() { return is_null_=false, data_; }
    const T &operator*() const { return data_; }
    friend std::ostream &operator<<(std::ostream &os, const_reference_t value) {
        if (!value) {
            os << "null";
        } else {
            os << value.data_;
        }
    return os;
    }
    friend bool operator==(const_reference_t v, std::nullptr_t) {
        return v.is_null_;
    }
    friend bool operator==(std::nullptr_t, const_reference_t v) {
        return v.is_null_;
    }
   private:
    bool is_null_;
    T data_;
};

using NullInt64 = NullValue<int64_t>;
using NullUInt64 = NullValue<uint64_t>;
using NullDouble = NullValue<double>;
using NullString = NullValue<std::string>;
using NullTm = NullValue<std::tm>;

using Value = std::variant<int64_t, uint64_t, double, std::string, std::tm, NullInt64, NullUInt64, NullDouble, NullString, NullTm>;

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
    virtual const std::vector<std::string> &columns() const = 0;
    virtual bool next() = 0;
    virtual void scan(std::vector<Value> &dest) = 0;
};

class Statement {
   public:
    virtual ~Statement(){};
    virtual std::size_t num_input() = 0;
    virtual Result exec(const std::vector<Value> &args) = 0;
    virtual Rows query(const std::vector<Value> &args) = 0;
};

class Transaction {
   public:
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

class Connection {
   public:
    virtual Stmt prepare(const std::string &query) = 0;
    virtual Tx begin() = 0;
    virtual void enter_thread() = 0;
    virtual void leave_thread() = 0;
};

class Driver {
   public:
    Driver() = default;
    virtual ~Driver() = default;
    virtual Conn open(const std::string &name) = 0;
};

void register_driver(const std::string &name, std::shared_ptr<Driver> driver);

bool unregister_driver(const std::string &name);

std::shared_ptr<Driver> get_driver(const std::string &name);

}  // namespace driver
}  // namespace sqlcc

inline std::ostream &operator<<(std::ostream &os, const std::tm &tm) {
    os << std::put_time(&tm, "%FT%T%z");
    return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const sqlcc::driver::Value &value) {
    std::visit([&os](auto &&arg) { os << arg; }, value);
    return os;
}

inline std::ostream &operator<<(
    std::ostream &os, const std::vector<sqlcc::driver::Value> &values) {
    os << '[';
    for (std::size_t i = 0; i < values.size(); i++) {
        os << values[i];
        if (i < values.size() - 1) os << ", ";
    }
    os << ']';
    return os;
}
