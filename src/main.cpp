#include "softphone.h"
#include "config.h"
#include "settings.h"
#include "dock_click_handler.h"
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
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName(ORG_NAME);
    QGuiApplication::setApplicationName(APP_NAME);
    QGuiApplication::setApplicationVersion(APP_VERSION);

    qSetMessagePattern("%{appname} [%{threadid}] [%{type}] %{message} (%{file}:%{line})");

    std::unique_ptr<Softphone> softphone(new Softphone());//must be created before the engine
    QQmlApplicationEngine engine;
    //set properties
    QQmlContext *context = engine.rootContext();//registered properties are available to all components
    if (nullptr != context) {
        qDebug() << "*** Application started ***";
        context->setContextProperty(softphone->objectName(), softphone.get());
    } else {
        qDebug() << "Cannot get root context";
        return EXIT_FAILURE;
    }

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    QList<QObject*> rootObjList = engine.rootObjects();
    if (!rootObjList.isEmpty() && (nullptr != rootObjList.first())) {
        QObject *rootObj = rootObjList.first();
        softphone->setMainForm(rootObj);
#if defined(Q_OS_MACOS)
        setupDockClickHandler(rootObj);
        QGuiApplication::setQuitOnLastWindowClosed(false);
#elif defined (Q_OS_WIN)
        setupTopMostWindow(rootObj);
#endif
    }

    auto *settings = softphone->settings();
    QObject::connect(&app, &QGuiApplication::aboutToQuit, settings, &Settings::save);

    return QGuiApplication::exec();
}
