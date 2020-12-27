#pragma once

#include <string>

namespace sqlcc {
namespace driver {
namespace mysql {

struct Config {
    std::string user;
    std::string passwd;
    std::string host;
    int32_t port;
    std::string dbname;
    int32_t timeout;
    int32_t read_timeout;
    int32_t write_timeout;
    int32_t reconnect;
    std::string charset;
    Config();
};

std::ostream& operator<<(std::ostream& os, const Config& cfg);

// [username[:password]@][protocol[(address)]]/dbname[?param1=value1&...&paramN=valueN]
Config parse_dsn(const std::string& dsn);

} // namespace mysql
} // namespace driver
} // namespace sqlcc
