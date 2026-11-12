//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#pragma once

#include "Domain.h"

#include <QObject>
#include <QtQml/qqmlregistration.h>

class AuthService;

/// The signed-in user, as seen from QML.
class Session : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Session is created by AppController.")

    Q_PROPERTY(bool signedIn READ signedIn NOTIFY changed)
    Q_PROPERTY(int userId READ userId NOTIFY changed)
    Q_PROPERTY(QString displayName READ displayName NOTIFY changed)
    Q_PROPERTY(QString email READ email NOTIFY changed)
    Q_PROPERTY(QString initials READ initials NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString demoPassword READ demoPassword CONSTANT)

public:
    Session(AuthService& auth, QObject* parent = nullptr);

    bool signedIn() const { return user_.id != 0; }
    int userId() const { return user_.id; }
    QString displayName() const { return user_.displayName; }
    QString email() const { return user_.email; }
    QString initials() const;
    QString error() const { return error_; }
    QString demoPassword() const;

    Q_INVOKABLE bool signIn(const QString& email, const QString& password);
    Q_INVOKABLE bool registerAccount(const QString& name, const QString& email, const QString& password);
    Q_INVOKABLE void signOut();
    Q_INVOKABLE void clearError();

signals:
    void changed();
    void errorChanged();

private:
    void setError(const QString& message);

    AuthService& auth_;
    User user_;
    QString error_;
};
