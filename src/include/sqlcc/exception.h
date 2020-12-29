#pragma once

#include <stdexcept>
namespace sqlcc {

class Exception : public std::exception {
public:
    Exception(int code, const char* what_arg): code(code), err_(what_arg) {}
    Exception(int code, const std::string& what_arg): code(code), err_(what_arg) {}
    const char* what() const noexcept override
    {
        return err_.what();
    }
    const int code;
private:
    std::runtime_error err_;
};

} // namespace sqlcc
