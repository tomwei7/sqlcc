#pragma once

#include <iostream>
#include <utility>

#include <sqlcc/driver/driver.h>

namespace sqlcc {

template <typename... Args>
std::vector<driver::Value> merge_values(const Args &...args) {
  std::vector<driver::Value> values;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
  std::initializer_list<int>{
      ([&values](const auto &&value) { values.push_back(value); }(
           std::forward<const Args>(args)),
       0)...};
#pragma clang diagnostic pop
  return values;
}

class Database {
public:
  class Rows {};

  template <typename... Args>
  void query(const std::string &query, Args &...args) {
    std::vector<driver::Value> values = merge_values(args...);
  }

  template <typename... Args>
  void exec(const std::string &query, Args &...args);
};

} // namespace sqlcc
