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

template <class> inline constexpr bool always_false_v = false;

using Value = std::variant<int64_t, double, std::string, std::tm>;

using Conn = std::unique_ptr<Connection>;
using Stmt = std::unique_ptr<Statement>;
using Tx = std::unique_ptr<Transaction>;
using Result = std::unique_ptr<SQLResult>;
using Rows = std::unique_ptr<SQLRows>;

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

} // namespace driver
} // namespace sqlcc
