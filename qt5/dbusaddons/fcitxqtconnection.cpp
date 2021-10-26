/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "fcitxqtconnection_p.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTimer>

#include <errno.h>
#include <signal.h>

// utils function in fcitx-utils and fcitx-config
bool _pid_exists(pid_t pid) {
    if (pid <= 0)
        return 0;
    return !(kill(pid, 0) && (errno == ESRCH));
}

FcitxQtConnection::FcitxQtConnection(QObject *parent)
    : QObject(parent), d_ptr(new FcitxQtConnectionPrivate(this)) {}

void FcitxQtConnection::startConnection() {
    Q_D(FcitxQtConnection);
    if (!d->m_initialized) {
        d->initialize();
        d->createConnection();
    }
}

void FcitxQtConnection::endConnection() {
    Q_D(FcitxQtConnection);
    d->cleanUp();
    d->finalize();
    d->m_connectedOnce = false;
}

bool FcitxQtConnection::autoReconnect() {
    Q_D(FcitxQtConnection);
    return d->m_autoReconnect;
}

void FcitxQtConnection::setAutoReconnect(bool a) {
    Q_D(FcitxQtConnection);
    d->m_autoReconnect = a;
}

QDBusConnection *FcitxQtConnection::connection() {
    Q_D(FcitxQtConnection);
    return d->m_connection;
}

const QString &FcitxQtConnection::serviceName() {
    Q_D(FcitxQtConnection);
    return d->m_serviceName;
}

bool FcitxQtConnection::isConnected() {
    Q_D(FcitxQtConnection);
    return d->isConnected();
}

FcitxQtConnection::~FcitxQtConnection() {}

FcitxQtConnectionPrivate::FcitxQtConnectionPrivate(FcitxQtConnection *conn)
    : QObject(conn), q_ptr(conn), m_displayNumber(-1),
      m_serviceName(
          QString("%1-%2").arg("org.fcitx.Fcitx").arg(displayNumber())),
      m_connection(0), m_serviceWatcher(new QDBusServiceWatcher(this)),
      m_watcher(new QFileSystemWatcher(this)), m_autoReconnect(true),
      m_connectedOnce(false), m_initialized(false) {}

FcitxQtConnectionPrivate::~FcitxQtConnectionPrivate() {
    if (m_connection)
        delete m_connection;
}

void FcitxQtConnectionPrivate::initialize() {
    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    m_serviceWatcher->addWatchedService(m_serviceName);

    QFileInfo info(socketFile());
    QDir dir(info.path());
    if (!dir.exists()) {
        QDir rt(QDir::root());
        rt.mkpath(info.path());
    }
    m_watcher->addPath(info.path());
    if (info.exists()) {
        m_watcher->addPath(info.filePath());
    }

    connect(m_watcher, &QFileSystemWatcher::fileChanged, this,
            &FcitxQtConnectionPrivate::socketFileChanged);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &FcitxQtConnectionPrivate::socketFileChanged);
    m_initialized = true;
}

void FcitxQtConnectionPrivate::finalize() {
    m_serviceWatcher->removeWatchedService(m_serviceName);
    m_watcher->removePaths(m_watcher->files());
    m_watcher->removePaths(m_watcher->directories());
    disconnect(m_watcher, &QFileSystemWatcher::fileChanged, this,
               &FcitxQtConnectionPrivate::socketFileChanged);
    disconnect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
               &FcitxQtConnectionPrivate::socketFileChanged);
    m_initialized = false;
}

void FcitxQtConnectionPrivate::socketFileChanged() {
    QFileInfo info(socketFile());
    if (info.exists()) {
        if (m_watcher->files().indexOf(info.filePath()) == -1)
            m_watcher->addPath(info.filePath());
    }

    QString addr = address();
    if (addr.isNull())
        return;

    cleanUp();
    createConnection();
}

QByteArray FcitxQtConnectionPrivate::localMachineId() {
    return QDBusConnection::localMachineId();
}

int FcitxQtConnectionPrivate::displayNumber() {
    if (m_displayNumber < 0) {
        QByteArray displayNumber("0");
        QByteArray display(qgetenv("DISPLAY"));
        int pos = display.indexOf(':');

        if (pos >= 0) {
            ++pos;
            int pos2 = display.indexOf('.', pos);
            if (pos2 > 0) {
                displayNumber = display.mid(pos, pos2 - pos);
            } else {
                displayNumber = display.mid(pos);
            }
        }

        bool ok;
        int d = displayNumber.toInt(&ok);
        if (ok) {
            m_displayNumber = d;
        } else {
            m_displayNumber = 0;
        }
    }

    return m_displayNumber;
}

