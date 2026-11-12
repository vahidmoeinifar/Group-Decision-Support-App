//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#pragma once

#include "DecisionController.h"
#include "DecisionListModel.h"
#include "Session.h"

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

#include <memory>

class Database;
class AuthService;
class DecisionService;

/// Builds and owns everything the UI needs, and watches the database for
/// changes made by other windows (live updates).
class AppController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("AppController is created in main().")

    Q_PROPERTY(Session* session READ session CONSTANT)
    Q_PROPERTY(DecisionListModel* decisions READ decisions CONSTANT)
    Q_PROPERTY(DecisionController* current READ current CONSTANT)
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)

public:
    explicit AppController(const QString& databasePath, QObject* parent = nullptr);
    ~AppController() override;

    Session* session() const { return session_; }
    DecisionListModel* decisions() const { return decisions_; }
    DecisionController* current() const { return current_; }
    QString databasePath() const;
    bool live() const { return timer_.isActive(); }

signals:
    void notify(const QString& message, const QString& kind);
    void liveChanged();

private:
    void onSessionChanged();
    void pollDatabase();

    std::unique_ptr<Database> db_;
    std::unique_ptr<AuthService> auth_;
    std::unique_ptr<DecisionService> service_;
    Session* session_ = nullptr;
    DecisionListModel* decisions_ = nullptr;
    DecisionController* current_ = nullptr;
    QTimer timer_;
    int dataVersion_ = 0;
};
