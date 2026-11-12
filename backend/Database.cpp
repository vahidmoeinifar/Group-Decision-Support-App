#include "Database.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>

#include <stdexcept>

namespace {

// Each statement is run on its own: the SQLite driver runs only one statement per exec().
const char* const kSchema[] = {
    R"(CREATE TABLE IF NOT EXISTS users (
        id            INTEGER PRIMARY KEY AUTOINCREMENT,
        email         TEXT    NOT NULL UNIQUE COLLATE NOCASE,
        display_name  TEXT    NOT NULL,
        password_hash TEXT    NOT NULL,
        created_at    TEXT    NOT NULL))",

    R"(CREATE TABLE IF NOT EXISTS decisions (
        id               INTEGER PRIMARY KEY AUTOINCREMENT,
        title            TEXT    NOT NULL,
        description      TEXT    NOT NULL DEFAULT '',
        owner_id         INTEGER NOT NULL REFERENCES users(id),
        current_round    INTEGER NOT NULL DEFAULT 1,
        consensus_target REAL    NOT NULL DEFAULT 0.8,
        version          INTEGER NOT NULL DEFAULT 0,
        last_change      TEXT    NOT NULL DEFAULT '',
        created_at       TEXT    NOT NULL))",

    R"(CREATE TABLE IF NOT EXISTS options (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        decision_id INTEGER NOT NULL REFERENCES decisions(id) ON DELETE CASCADE,
        name        TEXT    NOT NULL,
        position    INTEGER NOT NULL))",

    R"(CREATE TABLE IF NOT EXISTS criteria (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        decision_id INTEGER NOT NULL REFERENCES decisions(id) ON DELETE CASCADE,
        name        TEXT    NOT NULL,
        weight      REAL    NOT NULL,
        type        INTEGER NOT NULL,          -- 0 = benefit, 1 = cost
        position    INTEGER NOT NULL))",

    R"(CREATE TABLE IF NOT EXISTS experts (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        decision_id INTEGER NOT NULL REFERENCES decisions(id) ON DELETE CASCADE,
        name        TEXT    NOT NULL,
        email       TEXT    NOT NULL COLLATE NOCASE,
        user_id     INTEGER REFERENCES users(id) ON DELETE SET NULL,
        weight      REAL    NOT NULL,
        position    INTEGER NOT NULL))",

    R"(CREATE TABLE IF NOT EXISTS evaluations (
        id           INTEGER PRIMARY KEY AUTOINCREMENT,
        expert_id    INTEGER NOT NULL REFERENCES experts(id)  ON DELETE CASCADE,
        option_id    INTEGER NOT NULL REFERENCES options(id)  ON DELETE CASCADE,
        criterion_id INTEGER NOT NULL REFERENCES criteria(id) ON DELETE CASCADE,
        round        INTEGER NOT NULL,
        score        REAL    NOT NULL,
        UNIQUE (expert_id, option_id, criterion_id, round)))",

    "CREATE INDEX IF NOT EXISTS ix_decisions_owner ON decisions(owner_id)",
    "CREATE INDEX IF NOT EXISTS ix_experts_user ON experts(user_id)",
    "CREATE INDEX IF NOT EXISTS ix_experts_email ON experts(email)",
    "CREATE INDEX IF NOT EXISTS ix_evaluations_expert_round ON evaluations(expert_id, round)",
};

} // namespace

Database::Database(const QString& path, const QString& connectionName)
    : path_(path), name_(connectionName)
{
    if (path != QLatin1String(":memory:"))
        QDir().mkpath(QFileInfo(path).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name_);
    db.setDatabaseName(path);
    if (!db.open())
        throw std::runtime_error("Could not open the database: " + db.lastError().text().toStdString());

    exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    exec(QStringLiteral("PRAGMA busy_timeout = 3000")); // wait if another window is writing
    if (path != QLatin1String(":memory:"))
        exec(QStringLiteral("PRAGMA journal_mode = WAL")); // readers do not block the writer
    createSchema();
}

Database::~Database()
{
    {
        QSqlDatabase db = QSqlDatabase::database(name_, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(name_);
}

QSqlDatabase Database::connection() const
{
    return QSqlDatabase::database(name_, false);
}

QSqlQuery Database::exec(const QString& sql, const QVariantList& values) const
{
    QSqlQuery query(connection());
    if (!query.prepare(sql))
        throw std::runtime_error("SQL error: " + query.lastError().text().toStdString());
    for (const QVariant& v : values)
        query.addBindValue(v);
    if (!query.exec())
        throw std::runtime_error("SQL error: " + query.lastError().text().toStdString());
    return query;
}

int Database::lastInsertId() const
{
    QSqlQuery q = exec(QStringLiteral("SELECT last_insert_rowid()"));
    return q.next() ? q.value(0).toInt() : 0;
}

int Database::dataVersion() const
{
    QSqlQuery q = exec(QStringLiteral("PRAGMA data_version"));
    return q.next() ? q.value(0).toInt() : 0;
}

void Database::createSchema()
{
    for (const char* statement : kSchema)
        exec(QString::fromUtf8(statement));
    exec(QStringLiteral("PRAGMA user_version = 1"));
}

Transaction::Transaction(const Database& db) : db_(db)
{
    if (!db_.connection().transaction())
        throw std::runtime_error("Could not start a transaction.");
}

Transaction::~Transaction()
{
    if (!done_)
        db_.connection().rollback();
}

void Transaction::commit()
{
    if (!db_.connection().commit())
        throw std::runtime_error("Could not save the changes.");
    done_ = true;
}
