#include "driver/mysql/stmt.h"

#include <stdexcept>
#include <cstring>

namespace sqlcc {
namespace driver {
namespace mysql {

Statement::Statement(Connection *conn, const std::string &query)
    : conn_(conn), query_(query), stmt_(nullptr) {
  stmt_ = mysql_stmt_init(&conn_->mysql_);
  if (stmt_ == nullptr) {
      throw std::runtime_error("unable to init stmt");
  }
  int ret = mysql_stmt_prepare(stmt_, query_.c_str(), -1);
  if (ret != 0) {
      throw std::runtime_error("unable to prepare stmt");
  }
}

Statement::~Statement() {
    mysql_stmt_close(stmt_);
}

std::size_t Statement::num_input() {
    return (std::size_t)mysql_stmt_param_count(stmt_);
}

template<class> inline constexpr bool always_false_v = false;

static void bind_value(const Value& value, MYSQL_BIND* bind) {
    std::visit([bind](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>) {
            bind->buffer_type = MYSQL_TYPE_LONGLONG;
            bind->buffer = (void*)&const_cast<int64_t&>(arg);
            bind->buffer_length = sizeof(arg);
        } else if constexpr(std::is_same_v<T, double>) {
            bind->buffer_type = MYSQL_TYPE_DOUBLE;
            bind->buffer = (void*)&const_cast<double&>(arg);
            bind->buffer_length = sizeof(arg);
        } else if constexpr(std::is_same_v<T, std::string>) {
            bind->buffer_type = MYSQL_TYPE_STRING;
            bind->buffer = (void*)const_cast<std::string&>(arg).c_str();
            bind->buffer_length = arg.size();
        } else 
           static_assert(always_false_v<T>, "non-exhaustive visitor!");
    }, value);
}

static void bind_args(const std::vector<Value>& args, MYSQL_BIND* bind) {
    for (int i = 0; i < args.size(); i++) {
       const auto& value = args[i]; 
       bind_value(value, &bind[i]);
    }
}

Result Statement::exec(const std::vector<Value>& args) {
    MYSQL_BIND* bind = (MYSQL_BIND*)malloc(sizeof(MYSQL_BIND)*num_input());
    memset(bind, 0, sizeof(MYSQL_BIND)*num_input());

    bind_args(args, bind);
    int ret = mysql_stmt_bind_param(stmt_, bind);
    if (ret != 0) {
        throw std::runtime_error("unable to bind param");
    }
    ret = mysql_stmt_execute(stmt_);
    if (ret != 0) {
        throw std::runtime_error("unable to execute stmt");
    }
    int64_t last_insert_id = 0;
    int64_t rows_affected = mysql_stmt_affected_rows(stmt_);
    return std::make_unique<SQLResult>(last_insert_id, rows_affected);
}


Rows Statement::query(const std::vector<Value>& args) {
    return nullptr;
}

SQLResult::SQLResult(int64_t last_insert_id, int64_t rows_affected)
    :last_insert_id_(last_insert_id), rows_affected_(rows_affected) {}

int64_t SQLResult::last_insert_id() {
    return last_insert_id_;
}

int64_t SQLResult::rows_affected() {
    return rows_affected_;
}

} // namespace mysql
} // namespace driver
} // namespace sqlcc
