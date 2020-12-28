#pragma once

#include <stdexcept>

namespace sqlcc {

class Exception : public std::runtime_error {
public:
    const int get_errno() const;
};

} // namespace sqlcc
