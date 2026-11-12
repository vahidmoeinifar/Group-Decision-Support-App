#pragma once

#include <QString>

/// Salted PBKDF2-SHA256 password hashing.
/// Stored format: "pbkdf2-sha256$<iterations>$<salt base64>$<hash base64>".
namespace passwords {

QString hash(const QString& password, int iterations = 100000);
bool verify(const QString& password, const QString& stored);

} // namespace passwords
