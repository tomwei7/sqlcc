#include "driver/mysql/dsn.h"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace sqlcc {
namespace driver {
namespace mysql {

Config::Config()
    : host("localhost"),
      port(3306),
      timeout(5),
      read_timeout(5),
      write_timeout(5),
      reconnect(1) {}

std::ostream& operator<<(std::ostream& os, const Config& cfg) {
    if (cfg.user.size() || cfg.passwd.size()) {
        os << cfg.user << ':' << cfg.passwd << '@';
    }
    os << "tcp(" << cfg.host << ':' << cfg.port << ")/" << cfg.dbname;
    return os;
}

static void parse_userinfo(Config* cfg, const std::string& userinfo) {
    std::size_t colon_pos = userinfo.find(':');
    if (colon_pos == userinfo.npos) {
        cfg->user = userinfo;
    } else {
        cfg->user = userinfo.substr(0, colon_pos);
        cfg->passwd = userinfo.substr(colon_pos + 1);
    }
}

static void parse_tcp_address(Config* cfg, const std::string& address) {
    std::size_t colon_pos = address.find(':');
    if (colon_pos == address.npos) {
        cfg->host = address;
        return;
    }
    cfg->host = address.substr(0, colon_pos);
    cfg->port = std::stol(address.substr(colon_pos + 1));
}

static void parse_protocol_address(Config* cfg,
                                   const std::string& protocol_addr) {
    std::size_t left_bracket_pos = protocol_addr.find('(');
    if (left_bracket_pos == protocol_addr.npos) {
        return parse_tcp_address(cfg, protocol_addr);
    }
    std::string protocol = protocol_addr.substr(0, left_bracket_pos);
    if (protocol != "tcp") {
        throw std::invalid_argument("only support tcp protocol yet");
    }
    std::size_t right_bracket_pos = protocol_addr.find(')');
    if (right_bracket_pos == protocol_addr.npos) {
        throw std::invalid_argument("invalid address");
    }
    std::size_t count = right_bracket_pos - left_bracket_pos - 1;
    return parse_protocol_address(
        cfg, protocol_addr.substr(left_bracket_pos + 1, count));
}

static void parse_dbname(Config* cfg, const std::string& dbname) {
    cfg->dbname = dbname;
}

static void parse_kv_param(Config* cfg, const std::string& kv_param) {
    std::size_t equal_pos = kv_param.find('=');
    std::string k;
    std::string v;
    if (equal_pos != kv_param.npos) {
        k = kv_param.substr(0, equal_pos);
        v = kv_param.substr(equal_pos + 1);
    } else {
        k = kv_param;
    }
    if (k == "timeout") {
        cfg->timeout = std::stol(v);
    } else if (k == "read_timeout") {
        cfg->read_timeout = std::stol(v);
    } else if (k == "write_timeout") {
        cfg->write_timeout = std::stol(v);
    } else if (k == "reconnect") {
        cfg->reconnect = std::stol(v);
    } else if (k == "charset") {
        cfg->charset = v;
    } else {
        throw std::invalid_argument("unrecognized param: " + k);
    }
}

static void parse_param(Config* cfg, const std::string& paramstr) {
    std::size_t and_pos = paramstr.npos;
    std::string unresolved = paramstr;
    do {
        and_pos = unresolved.find('&');
        parse_kv_param(cfg, unresolved.substr(0, and_pos));
        unresolved = unresolved.substr(and_pos + 1);
    } while (and_pos != unresolved.npos);
}

Config parse_dsn(const std::string& dsn) {
    Config cfg;
    std::string unresolved(dsn);

    // parse userinfo
    std::size_t at_pos = unresolved.find('@');
    if (at_pos != dsn.npos) {
        parse_userinfo(&cfg, unresolved.substr(0, at_pos));
    }
    unresolved = unresolved.substr(at_pos + 1);

    // parse protocol and address
    std::size_t slash_pos = unresolved.find('/');
    if (slash_pos == unresolved.npos) {
        parse_protocol_address(&cfg, unresolved);
        return cfg;
    }
    parse_protocol_address(&cfg, unresolved.substr(0, slash_pos));
    unresolved = unresolved.substr(slash_pos + 1);

    std::size_t question_mark_pos = unresolved.find('?');
    if (question_mark_pos == unresolved.npos) {
        parse_dbname(&cfg, unresolved);
        return cfg;
    }
    parse_dbname(&cfg, unresolved.substr(0, question_mark_pos));
    parse_param(&cfg, unresolved.substr(question_mark_pos + 1));
    return cfg;
}

}  // namespace mysql
}  // namespace driver
}  // namespace sqlcc
