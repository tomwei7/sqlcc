#include "sqlcc/driver/driver.h"

#include <map>
#include <stdexcept>

namespace sqlcc {
namespace driver {
namespace mysql {
    extern std::shared_ptr<Driver> create_mysql_driver();
}

static std::map<std::string, std::shared_ptr<Driver>> driver_map = {
    {"mysql", mysql::create_mysql_driver()},
};

void register_driver(const std::string& name, std::shared_ptr<Driver> driver) {
    const auto& it = driver_map.find(name);
    if (it != driver_map.end()) {
        throw std::runtime_error("sql driver: " + name + " alreay register");
    }
    driver_map[name] = driver;
}

bool unregister_driver(const std::string& name) {
    const auto& it = driver_map.find(name);
    if (it == driver_map.end()) {
        return false;
    }
    driver_map.erase(it);
    return true;
}

std::shared_ptr<Driver> get_driver(const std::string &name) {
    const auto& it = driver_map.find(name);
    if (it == driver_map.end()) {
        throw std::runtime_error("sql driver: " + name + " not exists");
    }
    return it->second;
}

}  // namespace driver
}  // namespace sqlcc
