#include <gtest/gtest.h>

#include "sqlcc/sqlcc.h"

namespace sqlcc {

TEST(ValueBind, ValueBind) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::vector<driver::Value> values = merge_values(1, 2, 3, "test", tm);
    std::cerr << "values: " << values << std::endl;
}

} // namespace sqlcc
