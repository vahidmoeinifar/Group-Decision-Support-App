//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
#pragma once

#include <QString>

#include <stdexcept>

/// An error the user can understand and act on. The UI shows message() as it is.
class AppError : public std::runtime_error {
public:
    enum class Kind {
        Validation, ///< The input does not make sense (missing name, bad number, ...).
        Auth,       ///< Wrong email or password.
        Forbidden,  ///< Signed in, but this role may not do this.
        NotFound,   ///< Does not exist, or the user is not part of it.
        Conflict    ///< Clashes with existing data (e.g. email already registered).
    };

    AppError(Kind kind, const QString& message)
        : std::runtime_error(message.toStdString()), kind_(kind), message_(message) {}

    Kind kind() const { return kind_; }
    QString message() const { return message_; }

    static AppError validation(const QString& m) { return {Kind::Validation, m}; }
    static AppError forbidden(const QString& m) { return {Kind::Forbidden, m}; }
    static AppError notFound(const QString& m) { return {Kind::NotFound, m}; }

private:
    Kind kind_;
    QString message_;
};
