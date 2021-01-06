#include "sqlcc/sqlcc.h"

namespace sqlcc {

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

class StatementImpl: public Statement {
public:
    StatementImpl(driver::Stmt&& stmt): stmt_(std::move(stmt)) {}
    ~StatementImpl() {};
    Result exec(const std::vector<driver::Value>& args);
private:
    driver::Stmt stmt_;
};

Result StatementImpl::exec(const std::vector<driver::Value>& args) {
    driver::Result result = stmt_->exec(args);
    return std::make_shared<ResultImpl>(std::move(result));
}

class ConnectionImpl: public Connection {
public:
    ConnectionImpl(driver::Conn&& conn): conn_(std::move(conn)) {}
    ~ConnectionImpl() {}
    Stmt prepare(const std::string& query) override;
private:
    driver::Conn conn_;
};

Stmt ConnectionImpl::prepare(const std::string& query) {
    driver::Stmt stmt = conn_->prepare(query);
    return std::make_shared<StatementImpl>(std::move(stmt));
}

class DatabaseImpl: public Database {
public:
    DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn);
    ~DatabaseImpl() {}
    Conn conn() override;
    void ping() override;
    std::shared_ptr<driver::Driver> driver() override;
protected:
    Result do_exec(const std::string& query, const std::vector<driver::Value>& args) override;
private:
    std::shared_ptr<driver::Driver> driver_;
    std::string dsn_;
};

DatabaseImpl::DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn): driver_(driver), dsn_(dsn) {}

Conn DatabaseImpl::conn() {
    driver::Conn conn = driver_->open(dsn_);
    return std::make_shared<ConnectionImpl>(std::move(conn));
}

Result DatabaseImpl::do_exec(const std::string& query, const std::vector<driver::Value>& args) {
    Conn conn = this->conn();
    Stmt stmt = conn->prepare(query);
    return stmt->exec(args);
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
