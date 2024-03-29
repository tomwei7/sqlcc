#include "driver/mysql/stmt.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "driver/mysql/exception.h"
#include "sqlcc/exception.h"

namespace sqlcc {
namespace driver {
namespace mysql {

class SQLResult : public driver::SQLResult {
   public:
    SQLResult(int64_t last_insert_id, int64_t rows_affected);
    int64_t LastInsertID() override;
    int64_t RowsAffected() override;

   private:
    int64_t last_insert_id_;
    int64_t rows_affected_;
};

SQLResult::SQLResult(int64_t last_insert_id, int64_t rows_affected)
    : last_insert_id_(last_insert_id), rows_affected_(rows_affected) {}

int64_t SQLResult::LastInsertID() { return last_insert_id_; }

int64_t SQLResult::RowsAffected() { return rows_affected_; }

class SQLRows : public driver::SQLRows {
   public:
    SQLRows(MYSQL_STMT *stmt);
    ~SQLRows();
    virtual const std::vector<std::string> &Columns() const override;
    virtual bool Next() override;
    virtual void Scan(std::vector<Value> &dest) override;

   private:
    MYSQL_STMT *stmt_;
    MYSQL_FIELD *fields_;
    std::size_t fields_size_;
    std::vector<std::string> columns_;
    MYSQL_BIND *bind_;
};

static void AllocateResultBind(MYSQL_FIELD *field, MYSQL_BIND *bind) {
    bind->length = new uint64_t(0);
    bind->error = new my_bool(0);
    bind->is_null = new my_bool(0);
    switch (field->type) {
        case (MYSQL_TYPE_DECIMAL):
        case (MYSQL_TYPE_TINY):
        case (MYSQL_TYPE_SHORT):
        case (MYSQL_TYPE_LONG):
        case (MYSQL_TYPE_INT24):
        case (MYSQL_TYPE_LONGLONG): {
            bind->buffer_type = MYSQL_TYPE_LONGLONG;
            bind->buffer = (void *)new int64_t;
            bind->buffer_length = sizeof(int64_t);
            break;
        }
        case (MYSQL_TYPE_FLOAT):
        case (MYSQL_TYPE_DOUBLE): {
            bind->buffer_type = MYSQL_TYPE_DOUBLE;
            bind->buffer = (void *)new double;
            bind->buffer_length = sizeof(double);
            break;
        }
        default: {
            bind->buffer_type = MYSQL_TYPE_STRING;
            bind->buffer = (void *)new char[field->length];
            bind->buffer_length = field->length;
            break;
        }
    }
}

static Value NullValueFromBind(MYSQL_BIND *bind) {
    switch (bind->buffer_type) {
        case (MYSQL_TYPE_LONGLONG):
            return NullInt64();
        case (MYSQL_TYPE_DOUBLE):
            return NullDouble();
        default:
            return NullString();
    }
}

static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, int64_t &dest) {
    if (*bind->is_null) {
        throw Exception(400, "can't bind null to int64_t");
    }
    if (bind->buffer_type != MYSQL_TYPE_LONGLONG) {
        throw Exception(400, "can't bind to int64_t");
    }
    dest = *(int64_t *)(bind->buffer);
}
static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, uint64_t &dest) {
    if (*bind->is_null) {
        throw Exception(400, "can't bind null to uint64_t");
    }
    if (bind->buffer_type != MYSQL_TYPE_LONGLONG) {
        throw Exception(400, "can't bind to uint64_t");
    }
    dest = *(uint64_t *)(bind->buffer);
}
static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, double &dest) {
    if (*bind->is_null) {
        throw Exception(400, "can't bind null to double");
    }
    if (bind->buffer_type != MYSQL_TYPE_DOUBLE) {
        throw Exception(400, "can't bind to doublue");
    }
    dest = *(double *)(bind->buffer);
}
static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, std::string &dest) {
    if (*bind->is_null) {
        throw Exception(400, "can't bind null to string");
    }
    if (bind->buffer_type != MYSQL_TYPE_STRING) {
        throw Exception(400, "can't bind to string");
    }
    dest = std::string((char *)bind->buffer, *(bind->length));
}

static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, std::tm &dest) {
    if (*bind->is_null) {
        throw Exception(400, "can't bind null to tm");
    }
    if (bind->buffer_type != MYSQL_TYPE_STRING) {
        throw Exception(400, "can't bind to tm");
    }
    if (*bind->length != 19) {
        throw Exception(400, "can't bind to tm");
    }
    memset(&dest, 0, sizeof(std::tm));
    std::istringstream ss(std::string((char *)bind->buffer, *(bind->length)));
    ss >> std::get_time(&dest, "%Y-%m-%d %T");
}