const QString &FcitxQtConnectionPrivate::socketFile() {
    if (!m_socketFile.isEmpty())
        return m_socketFile;

    QString filename =
        QString("%1-%2")
            .arg(QString::fromLatin1(QDBusConnection::localMachineId()))
            .arg(displayNumber());

    QString home = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if (home.isEmpty()) {
        home = QDir::homePath().append(QLatin1Literal("/.config"));
    }
    m_socketFile = QString("%1/fcitx/dbus/%2").arg(home).arg(filename);

    return m_socketFile;
}

QString FcitxQtConnectionPrivate::address() {
    QString addr;
    QByteArray addrVar = qgetenv("FCITX_DBUS_ADDRESS");
    if (!addrVar.isNull())
        return QString::fromLocal8Bit(addrVar);

    QFile file(socketFile());
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    const int BUFSIZE = 1024;

    char buffer[BUFSIZE];
    size_t sz = file.read(buffer, BUFSIZE);
    file.close();
    if (sz == 0)
        return QString();
    char *p = buffer;
    while (*p)
        p++;
    size_t addrlen = p - buffer;
    if (sz != addrlen + 2 * sizeof(pid_t) + 1)
        return QString();

    /* skip '\0' */
    p++;
    pid_t *ppid = (pid_t *)p;
    pid_t daemonpid = ppid[0];
    pid_t fcitxpid = ppid[1];

    if (!_pid_exists(daemonpid) || !_pid_exists(fcitxpid))
        return QString();

    addr = QLatin1String(buffer);

    return addr;
}

void FcitxQtConnectionPrivate::createConnection() {
    if (m_connectedOnce && !m_autoReconnect) {
        return;
    }

    disconnect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
               this, &FcitxQtConnectionPrivate::imChanged);
    QString addr = address();
    if (!addr.isNull()) {
        QDBusConnection connection(
            QDBusConnection::connectToBus(addr, "fcitx"));
        if (connection.isConnected()) {
            // qDebug() << "create private";
            m_connection = new QDBusConnection(connection);
        } else
            QDBusConnection::disconnectFromBus("fcitx");
    }

    if (!m_connection) {
        QDBusConnection *connection =
            new QDBusConnection(QDBusConnection::sessionBus());
        connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged,
                this, &FcitxQtConnectionPrivate::imChanged);
        QDBusReply<bool> registered =
            connection->interface()->isServiceRegistered(m_serviceName);
        if (!registered.isValid() || !registered.value()) {
            delete connection;
        } else {
            m_connection = connection;
        }
    }

    Q_Q(FcitxQtConnection);
    if (m_connection) {

        m_connection->connect("org.freedesktop.DBus.Local",
                              "/org/freedesktop/DBus/Local",
                              "org.freedesktop.DBus.Local", "Disconnected",
                              this, SLOT(dbusDisconnected()));
        m_connectedOnce = true;
        Q_EMIT q->connected();
    }
}

void FcitxQtConnectionPrivate::dbusDisconnected() {
    cleanUp();

    createConnection();
}

void FcitxQtConnectionPrivate::imChanged(const QString &service,
                                         const QString &oldowner,
                                         const QString &newowner) {
    if (service == m_serviceName) {
        /* old die */
        if (oldowner.length() > 0 || newowner.length() > 0)
            cleanUp();

        /* new rise */
        if (newowner.length() > 0) {
            QTimer::singleShot(100, this, SLOT(newServiceAppear()));
        }
    }
}

void FcitxQtConnectionPrivate::cleanUp() {
    Q_Q(FcitxQtConnection);
    bool doemit = false;
    QDBusConnection::disconnectFromBus("fcitx");
    if (m_connection) {
        delete m_connection;
        m_connection = nullptr;
        doemit = true;
    }

    if (!m_autoReconnect && m_connectedOnce)
        finalize();

    /* we want m_connection and finalize being called before the signal
     * thus isConnected will return false in slot
     * and startConnection can be called in slot
     */
    if (doemit)
        Q_EMIT q->disconnected();
}

bool FcitxQtConnectionPrivate::isConnected() {
    return m_connection && m_connection->isConnected();
}

void FcitxQtConnectionPrivate::newServiceAppear() {
    if (!isConnected()) {
        cleanUp();

        createConnection();
    }
}
