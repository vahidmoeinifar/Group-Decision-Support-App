//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
///
#include "AppController.h"

#include <QCommandLineParser>
#include <QDir>
#include <QFont>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QVariant>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("GroupDecision"));
    QGuiApplication::setApplicationName(QStringLiteral("GroupDecision"));
    QGuiApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    // Optional: --db <file> to use another database file
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Multi-criteria group decision making"));
    parser.addHelpOption();
    QCommandLineOption dbOption(QStringLiteral("db"), QStringLiteral("SQLite database file."), QStringLiteral("file"));
    parser.addOption(dbOption);
    parser.process(app);

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = parser.isSet(dbOption) ? parser.value(dbOption)
                                                  : QDir(dataDir).filePath(QStringLiteral("groupdecision.db"));
    qInfo().noquote() << "Database:" << QDir::toNativeSeparators(dbPath);

    // Default text size for every Text item in QML
    QFont font = QGuiApplication::font();
    font.setPixelSize(14);
    QGuiApplication::setFont(font);

    // "Basic" is a plain style that is fully customizable and looks the same on every platform
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller(dbPath);

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qt/qml"));
    engine.setInitialProperties({{QStringLiteral("app"), QVariant::fromValue(&controller)}});

    const QUrl url(QStringLiteral("qrc:/qt/qml/GroupDecision/qml/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return QGuiApplication::exec();
}
