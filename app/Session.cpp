//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#include "Session.h"

#include "AppError.h"
#include "AuthService.h"
#include "SeedData.h"

Session::Session(AuthService& auth, QObject* parent) : QObject(parent), auth_(auth) {}

QString Session::initials() const
{
    QString result;
    for (const QString& part : user_.displayName.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        result += part.front().toUpper();
        if (result.size() == 2)
            break;
    }
    return result;
}

QString Session::demoPassword() const
{
    return QString::fromLatin1(seed::kDemoPassword);
}

bool Session::signIn(const QString& email, const QString& password)
{
    try {
        user_ = auth_.signIn(email, password);
        setError({});
        emit changed();
        return true;
    } catch (const AppError& e) {
        setError(e.message());
        return false;
    }
}

bool Session::registerAccount(const QString& name, const QString& email, const QString& password)
{
    try {
        user_ = auth_.registerUser(email, name, password);
        setError({});
        emit changed();
        return true;
    } catch (const AppError& e) {
        setError(e.message());
        return false;
    }
}

void Session::signOut()
{
    user_ = {};
    setError({});
    emit changed();
}

void Session::clearError()
{
    setError({});
}

void Session::setError(const QString& message)
{
    if (error_ == message)
        return;
    error_ = message;
    emit errorChanged();
}
