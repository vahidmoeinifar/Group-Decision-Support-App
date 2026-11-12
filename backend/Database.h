//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariantList>

/// Owns one SQLite connection: opens it, creates the tables and runs queries.
/// Pass ":memory:" as the path for a throwaway database (used by the tests).
class Database {
public:
    explicit Database(const QString& path, const QString& connectionName = QStringLiteral("main"));
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase connection() const;
    QString path() const { return path_; }

    /// Runs one SQL statement with positional (?) values. Throws std::runtime_error on failure.
    QSqlQuery exec(const QString& sql, const QVariantList& values = {}) const;

    /// Last id created by an INSERT on this connection.
    int lastInsertId() const;

    /// Changes whenever ANOTHER connection (e.g. a second app window) commits.
    /// Polling this is how the app notices changes made elsewhere.
    int dataVersion() const;

private:
    void createSchema();

    QString path_;
    QString name_;
};

/// Starts a transaction and rolls it back unless commit() is called (e.g. when an exception is thrown).
class Transaction {
public:
    explicit Transaction(const Database& db);
    ~Transaction();
    void commit();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

private:
    const Database& db_;
    bool done_ = false;
};
