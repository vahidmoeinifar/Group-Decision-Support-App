//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#include "AppController.h"

#include "AuthService.h"
#include "Database.h"
#include "DecisionController.h"
#include "DecisionListModel.h"
#include "DecisionService.h"
#include "SeedData.h"
#include "Session.h"

#include <QDebug>

AppController::AppController(const QString& databasePath, QObject* parent)
    : QObject(parent),
      db_(std::make_unique<Database>(databasePath)),
      auth_(std::make_unique<AuthService>(*db_)),
      service_(std::make_unique<DecisionService>(*db_))
{
    if (seed::addDemoData(*db_))
        qInfo() << "Added demo accounts and an example decision.";

    // QObject children: Qt deletes them together with this object
    session_ = new Session(*auth_, this);
    decisions_ = new DecisionListModel(*service_, this);
    current_ = new DecisionController(*service_, *session_, this);

    connect(session_, &Session::changed, this, &AppController::onSessionChanged);
    connect(current_, &DecisionController::notify, this, &AppController::notify);
    connect(current_, &DecisionController::listChanged, this, [this] { decisions_->refresh(session_->userId()); });

    // Live updates: every second, ask SQLite whether another connection committed something
    dataVersion_ = db_->dataVersion();
    timer_.setInterval(1000);
    connect(&timer_, &QTimer::timeout, this, &AppController::pollDatabase);
}

AppController::~AppController()
{
    // The QObject children use the services, so delete them before the services go
    timer_.stop();
    delete current_;
    delete decisions_;
    delete session_;
}

QString AppController::databasePath() const
{
    return db_->path();
}

void AppController::onSessionChanged()
{
    if (session_->signedIn()) {
        decisions_->refresh(session_->userId());
        if (decisions_->count() > 0)
            current_->open(decisions_->idAt(0));
        dataVersion_ = db_->dataVersion();
        timer_.start();
    } else {
        timer_.stop();
        current_->close();
        decisions_->refresh(0);
    }
    emit liveChanged();
}

void AppController::pollDatabase()
{
    const int version = db_->dataVersion();
    if (version == dataVersion_)
        return;
    dataVersion_ = version;

    decisions_->refresh(session_->userId());
    current_->checkForOutsideChange();
}
