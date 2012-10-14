#ifndef QFCITXPLATFORMINPUTCONTEXT_H
#define QFCITXPLATFORMINPUTCONTEXT_H

#include <qpa/qplatforminputcontext.h>
#include <QDBusConnection>
#include "fcitxformattedpreedit.h"

#define MAX_COMPOSE_LEN 7

enum FcitxKeyEventType {
    FCITX_PRESS_KEY,
    FCITX_RELEASE_KEY
};

/** fcitx input context capacity flags */
enum FcitxCapacityFlags {
    CAPACITY_NONE = 0,
    CAPACITY_CLIENT_SIDE_UI = (1 << 0),
    CAPACITY_PREEDIT = (1 << 1),
    CAPACITY_CLIENT_SIDE_CONTROL_STATE =  (1 << 2),
    CAPACITY_PASSWORD = (1 << 3),
    CAPACITY_FORMATTED_PREEDIT = (1 << 4),
    CAPACITY_CLIENT_UNFOCUS_COMMIT = (1 << 5),
    CAPACITY_SURROUNDING_TEXT = (1 << 6)
};

/** message type and flags */
enum FcitxMessageType {
    MSG_TYPE_FIRST = 0,
    MSG_TYPE_LAST = 6,
    MSG_TIPS = 0,           /**< Hint Text */
    MSG_INPUT = 1,          /**< User Input */
    MSG_INDEX = 2,          /**< Index Number */
    MSG_FIRSTCAND = 3,      /**< First candidate */
    MSG_USERPHR = 4,        /**< User Phrase */
    MSG_CODE = 5,           /**< Typed character */
    MSG_OTHER = 6,          /**< Other Text */
    MSG_NOUNDERLINE = (1 << 3), /**< backward compatible, no underline is a flag */
    MSG_HIGHLIGHT = (1 << 4), /**< highlight the preedit */
    MSG_DONOT_COMMIT_WHEN_UNFOCUS = (1 << 5), /**< backward compatible */
    MSG_REGULAR_MASK = 0x7 /**< regular color type mask */
};


enum FcitxKeyState {
    FcitxKeyState_None = 0,
    FcitxKeyState_Shift = 1 << 0,
    FcitxKeyState_CapsLock = 1 << 1,
    FcitxKeyState_Ctrl = 1 << 2,
    FcitxKeyState_Alt = 1 << 3,
    FcitxKeyState_Alt_Shift = FcitxKeyState_Alt | FcitxKeyState_Shift,
    FcitxKeyState_Ctrl_Shift = FcitxKeyState_Ctrl | FcitxKeyState_Shift,
    FcitxKeyState_Ctrl_Alt = FcitxKeyState_Ctrl | FcitxKeyState_Alt,
    FcitxKeyState_Ctrl_Alt_Shift = FcitxKeyState_Ctrl | FcitxKeyState_Alt | FcitxKeyState_Shift,
    FcitxKeyState_NumLock = 1 << 4,
    FcitxKeyState_Super = 1 << 6,
    FcitxKeyState_ScrollLock = 1 << 7,
    FcitxKeyState_MousePressed = 1 << 8,
    FcitxKeyState_HandledMask = 1 << 24,
    FcitxKeyState_IgnoredMask = 1 << 25,
    FcitxKeyState_Super2    = 1 << 26,
    FcitxKeyState_Hyper    = 1 << 27,
    FcitxKeyState_Meta     = 1 << 28,
    FcitxKeyState_UsedMask = 0x5c001fff
};

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
    void deleteSurroundingText(uint start, uint offset);
    void forwardKey(uint keyval, uint state, int type);
    void createInputContextFinished(QDBusPendingCallWatcher* watcher);

private:
    static int displayNumber();
    void createInputContext();
    bool processCompose(uint keyval, uint state, FcitxKeyEventType event);
    bool checkAlgorithmically();
    bool checkCompactTable(const struct _FcitxComposeTableCompact *table);
    QKeyEvent* createKeyEvent(uint keyval, uint state, int type);
    void commitPreedit();

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
    bool m_has_focus;
    uint m_compose_buffer[MAX_COMPOSE_LEN + 1];
    int m_n_compose;
    QString m_serviceName;
    int m_cursorPos;
    bool m_useSurroundingText;
    QString m_preedit;
    QString m_commitPreedit;
    FcitxFormattedPreeditList m_preeditList;
};

#endif // QFCITXPLATFORMINPUTCONTEXT_H
