//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
#include "AuthService.h"

#include "AppError.h"
#include "Database.h"
#include "PasswordHasher.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QVariant>

bool AuthService::looksLikeEmail(const QString& email)
{
    static const QRegularExpression pattern(QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));
    return pattern.match(email).hasMatch();
}

User AuthService::registerUser(const QString& emailIn, const QString& nameIn, const QString& password)
{
    const QString email = emailIn.trimmed().toLower();
    const QString name = nameIn.trimmed();

    if (name.isEmpty())
        throw AppError::validation(QStringLiteral("Please enter your name."));
    if (!looksLikeEmail(email))
        throw AppError::validation(QStringLiteral("Please enter a valid email address."));
    if (password.size() < 8)
        throw AppError::validation(QStringLiteral("The password needs at least 8 characters."));

    if (db_.exec(QStringLiteral("SELECT 1 FROM users WHERE email = ?"), {email}).next())
        throw AppError(AppError::Kind::Conflict, QStringLiteral("An account with this email already exists."));

    Transaction tx(db_);
    db_.exec(QStringLiteral("INSERT INTO users (email, display_name, password_hash, created_at) VALUES (?, ?, ?, ?)"),
             {email, name, passwords::hash(password), QDateTime::currentDateTimeUtc().toString(Qt::ISODate)});
    const int id = db_.lastInsertId();

    // Someone may have invited this email before the account existed
    db_.exec(QStringLiteral("UPDATE experts SET user_id = ? WHERE email = ? AND user_id IS NULL"), {id, email});
    tx.commit();

    return {id, email, name};
}

User AuthService::signIn(const QString& emailIn, const QString& password) const
{
    const QString email = emailIn.trimmed().toLower();
    QSqlQuery q = db_.exec(QStringLiteral("SELECT id, email, display_name, password_hash FROM users WHERE email = ?"),
                           {email});

    // Same message for unknown email and wrong password, so nobody can probe which emails exist
    if (!q.next() || !passwords::verify(password, q.value(3).toString()))
        throw AppError(AppError::Kind::Auth, QStringLiteral("Email or password is incorrect."));

    return {q.value(0).toInt(), q.value(1).toString(), q.value(2).toString()};
}

std::optional<User> AuthService::findById(int userId) const
{
    QSqlQuery q = db_.exec(QStringLiteral("SELECT id, email, display_name FROM users WHERE id = ?"), {userId});
    if (!q.next())
        return std::nullopt;
    return User{q.value(0).toInt(), q.value(1).toString(), q.value(2).toString()};
}
