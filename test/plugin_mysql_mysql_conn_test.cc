#include <gtest/gtest.h>
#include <thread>

#include <chrono>

#include "plugin/mysql/mysql_conn.h"

TEST(PluginMySQLMySQLConn, PluginMySQLMySQLConn) {
    sqlcc::plugin::mysql::MySQLConnOptions opts;
    opts.user = "root";
    opts.passwd = "toor";

    sqlcc::plugin::mysql::MySQLConn conn(opts);
    std::this_thread::sleep_for(std::chrono::seconds(60));
}
