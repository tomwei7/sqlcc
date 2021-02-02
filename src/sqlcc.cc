#include "sqlcc/sqlcc.h"

#include <chrono>
#include <list>
#include <atomic>

namespace sqlcc {

class ConnectionImpl;
class StatementImpl;

class ResultImpl: public SQLResult {
public:
    ResultImpl(std::shared_ptr<driver::SQLResult> result): result_(result) {}
    ~ResultImpl() {}
    int64_t LastInsertID() override;
    int64_t RowsAffected() override;
private:
    std::shared_ptr<driver::SQLResult> result_;
};

int64_t ResultImpl::LastInsertID() {
    return result_->LastInsertID();
}

int64_t ResultImpl::RowsAffected() {
    return result_->RowsAffected();
}

class RowsImpl: public SQLRows {
public:
   public:
    RowsImpl(std::shared_ptr<StatementImpl> stmt, const std::vector<driver::Value>& args);
    ~RowsImpl() {}
    const std::vector<std::string> &Columns() const override;
    bool Next() override;
   protected:
    void DoScan(std::vector<driver::Value> &dest) override;
   private:
    std::shared_ptr<StatementImpl> stmt_;
    std::shared_ptr<driver::SQLRows> driver_rows_;
};

class StatementImpl: public Statement, public std::enable_shared_from_this<StatementImpl> {
public:
    StatementImpl(std::shared_ptr<ConnectionImpl> conn, const std::string& query);
    ~StatementImpl() {};
protected:
    Result DoExec(const std::vector<driver::Value>& args);
    Rows DoQuery(const std::vector<driver::Value>& args);
private:
    friend class RowsImpl;
    friend class DatabaseImpl;
    std::shared_ptr<ConnectionImpl> conn_;
    std::shared_ptr<driver::Stmt> dirver_stmt_;
};

class ConnectionImpl: public Connection, public std::enable_shared_from_this<ConnectionImpl> {
public:
    ConnectionImpl(std::shared_ptr<driver::Conn> conn): driver_conn_(conn) {}
    ~ConnectionImpl() {}
    Stmt Prepare(const std::string& query) override;
protected:
    std::shared_ptr<StatementImpl> DoPrepare(const std::string& query);
private:
    friend class StatementImpl;
    friend class DatabaseImpl;
    std::shared_ptr<driver::Conn> driver_conn_;
};

Stmt ConnectionImpl::Prepare(const std::string& query) {
    return DoPrepare(query);
}

std::shared_ptr<StatementImpl> ConnectionImpl::DoPrepare(const std::string& query) {
    return std::make_shared<StatementImpl>(shared_from_this(), query);
}

StatementImpl::StatementImpl(std::shared_ptr<ConnectionImpl> conn, const std::string& query): conn_(conn) {
    dirver_stmt_ = conn_->driver_conn_->Prepare(query);
}

Result StatementImpl::DoExec(const std::vector<driver::Value>& args) {
    std::shared_ptr<driver::SQLResult> result = dirver_stmt_->Exec(args);
    return std::make_shared<ResultImpl>(result);
}

Rows StatementImpl::DoQuery(const std::vector<driver::Value>& args) {
    return std::make_shared<RowsImpl>(shared_from_this(), args);
}

RowsImpl::RowsImpl(std::shared_ptr<StatementImpl> stmt, const std::vector<driver::Value>& args): stmt_(stmt) {
    driver_rows_ = stmt->dirver_stmt_->Query(args);
}

const std::vector<std::string>& RowsImpl::Columns() const {
    return driver_rows_->Columns();
}

bool RowsImpl::Next() {
    return driver_rows_->Next();
}

void RowsImpl::DoScan(std::vector<driver::Value> &dest) {
    return driver_rows_->Scan(dest);
}

class DatabaseImpl: public Database {
public:
    DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn);
    ~DatabaseImpl() {}
    Stmt Prepare(const std::string& query) override;
    std::shared_ptr<Connection> Conn() override;
    void Ping() override;
    std::shared_ptr<driver::Driver> Driver() override;
protected:
    std::shared_ptr<ConnectionImpl> GetConn();
    Result DoExec(const std::string& query, const std::vector<driver::Value>& args) override;
    Rows DoQuery(const std::string& query, const std::vector<driver::Value>& args) override;
private:
    std::shared_ptr<driver::Driver> driver_;
    std::string dsn_;
};

DatabaseImpl::DatabaseImpl(std::shared_ptr<driver::Driver> driver, const std::string& dsn): driver_(driver), dsn_(dsn) {}

std::shared_ptr<Connection> DatabaseImpl::Conn() {
    return GetConn();
}

std::shared_ptr<ConnectionImpl> DatabaseImpl::GetConn() {
    std::shared_ptr<driver::Conn> conn = driver_->Open(dsn_);
    return std::make_shared<ConnectionImpl>(conn);
}

Stmt DatabaseImpl::Prepare(const std::string& query) {
    return this->Conn()->Prepare(query);
}

Result DatabaseImpl::DoExec(const std::string& query, const std::vector<driver::Value>& args) {
    return GetConn()->DoPrepare(query)->DoExec(args);
}

Rows DatabaseImpl::DoQuery(const std::string& query, const std::vector<driver::Value>& args) {
    return GetConn()->DoPrepare(query)->DoQuery(args);
}

void DatabaseImpl::Ping() {
    driver_->Open(dsn_);
}

std::shared_ptr<driver::Driver> DatabaseImpl::Driver() {
    return driver_;
}

DB Open(const std::string &driver_name, const std::string &dsn) {
    std::shared_ptr<driver::Driver> driver = driver::GetDriver(driver_name);
    return std::make_shared<DatabaseImpl>(driver, dsn);
}

} // namespace sqlcc
