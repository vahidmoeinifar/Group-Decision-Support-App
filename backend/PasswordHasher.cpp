#include "PasswordHasher.h"

#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QStringList>

namespace passwords {

namespace {

constexpr int kKeyLength = 32;
const QString kScheme = QStringLiteral("pbkdf2-sha256");

QByteArray derive(const QString& password, const QByteArray& salt, int iterations)
{
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), salt,
                                              iterations, kKeyLength);
}

/// Compares in constant time, so the time taken does not leak how many bytes matched.
bool equalConstantTime(const QByteArray& a, const QByteArray& b)
{
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (int i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

} // namespace

QString hash(const QString& password, int iterations)
{
    QByteArray salt(16, Qt::Uninitialized);
    QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(salt.data()), salt.size() / 4);

    return QStringList{kScheme, QString::number(iterations), QString::fromLatin1(salt.toBase64()),
                       QString::fromLatin1(derive(password, salt, iterations).toBase64())}
        .join(QLatin1Char('$'));
}

bool verify(const QString& password, const QString& stored)
{
    const QStringList parts = stored.split(QLatin1Char('$'));
    if (parts.size() != 4 || parts[0] != kScheme)
        return false;

    bool ok = false;
    const int iterations = parts[1].toInt(&ok);
    if (!ok || iterations <= 0)
        return false;

    const QByteArray salt = QByteArray::fromBase64(parts[2].toLatin1());
    const QByteArray expected = QByteArray::fromBase64(parts[3].toLatin1());
    return equalConstantTime(derive(password, salt, iterations), expected);
}

} // namespace passwords
