//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
///
#pragma once

#include "Domain.h"

#include <optional>

class Database;

/// Accounts: register, sign in, look up. Passwords are stored only as salted hashes.
class AuthService {
public:
    explicit AuthService(const Database& db) : db_(db) {}

    /// Creates an account and links any invitations sent to this email. Throws AppError.
    User registerUser(const QString& email, const QString& displayName, const QString& password);

    /// Throws AppError (Auth) when the email or password is wrong.
    User signIn(const QString& email, const QString& password) const;

    std::optional<User> findById(int userId) const;

    static bool looksLikeEmail(const QString& email);

private:
    const Database& db_;
};
