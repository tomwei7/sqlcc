#include "driver/mysql/conn.h"

#include "driver/mysql/stmt.h"

#include <stdexcept>

namespace sqlcc {
namespace driver {
namespace mysql {

static void SetMySQLOptions(const Config& cfg, MYSQL* mysql) {
    int ret = 0;
    ret = mysql_optionsv(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &cfg.timeout);
    if (ret != 0) 
        throw std::invalid_argument("invalid timeout");
    ret = mysql_optionsv(mysql, MYSQL_OPT_READ_TIMEOUT, &cfg.read_timeout);
    if (ret != 0) 
        throw std::invalid_argument("invalid read_timeout");
    ret = mysql_optionsv(mysql, MYSQL_OPT_WRITE_TIMEOUT, &cfg.write_timeout);
    if (ret != 0) 
        throw std::invalid_argument("invalid write_timeout");
    ret = mysql_optionsv(mysql, MYSQL_OPT_RECONNECT, &cfg.reconnect);
    if (ret != 0) 
        throw std::invalid_argument("invalid reconnect");
    if (!cfg.charset.empty()) {
        ret = mysql_optionsv(mysql, MYSQL_SET_CHARSET_NAME, cfg.charset.c_str());
        if (ret != 0) 
            throw std::invalid_argument("invalid charset name");
    }
}

MySQLConn::MySQLConn(const Config& cfg): cfg_(cfg) {
    mysql_init(&mysql_);
    SetMySQLOptions(cfg_, &mysql_);

    const char* passwd = nullptr;
    if (!cfg.passwd.empty()) {
        passwd = cfg.passwd.c_str();
    }
    const char* dbname = nullptr;
    if (!cfg.dbname.empty()) {
        dbname = cfg.dbname.c_str();
    }
    MYSQL* mysql = mysql_real_connect(&mysql_, cfg.host.c_str(), cfg.user.c_str(), passwd, dbname, cfg_.port, nullptr, 0);
    if (mysql == nullptr) {
        throw std::runtime_error("unable to connection mysql");
    }
}

std::shared_ptr<driver::Stmt> MySQLConn::Prepare(const std::string& query) {
    return std::make_shared<MySQLStmt>(this, query);
}

std::shared_ptr<Tx> MySQLConn::Begin() {
    return nullptr;
}

void MySQLConn::EnterThread() {
    mysql_thread_init();
}

void MySQLConn::LeaveThread() {
    mysql_thread_end();
}

MySQLConn::~MySQLConn() {
    mysql_close(&mysql_);
}

} // namespace mysql
} // namespace plugin
} // namespace sqlcc