template <typename T>
static void BindTo(MYSQL_FIELD *field, MYSQL_BIND *bind, T &dest) {
    bool is_null = *(bind->is_null);
    if (is_null) {
        dest = T();
        return;
    }
    return BindTo(field, bind, *dest);
}

static void BindToValue(MYSQL_FIELD *field, MYSQL_BIND *bind, Value &value) {
    std::visit([&field, &bind](auto &&value) { BindTo(field, bind, value); },
               value);
    bool is_null = *bind->is_null;
    if (is_null) {
        value = NullValueFromBind(bind);
        return;
    }
}

SQLRows::SQLRows(MYSQL_STMT *stmt)
    : stmt_(stmt), fields_(nullptr), fields_size_(0), bind_(nullptr) {
    fields_ = mariadb_stmt_fetch_fields(stmt_);
    fields_size_ = mysql_stmt_field_count(stmt_);
    for (std::size_t i = 0; i < fields_size_; i++) {
        columns_.push_back(fields_[i].name);
    }
    bind_ = new MYSQL_BIND[fields_size_];
    memset(bind_, 0, sizeof(MYSQL_BIND) * fields_size_);
    for (std::size_t i = 0; i < fields_size_; i++) {
        AllocateResultBind(&fields_[i], &bind_[i]);
    }
    int ret = mysql_stmt_bind_result(stmt_, bind_);
    if (ret != 0) {
        throw ExceptionFromStmt(stmt_);
    }
}

bool SQLRows::Next() {
    int ret = mysql_stmt_fetch(stmt_);
    if (ret != 0) {
        if (ret == MYSQL_NO_DATA) {
            return false;
        }
        throw ExceptionFromStmt(stmt_);
    }
    return true;
}

void SQLRows::Scan(std::vector<Value> &dest) {
    assert(dest.size() == fields_size_);
    for (std::size_t i = 0; i < fields_size_; i++) {
        BindToValue(&fields_[i], &bind_[i], dest[i]);
    }
}

static void free_result_bind(MYSQL_BIND *bind, std::size_t bind_size) {
    for (std::size_t i = 0; i < bind_size; i++) {
        delete bind[i].length;
        delete bind[i].error;
        delete bind[i].is_null;
        switch (bind[i].buffer_type) {
            case (MYSQL_TYPE_LONGLONG):
                delete (int64_t *)bind[i].buffer;
                break;
            case (MYSQL_TYPE_DOUBLE):
                delete (double *)bind[i].buffer;
                break;
            case (MYSQL_TYPE_STRING):
                delete (char *)bind[i].buffer;
                break;
            default:
                assert(false);
        }
    }
}

SQLRows::~SQLRows() {
    free_result_bind(bind_, fields_size_);
    delete[] bind_;
}

const std::vector<std::string> &SQLRows::Columns() const { return columns_; }

MySQLStmt::MySQLStmt(MySQLConn *conn, const std::string &query)
    : conn_(conn),
      query_(query),
      stmt_(nullptr),
      bind_(nullptr),
      bind_size_(0) {
    stmt_ = mysql_stmt_init(&conn_->mysql_);
    if (stmt_ == nullptr) {
        throw ExceptionFromMySQL(&conn_->mysql_);
    }
    int ret = mysql_stmt_prepare(stmt_, query_.c_str(), -1);
    if (ret != 0) {
        throw ExceptionFromStmt(stmt_);
    }
}

MySQLStmt::~MySQLStmt() {
    mysql_stmt_close(stmt_);
    if (bind_ != nullptr) {
        for (std::size_t i = 0; i < bind_size_; i++) {
            switch (bind_[i].buffer_type) {
                case (MYSQL_TYPE_DATETIME):
                    delete (MYSQL_TIME *)bind_[i].buffer;
                    break;
                default:;
            }
        }
        delete[] bind_;
    }
}

std::size_t MySQLStmt::NumInput() {
    return (std::size_t)mysql_stmt_param_count(stmt_);
}

MYSQL_TIME *StdTmToMySQLTm(const std::tm &tm) {
    MYSQL_TIME *mysql_tm = new MYSQL_TIME;
    memset(mysql_tm, 0, sizeof(MYSQL_TIME));

    mysql_tm->second = tm.tm_sec;
    mysql_tm->minute = tm.tm_min;
    mysql_tm->hour = tm.tm_hour;
    mysql_tm->day = tm.tm_mday;
    mysql_tm->month = tm.tm_mon + 1;
    mysql_tm->year = tm.tm_year + 1900;
    mysql_tm->time_type = MYSQL_TIMESTAMP_DATETIME;
    return mysql_tm;
}

