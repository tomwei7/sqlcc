#include "sqlcc/sqlcc.h"

namespace sqlcc {

class ConnectionImpl;
class StatementImpl;

class ResultImpl: public SQLResult {
public:
    ResultImpl(driver::Result&& result): result_(std::move(result)) {}
    ~ResultImpl() {}
    int64_t last_insert_id() override;
    int64_t rows_affected() override;
private:
    driver::Result result_;
};

int64_t ResultImpl::last_insert_id() {
    return result_->last_insert_id();
}

int64_t ResultImpl::rows_affected() {
    return result_->rows_affected();
}

class RowsImpl: public SQLRows {
public:
   public:
    RowsImpl(std::shared_ptr<StatementImpl> stmt, const std::vector<driver::Value>& args);
    ~RowsImpl() {}
    const std::vector<std::string> &columns() const override;
    bool next() override;
   protected:
    void do_scan(std::vector<driver::Value> &dest) override;
   private:
    std::shared_ptr<StatementImpl> stmt_;
    driver::Rows driver_rows_;
};

class StatementImpl: public Statement, public std::enable_shared_from_this<StatementImpl> {
public:
    StatementImpl(std::shared_ptr<ConnectionImpl> conn, const std::string& query);
    ~StatementImpl() {};
protected:
    Result do_exec(const std::vector<driver::Value>& args);
    Rows do_query(const std::vector<driver::Value>& args);
private:
    friend class RowsImpl;
    friend class DatabaseImpl;
    std::shared_ptr<ConnectionImpl> conn_;
    driver::Stmt dirver_stmt_;
};

class ConnectionImpl: public Connection, public std::enable_shared_from_this<ConnectionImpl> {
public:
    ConnectionImpl(driver::Conn&& conn): driver_conn_(std::move(conn)) {}
    ~ConnectionImpl() {}
    Stmt prepare(const std::string& query) override;
protected:
    std::shared_ptr<StatementImpl> do_prepare(const std::string& query);
private:
    friend class StatementImpl;
    friend class DatabaseImpl;
    driver::Conn driver_conn_;
};

Stmt ConnectionImpl::prepare(const std::string& query) {
    return do_prepare(query);
}

std::shared_ptr<StatementImpl> ConnectionImpl::do_prepare(const std::string& query) {
    return std::make_shared<StatementImpl>(shared_from_this(), query);
}

StatementImpl::StatementImpl(std::shared_ptr<ConnectionImpl> conn, const std::string& query): conn_(conn) {
    dirver_stmt_ = conn_->driver_conn_->prepare(query);
}

Result StatementImpl::do_exec(const std::vector<driver::Value>& args) {
    driver::Result result = dirver_stmt_->exec(args);
    return std::make_shared<ResultImpl>(std::move(result));
}

Rows StatementImpl::do_query(const std::vector<driver::Value>& args) {
    return std::make_shared<RowsImpl>(shared_from_this(), args);
}

RowsImpl::RowsImpl(std::shared_ptr<StatementImpl> stmt, const std::vector<driver::Value>& args): stmt_(stmt) {
    driver_rows_ = stmt->dirver_stmt_->query(args);
}

const std::vector<std::string>& RowsImpl::columns() const {
    return driver_rows_->columns();
}

bool RowsImpl::next() {
    return driver_rows_->next();
}

void RowsImpl::do_scan(std::vector<driver::Value> &dest) {
    return driver_rows_->scan(dest);
}

class DatabaseImpl: public Database {
public:
    DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn);
    ~DatabaseImpl() {}
    Stmt prepare(const std::string& query) override;
    Conn conn() override;
    void ping() override;
    std::shared_ptr<driver::Driver> driver() override;
protected:
    std::shared_ptr<ConnectionImpl> get_conn();
    Result do_exec(const std::string& query, const std::vector<driver::Value>& args) override;
    Rows do_query(const std::string& query, const std::vector<driver::Value>& args) override;
private:
    std::shared_ptr<driver::Driver> driver_;
    std::string dsn_;
};

DatabaseImpl::DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn): driver_(driver), dsn_(dsn) {}

Conn DatabaseImpl::conn() {
    return get_conn();
}

std::shared_ptr<ConnectionImpl> DatabaseImpl::get_conn() {
    driver::Conn conn = driver_->open(dsn_);
    return std::make_shared<ConnectionImpl>(std::move(conn));
}

Stmt DatabaseImpl::prepare(const std::string& query) {
    Conn conn = this->conn();
    return conn->prepare(query);
}

Result DatabaseImpl::do_exec(const std::string& query, const std::vector<driver::Value>& args) {
    return get_conn()->do_prepare(query)->do_exec(args);
}

Rows DatabaseImpl::do_query(const std::string& query, const std::vector<driver::Value>& args) {
    return get_conn()->do_prepare(query)->do_query(args);
}

void DatabaseImpl::ping() {
    driver_->open(dsn_);
}

std::shared_ptr<driver::Driver> DatabaseImpl::driver() {
    return driver_;
}

DB open(const std::string &driver_name, const std::string &dsn) {
    std::shared_ptr<driver::Driver> driver = driver::get_driver(driver_name);
    return std::make_shared<DatabaseImpl>(driver, dsn);
}

} // namespace sqlcc
