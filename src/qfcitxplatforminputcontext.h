#ifndef QFCITXPLATFORMINPUTCONTEXT_H
#define QFCITXPLATFORMINPUTCONTEXT_H

#include <QPlatformInputContext>
#include <QDBusConnection>
#include <fcitx/frontend.h>
#include "fcitxformattedpreedit.h"

#define MAX_COMPOSE_LEN 7

class QKeyEvent;
class QFreedesktopDBusProxy;
class QFcitxInputMethodProxy;
class QFcitxInputContextProxy;
class QDBusPendingCallWatcher;
class QFcitxPlatformInputContext : public QPlatformInputContext
{
    Q_OBJECT
public:
    QFcitxPlatformInputContext();
    virtual ~QFcitxPlatformInputContext();

    virtual bool isValid() const;

    virtual void invokeAction(QInputMethod::Action , int cursorPosition);
    virtual void reset();
    virtual void commit();
    virtual void update(Qt::InputMethodQueries quries );

    Q_INVOKABLE bool x11FilterEvent(uint keyval, uint keycode, uint state, bool press);

public Q_SLOTS:
    void inputItemChanged();
    void cursorRectChanged();
    void imChanged(const QString& service, const QString& oldowner, const QString& newowner);
    void closeIM();
    void enableIM();
    void commitString(const QString& str);
    void updatePreedit(const QString& str, int cursorPos);
    void updateFormattedPreedit(const FcitxFormattedPreeditList& preeditList, int cursorPos);
    void forwardKey(uint keyval, uint state, int type);
    void createInputContextFinished(QDBusPendingCallWatcher* watcher);

private:
    static int displayNumber();
    void createInputContext();
    bool processCompose(uint keyval, uint state, FcitxKeyEventType event);
    bool checkAlgorithmically();
    bool checkCompactTable(const struct _FcitxComposeTableCompact *table);
#if defined(Q_WS_X11)
    bool x11FilterEventFallback(QWidget *keywidget, XEvent *event , KeySym sym);
    XEvent* createXEvent(Display* dpy, WId wid, uint keyval, uint state, int type);
#endif // Q_WS_X11
    QKeyEvent* createKeyEvent(uint keyval, uint state, int type);

    void addCapacity(QFlags<FcitxCapacityFlags> capacity, bool forceUpdage = false)
    {
        QFlags< FcitxCapacityFlags > newcaps = m_capacity | capacity;
        if (m_capacity != newcaps || forceUpdage) {
            m_capacity = newcaps;
            updateCapacity();
        }
    }

    void removeCapacity(QFlags<FcitxCapacityFlags> capacity, bool forceUpdage = false)
    {
        QFlags< FcitxCapacityFlags > newcaps = m_capacity & (~capacity);
        if (m_capacity != newcaps || forceUpdage) {
            m_capacity = newcaps;
            updateCapacity();
        }
    }

    void updateCapacity();
    
    bool x11FilterEventFallback(uint keyval, uint keycode, uint state, bool press);

    QDBusConnection m_connection;
    QFreedesktopDBusProxy* m_dbusproxy;
    QFcitxInputMethodProxy* m_improxy;
    QFcitxInputContextProxy* m_icproxy;
    QFlags<FcitxCapacityFlags> m_capacity;
    int m_id;
    QString m_path;
    bool m_enable;
    bool m_has_focus;
    FcitxHotkey m_triggerKey[2];
    uint m_compose_buffer[MAX_COMPOSE_LEN + 1];
    int m_n_compose;
    QString m_serviceName;
};

#endif // QFCITXPLATFORMINPUTCONTEXT_H