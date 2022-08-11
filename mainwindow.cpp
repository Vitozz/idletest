#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QTimer>

// Screen Saver dbus services
static const QLatin1String COMMON_SS_SERV("org.freedesktop.ScreenSaver");
static const QLatin1String COMMON_SS_PATH("/ScreenSaver");
static const QLatin1String KDE_SS_SERV("org.kde.screensaver");
static const QLatin1String GNOME_SS_SERV("org.gnome.Mutter.IdleMonitor");
static const QLatin1String GNOME_SS_PATH("/org/gnome/Mutter/IdleMonitor/Core");
// Screen saver functions
static const QLatin1String GNOME_SS_F("GetIdletime");
static const QLatin1String COMMON_SS_F("GetSessionIdleTime");

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , availableService(QString())
{
    ui->setupUi(this);
    QTimer* timer = new QTimer(this);
    timer->setInterval(5000);
    connect(timer, &QTimer::timeout, this, &MainWindow::checkIdle);
    connect(ui->pushButton, &QPushButton::clicked, this, [timer]() {
        timer->start();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::getServicesAvailable()
{
    const auto services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
    const QStringList idleServices = { COMMON_SS_SERV, KDE_SS_SERV, GNOME_SS_SERV };
    // find first available dbus-service
    for (const auto& service : idleServices) {
        if (services.contains(service)) {
            availableService = service;
            return true;
        }
    }
    return false;
}

void MainWindow::checkIdle()
{
    getServicesAvailable();
    int result = 0;
    // KDE and freedesktop uses the same path interface and method but gnome uses other
    bool isNotGnome = availableService == COMMON_SS_SERV || availableService == KDE_SS_SERV;
    const QLatin1String iface = isNotGnome ? COMMON_SS_SERV : GNOME_SS_SERV;
    const QLatin1String path = isNotGnome ? COMMON_SS_PATH : GNOME_SS_PATH;
    const QLatin1String method = isNotGnome ? COMMON_SS_F : GNOME_SS_F;
    auto interface = QDBusInterface(availableService, path, iface);
    QDBusReply<uint> reply;
    if (interface.isValid())
        reply = interface.call(method);
    if (reply.isValid())
        result = isNotGnome ? reply.value() / 1000 : reply.value();
    else
        result = 0;
    ui->label->setText(QString::number(result));
}
