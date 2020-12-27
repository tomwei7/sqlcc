#include <gtest/gtest.h>

#include "plugin/mysql/driver.h"

TEST(PluginMySQLDriverOptions, ParseDSN) {
    std::string dsn = "host=192.168.1.1;port=3322;user=test;password=password;conn_num=8;io_thread=4";
    auto opt = sqlcc::plugin::mysql::Options::parse(dsn);

    EXPECT_EQ("192.168.1.1", opt.host);
    EXPECT_EQ("test", opt.user);
    EXPECT_EQ("password", opt.password);
    EXPECT_EQ(3322, opt.port);
}