static void BindValue(const Value &value, MYSQL_BIND *bind) {
    std::visit(
        [&bind](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                bind->buffer_type = MYSQL_TYPE_LONGLONG;
                bind->buffer = (void *)&const_cast<int64_t &>(arg);
                bind->buffer_length = sizeof(arg);
            } else if constexpr (std::is_same_v<T, uint64_t>) {
                bind->buffer_type = MYSQL_TYPE_DOUBLE;
                bind->buffer = (void *)&const_cast<uint64_t &>(arg);
                bind->buffer_length = sizeof(arg);
                bind->is_unsigned = true;
            } else if constexpr (std::is_same_v<T, double>) {
                bind->buffer_type = MYSQL_TYPE_DOUBLE;
                bind->buffer = (void *)&const_cast<double &>(arg);
                bind->buffer_length = sizeof(arg);
            } else if constexpr (std::is_same_v<T, std::string>) {
                bind->buffer_type = MYSQL_TYPE_STRING;
                bind->buffer = (void *)const_cast<std::string &>(arg).c_str();
                bind->buffer_length = arg.size();
            } else if constexpr (std::is_same_v<T, std::tm>) {
                bind->buffer_type = MYSQL_TYPE_DATETIME;
                bind->buffer = (void *)StdTmToMySQLTm(arg);
                bind->buffer_length = sizeof(MYSQL_TIME);
            } else if constexpr (std::is_same_v<T, NullInt64>) {
                bind->buffer_type = MYSQL_TYPE_LONGLONG;
                bind->is_null_value = (arg == nullptr);
                bind->buffer = (void *)&const_cast<int64_t &>(*arg);
                bind->buffer_length = sizeof(arg);
            } else if constexpr (std::is_same_v<T, NullUInt64>) {
                bind->buffer_type = MYSQL_TYPE_LONGLONG;
                bind->is_null_value = (arg == nullptr);
                bind->is_unsigned = true;
                bind->buffer = (void *)&const_cast<uint64_t &>(*arg);
                bind->buffer_length = sizeof(arg);
            } else if constexpr (std::is_same_v<T, NullDouble>) {
                bind->buffer_type = MYSQL_TYPE_DOUBLE;
                bind->is_null_value = (arg == nullptr);
                bind->buffer = (void *)&const_cast<double &>(*arg);
                bind->buffer_length = sizeof(arg);
            } else if constexpr (std::is_same_v<T, NullString>) {
                bind->buffer_type = MYSQL_TYPE_STRING;
                bind->is_null_value = (arg == nullptr);
                bind->buffer = (void *)const_cast<std::string &>(*arg).c_str();
                bind->buffer_length = (*arg).size();
            } else if constexpr (std::is_same_v<T, NullTm>) {
                bind->buffer_type = MYSQL_TYPE_DATETIME;
                bind->is_null_value = (arg == nullptr);
                bind->buffer = (void *)StdTmToMySQLTm(*arg);
                bind->buffer_length = sizeof(MYSQL_TIME);
            } else
                static_assert(always_false_v<T>, "non-exhaustive visitor!");
        },
        value);
}

static void bind_args(const std::vector<Value> &args, MYSQL_BIND *bind) {
    for (int i = 0; i < args.size(); i++) {
        BindValue(args[i], &bind[i]);
    }
}

std::shared_ptr<driver::SQLResult> MySQLStmt::Exec(
    const std::vector<Value> &args) {
    assert(args.size() == NumInput());
    if (args.size()) {
        BindValue(args);
    }

    int ret = mysql_stmt_execute(stmt_);
    if (ret != 0) {
        throw ExceptionFromStmt(stmt_);
    }
    int64_t last_insert_id = mysql_stmt_insert_id(stmt_);
    int64_t rows_affected = mysql_stmt_affected_rows(stmt_);
    return std::make_shared<SQLResult>(last_insert_id, rows_affected);
}

void MySQLStmt::BindValue(const std::vector<Value> &args) {
    assert(args.size());
    bind_ = new MYSQL_BIND[NumInput()];
    memset(bind_, 0, sizeof(MYSQL_BIND) * NumInput());
    bind_size_ = NumInput();

    bind_args(args, bind_);
    int ret = mysql_stmt_bind_param(stmt_, bind_);
    if (ret != 0) {
        throw ExceptionFromStmt(stmt_);
    }
}

std::shared_ptr<driver::SQLRows> MySQLStmt::Query(
    const std::vector<Value> &args) {
    assert(args.size() == NumInput());
    if (args.size()) {
        BindValue(args);
    }

    int ret = mysql_stmt_execute(stmt_);
    if (ret != 0) {
        throw ExceptionFromStmt(stmt_);
    }

    return std::make_shared<SQLRows>(stmt_);
}

}  // namespace mysql
}  // namespace driver
}  // namespace sqlcc
