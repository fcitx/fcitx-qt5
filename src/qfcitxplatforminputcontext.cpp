#include <QKeyEvent>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QInputPanel>

#include "qfcitxplatforminputcontext.h"
#include "qfcitxinputcontextproxy.h"
#include "qfcitxinputmethodproxy.h"
#include "qfreedesktopdbusproxy.h"

int QFcitxPlatformInputContext::displayNumber()
{
    QByteArray displayNumber("0");
    QByteArray display(qgetenv("DISPLAY"));
    int pos = display.indexOf(':');
    ++pos;
    int pos2 = display.indexOf('.', pos);
    if (pos2 > 0)
        displayNumber = display.mid(pos, pos2 - pos);

    bool ok;
    int d = displayNumber.toInt(&ok);
    if (ok)
        return d;
    else
        return 0;
}



QFcitxPlatformInputContext::QFcitxPlatformInputContext() :
    m_connection(QDBusConnection::sessionBus())
{
    FcitxFormattedPreedit::registerMetaType();

    memset(m_compose_buffer, 0, sizeof(uint) * (MAX_COMPOSE_LEN + 1));
    if (!m_connection.isConnected()) {
        qDebug("QFcitxPlatformInputContext: not connected.");
        return;
    }

    m_dbusproxy = new QFreedesktopDBusProxy(QLatin1String("org.freedesktop.DBus"),
                                            QLatin1String("/org/freedesktop/DBus"),
                                            m_connection,
                                            this
                                           );
    m_triggerKey[0].sym = m_triggerKey[1].sym = (FcitxKeySym) 0;
    m_triggerKey[0].state = m_triggerKey[1].state = 0;
    createInputContext();

    QInputPanel *p = qApp->inputPanel();
    connect(p, SIGNAL(inputItemChanged()), this, SLOT(inputItemChanged()));
    connect(p, SIGNAL(cursorRectangleChanged()), this, SLOT(cursorRectChanged()));
}

QFcitxPlatformInputContext::~QFcitxPlatformInputContext()
{
    delete m_dbusproxy;
    if (m_improxy)
        delete m_improxy;
    if (m_icproxy) {
        if (m_icproxy->isValid()) {
            m_icproxy->DestroyIC();
        }

        delete m_icproxy;
    }
}

bool QFcitxPlatformInputContext::isValid() const
{
    return QPlatformInputContext::isValid();
}

void QFcitxPlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    QPlatformInputContext::invokeAction(action, cursorPosition);
}

void QFcitxPlatformInputContext::reset()
{
    QPlatformInputContext::reset();
}

void QFcitxPlatformInputContext::update(Qt::InputMethodQueries quries )
{
    QPlatformInputContext::update(quries);
}

void QFcitxPlatformInputContext::commit()
{
    QPlatformInputContext::commit();
}

void QFcitxPlatformInputContext::inputItemChanged()
{

}

void QFcitxPlatformInputContext::cursorRectChanged()
{

}

void QFcitxPlatformInputContext::closeIM()
{
    this->m_enable = false;
}

void QFcitxPlatformInputContext::enableIM()
{
    this->m_enable = true;
}

void QFcitxPlatformInputContext::commitString(const QString& str)
{

}


void QFcitxPlatformInputContext::createInputContext()
{
    m_improxy = new QFcitxInputMethodProxy(QString("org.fcitx.Fcitx-%d").arg(displayNumber()),
                                            QLatin1String("/inputmethod"),
                                            m_connection,
                                            this);

    if (!m_improxy->isValid())
        return;

    char* name = fcitx_utils_get_process_name();
    QDBusPendingReply< int, bool, uint, uint, uint, uint > result = m_improxy->CreateICv2(name);
    free(name);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(result);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createInputContextFinished(QDBusPendingCallWatcher*)));
}

void QFcitxPlatformInputContext::createInputContextFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply< int, bool, uint, uint, uint, uint > result = *watcher;
    if (result.isError())
        qWarning() << result.error();
    else {
        this->m_id = qdbus_cast<int>(result.argumentAt(0));
        this->m_enable = qdbus_cast<bool>(result.argumentAt(1));
        m_triggerKey[0].sym = (FcitxKeySym) qdbus_cast<uint>(result.argumentAt(2));
        m_triggerKey[0].state = qdbus_cast<uint>(result.argumentAt(3));
        m_triggerKey[1].sym = (FcitxKeySym) qdbus_cast<uint>(result.argumentAt(4));
        m_triggerKey[1].state = qdbus_cast<uint>(result.argumentAt(5));
        this->m_path = QString("/inputcontext_%1").arg(m_id);
        m_icproxy = new QFcitxInputContextProxy(m_serviceName, m_path, m_connection, this);
        connect(m_icproxy, SIGNAL(CloseIM()), this, SLOT(closeIM()));
        connect(m_icproxy, SIGNAL(CommitString(QString)), this, SLOT(commitString(QString)));
        connect(m_icproxy, SIGNAL(EnableIM()), this, SLOT(enableIM()));
        connect(m_icproxy, SIGNAL(ForwardKey(uint, uint, int)), this, SLOT(forwardKey(uint, uint, int)));
        connect(m_icproxy, SIGNAL(UpdatePreedit(QString, int)), this, SLOT(updatePreedit(QString, int)));
        connect(m_icproxy, SIGNAL(UpdateFormattedPreedit(FcitxFormattedPreeditList,int)), this, SLOT(updateFormattedPreedit(FcitxFormattedPreeditList,int)));

        if (m_icproxy->isValid() && qApp->inputPanel()->inputItem())
            m_icproxy->FocusIn();

        QFlags<FcitxCapacityFlags> flag;
        flag |= CAPACITY_PREEDIT;
        flag |= CAPACITY_FORMATTED_PREEDIT;

        addCapacity(flag, true);
    }
    delete watcher;
}

void QFcitxPlatformInputContext::imChanged(const QString& service, const QString& oldowner, const QString& newowner)
{

}

void QFcitxPlatformInputContext::updateFormattedPreedit(const FcitxFormattedPreeditList& preeditList, int cursorPos)
{

}

void QFcitxPlatformInputContext::updatePreedit(const QString& str, int cursorPos)
{

}

void QFcitxPlatformInputContext::updateCapacity()
{
    if (!m_icproxy || !m_icproxy->isValid())
        return;

    QDBusPendingReply< void > result = m_icproxy->SetCapacity((uint) m_capacity);

    QEventLoop loop;
    QDBusPendingCallWatcher watcher(result);
    loop.connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);
}

void QFcitxPlatformInputContext::forwardKey(uint keyval, uint state, int type)
{

}
