#include "softphone.h"
#include "config.h"
#include "settings.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <memory>
#include <QDebug>

//TODO: QML debugging
//#include <QQmlDebuggingEnabler>
//QQmlDebuggingEnabler enabler;

int main(int argc, char *argv[])
{
    //main application
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName(ORG_NAME);
    QGuiApplication::setApplicationName(APP_NAME);
    QGuiApplication::setApplicationVersion(APP_VERSION);

    qSetMessagePattern("%{appname} [%{threadid}] [%{type}] %{message} (%{file}:%{line})");

    QQmlApplicationEngine engine;
    //set properties
    QQmlContext *context = engine.rootContext();//registered properties are available to all components
    std::unique_ptr<Softphone> softphone(new Softphone());
    if (nullptr != context) {
        qDebug() << "*** Application started ***";
        context->setContextProperty(softphone->objectName(), softphone.get());
    } else {
        qDebug() << "Cannot get root context";
        return EXIT_FAILURE;
    }

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    auto *settings = softphone->settings();
    QObject::connect(&app, &QGuiApplication::aboutToQuit, settings, &Settings::save);

    return QGuiApplication::exec();
}
