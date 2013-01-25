/***************************************************************************
 *   Copyright (C) 2011~2013 by CSSlayer                                   *
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

#include <QKeyEvent>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QInputMethod>
#include <QTextCharFormat>
#include <QPalette>
#include <QWindow>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "keyserver_x11.h"
#include "fcitx-compose-data.h"

#include "qfcitxplatforminputcontext.h"
#include "fcitx-qt5/fcitxqtinputcontextproxy.h"
#include "fcitx-qt5/fcitxqtinputmethodproxy.h"
#include <fcitx-qt5/fcitxqtconnection.h>
#include "keyuni.h"
#include "utils.h"

typedef struct _FcitxComposeTableCompact FcitxComposeTableCompact;
struct _FcitxComposeTableCompact {
    const quint32 *data;
    int max_seq_len;
    int n_index_size;
    int n_index_stride;
};

static const FcitxComposeTableCompact fcitx_compose_table_compact = {
    fcitx_compose_seqs_compact,
    5,
    23,
    6
};

static const uint fcitx_compose_ignore[] = {
    XK_Shift_L,
    XK_Shift_R,
    XK_Control_L,
    XK_Control_R,
    XK_Caps_Lock,
    XK_Shift_Lock,
    XK_Meta_L,
    XK_Meta_R,
    XK_Alt_L,
    XK_Alt_R,
    XK_Super_L,
    XK_Super_R,
    XK_Hyper_L,
    XK_Hyper_R,
    XK_Mode_switch,
    XK_ISO_Level3_Shift,
    XK_VoidSymbol
};

static bool key_filtered = false;

static bool
get_boolean_env(const char *name,
                 bool defval)
{
    const char *value = getenv(name);

    if (value == NULL)
        return defval;

    if (strcmp(value, "") == 0 ||
        strcmp(value, "0") == 0 ||
        strcmp(value, "false") == 0 ||
        strcmp(value, "False") == 0 ||
        strcmp(value, "FALSE") == 0)
        return false;

    return true;
}

static int
compare_seq_index(const void *key, const void *value)
{
    const uint *keysyms = (const uint *)key;
    const quint32 *seq = (const quint32 *)value;

    if (keysyms[0] < seq[0])
        return -1;
    else if (keysyms[0] > seq[0])
        return 1;
    return 0;
}

static int
compare_seq(const void *key, const void *value)
{
    int i = 0;
    const uint *keysyms = (const uint *)key;
    const quint32 *seq = (const quint32 *)value;

    while (keysyms[i]) {
        if (keysyms[i] < seq[i])
            return -1;
        else if (keysyms[i] > seq[i])
            return 1;
        i++;
    }

    return 0;
}


QFcitxPlatformInputContext::QFcitxPlatformInputContext() :
    m_connection(new FcitxQtConnection(this)),
    m_improxy(0),
    m_n_compose(0),
    m_cursorPos(0),
    m_useSurroundingText(false),
    m_syncMode(true),
    m_lastWId(0)
{
    FcitxQtFormattedPreedit::registerMetaType();

    memset(m_compose_buffer, 0, sizeof(uint) * (MAX_COMPOSE_LEN + 1));
    connect(m_connection, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(cleanUp()));

    m_connection->startConnection();
}

QFcitxPlatformInputContext::~QFcitxPlatformInputContext()
{
    cleanUp();
}

void QFcitxPlatformInputContext::connected()
{
    if (!m_connection->isConnected())
        return;

    // qDebug() << "create Input Context" << m_connection->name();
    if (m_improxy) {
        delete m_improxy;
        m_improxy = 0;
    }
    m_improxy = new FcitxQtInputMethodProxy(m_connection->serviceName(), QLatin1String("/inputmethod"), *m_connection->connection(), this);

    QWindow* w = qApp->focusWindow();
    if (w)
        createICData(w);
}

void QFcitxPlatformInputContext::cleanUp()
{
    for(QHash<WId, FcitxQtICData *>::const_iterator i = m_icMap.constBegin(),
                                             e = m_icMap.constEnd(); i != e; ++i) {
        FcitxQtICData* data = i.value();

        if (data->proxy)
            delete data->proxy;
    }

    m_icMap.clear();

    if (m_improxy) {
        delete m_improxy;
        m_improxy = 0;
    }

    reset();
}

bool QFcitxPlatformInputContext::isValid() const
{
    return true;
}

void QFcitxPlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    if (action == QInputMethod::Click
        && (cursorPosition <= 0 || cursorPosition >= m_preedit.length())
    )
    {
        // qDebug() << action << cursorPosition;
        commitPreedit();
    }
}

void QFcitxPlatformInputContext::commitPreedit()
{
    QObject *input = qApp->focusObject();
    if (!input)
        return;
    if (m_commitPreedit.length() <= 0)
        return;
    QInputMethodEvent e;
    e.setCommitString(m_commitPreedit);
    QCoreApplication::sendEvent(input, &e);
    m_commitPreedit.clear();
}


void QFcitxPlatformInputContext::reset()
{
    commitPreedit();
    FcitxQtInputContextProxy* proxy = validIC();
    if (proxy)
        proxy->Reset();
    QPlatformInputContext::reset();
}

void QFcitxPlatformInputContext::update(Qt::InputMethodQueries queries )
{
    QWindow* window = qApp->focusWindow();
    FcitxQtInputContextProxy* proxy = validICByWindow(window);
    if (!proxy)
        return;

    FcitxQtICData* data = m_icMap.value(window->winId());

    QInputMethod *method = qApp->inputMethod();
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodQueryEvent query(queries);
    QGuiApplication::sendEvent(input, &query);

    if (queries & Qt::ImCursorRectangle) {
        cursorRectChanged();
    }

    if (queries & Qt::ImHints) {
        Qt::InputMethodHints hints = Qt::InputMethodHints(query.value(Qt::ImHints).toUInt());


#define CHECK_HINTS(_HINTS, _CAPACITY) \
    if (hints & _HINTS) \
        addCapacity(data, _CAPACITY); \
    else \
        removeCapacity(data, _CAPACITY);

        CHECK_HINTS(Qt::ImhNoAutoUppercase, CAPACITY_NOAUTOUPPERCASE)
        CHECK_HINTS(Qt::ImhPreferNumbers, CAPACITY_NUMBER)
        CHECK_HINTS(Qt::ImhPreferUppercase, CAPACITY_UPPERCASE)
        CHECK_HINTS(Qt::ImhPreferLowercase, CAPACITY_LOWERCASE)
        CHECK_HINTS(Qt::ImhNoPredictiveText, CAPACITY_NO_SPELLCHECK)
        CHECK_HINTS(Qt::ImhDigitsOnly, CAPACITY_DIGIT)
        CHECK_HINTS(Qt::ImhFormattedNumbersOnly, CAPACITY_NUMBER)
        CHECK_HINTS(Qt::ImhUppercaseOnly, CAPACITY_UPPERCASE)
        CHECK_HINTS(Qt::ImhLowercaseOnly, CAPACITY_LOWERCASE)
        CHECK_HINTS(Qt::ImhDialableCharactersOnly, CAPACITY_DIALABLE)
        CHECK_HINTS(Qt::ImhEmailCharactersOnly, CAPACITY_EMAIL)
    }

    bool setSurrounding = false;
    do {
        if (!m_useSurroundingText)
            break;
        if (!((queries & Qt::ImSurroundingText) && (queries & Qt::ImCursorPosition)))
            break;
        if (data->capacity.testFlag(CAPACITY_PASSWORD))
            break;
        QVariant var = query.value(Qt::ImSurroundingText);
        QVariant var1 = query.value(Qt::ImCursorPosition);
        QVariant var2 = query.value(Qt::ImAnchorPosition);
        if (!var.isValid() || !var1.isValid())
            break;
        QString text = var.toString();
        /* we don't want to waste too much memory here */
#define SURROUNDING_THRESHOLD 4096
        if (text.length() < SURROUNDING_THRESHOLD) {
            if (_utf8_check_string(text.toUtf8().data())) {
                addCapacity(data, CAPACITY_SURROUNDING_TEXT);

                int cursor = var1.toInt();
                int anchor;
                if (var2.isValid())
                    anchor = var2.toInt();
                else
                    anchor = cursor;
                if (data->surroundingText != text) {
                    data->surroundingText = text;
                    proxy->SetSurroundingText(text, cursor, anchor);
                }
                else {
                    if (data->surroundingAnchor != anchor ||
                        data->surroundingCursor != cursor)
                        proxy->SetSurroundingTextPosition(cursor, anchor);
                }
                data->surroundingCursor = cursor;
                data->surroundingAnchor = anchor;
                setSurrounding = true;
            }
        }
        if (!setSurrounding) {
            data->surroundingAnchor = -1;
            data->surroundingCursor = -1;
            data->surroundingText = QString::null;
            removeCapacity(data, CAPACITY_SURROUNDING_TEXT);
        }
    } while(0);
}

void QFcitxPlatformInputContext::commit()
{
    QPlatformInputContext::commit();
}

void QFcitxPlatformInputContext::setFocusObject(QObject* object)
{
    FcitxQtInputContextProxy* proxy = validICByWId(m_lastWId);
    if (proxy) {
        proxy->FocusOut();
    }

    QWindow *window = qApp->focusWindow();
    if (window) {
        m_lastWId = window->winId();
    } else {
        m_lastWId = 0;
        return;
    }
    proxy = validICByWindow(window);
    if (proxy)
        proxy->FocusIn();
    else {
        FcitxQtICData* data = m_icMap.value(window->winId());
        if (!data) {
            createICData(window);
            return;
        }
    }
}

void QFcitxPlatformInputContext::windowDestroyed(QObject* object)
{
    /* access QWindow is not possible here, so we use our own map to do so */
    WId wid = m_windowToWidMap.take(object);
    if (!wid)
        return;
    FcitxQtICData* data = m_icMap.take(wid);
    if (!data)
        return;

    delete data;
    // qDebug() << "Window Destroyed and we destroy IC correctly, horray!";
}

void QFcitxPlatformInputContext::cursorRectChanged()
{
    QWindow *inputWindow = qApp->focusWindow();
    if (!inputWindow)
        return;
    FcitxQtInputContextProxy* proxy = validICByWindow(inputWindow);
    if (!proxy)
        return;

    FcitxQtICData* data = m_icMap.value(inputWindow->winId());

    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if(!r.isValid())
        return;

    r.moveTopLeft(inputWindow->mapToGlobal(r.topLeft()));

    if (data->rect != r) {
        data->rect = r;
        proxy->SetCursorRect(r.x(), r.y(), r.width(), r.height());
    }
}

void QFcitxPlatformInputContext::createInputContext(WId w)
{
    if (!m_connection->isConnected())
        return;

    // qDebug() << "create Input Context" << m_connection->connection()->name();

    if (m_improxy) {
        delete m_improxy;
        m_improxy = NULL;
    }
    m_improxy = new FcitxQtInputMethodProxy(m_connection->serviceName(), QLatin1String("/inputmethod"), *m_connection->connection(), this);

    if (!m_improxy->isValid())
        return;

    QFileInfo info(QCoreApplication::applicationFilePath());
    QDBusPendingReply< int, bool, uint, uint, uint, uint > result = m_improxy->CreateICv3(info.fileName(), QCoreApplication::applicationPid());
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(result);
    watcher->setProperty("wid", (qulonglong) w);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createInputContextFinished(QDBusPendingCallWatcher*)));
}

void QFcitxPlatformInputContext::createInputContextFinished(QDBusPendingCallWatcher* watcher)
{
    WId w = watcher->property("wid").toULongLong();
    FcitxQtICData* data = m_icMap.value(w);
    if (!data)
        return;

    QDBusPendingReply< int, bool, uint, uint, uint, uint > result = *watcher;

    do {
        if (result.isError()) {
            break;
        }

        if (!m_connection->isConnected())
            break;

        int id = qdbus_cast<int>(result.argumentAt(0));
        QString path = QString("/inputcontext_%1").arg(id);
        if (data->proxy) {
            delete data->proxy;
        }
        data->proxy = new FcitxQtInputContextProxy(m_connection->serviceName(), path, *m_connection->connection(), this);
        connect(data->proxy, SIGNAL(CommitString(QString)), this, SLOT(commitString(QString)));
        connect(data->proxy, SIGNAL(ForwardKey(uint, uint, int)), this, SLOT(forwardKey(uint, uint, int)));
        connect(data->proxy, SIGNAL(UpdateFormattedPreedit(FcitxQtFormattedPreeditList,int)), this, SLOT(updateFormattedPreedit(FcitxQtFormattedPreeditList,int)));
        connect(data->proxy, SIGNAL(DeleteSurroundingText(int,uint)), this, SLOT(deleteSurroundingText(int,uint)));

        if (data->proxy->isValid()) {
            QWindow* window = qApp->focusWindow();
            if (window && window->winId() == w)
                data->proxy->FocusIn();
        }

        QFlags<FcitxCapacityFlags> flag;
        flag |= CAPACITY_PREEDIT;
        flag |= CAPACITY_FORMATTED_PREEDIT;
        flag |= CAPACITY_CLIENT_UNFOCUS_COMMIT;
        m_useSurroundingText = get_boolean_env("FCITX_QT_ENABLE_SURROUNDING_TEXT", true);
        if (m_useSurroundingText)
            flag |= CAPACITY_SURROUNDING_TEXT;

        /*
         * event loop will cause some problem, so we tries to use async way.
         */
        m_syncMode = get_boolean_env("FCITX_QT_USE_SYNC", false);

        addCapacity(data, flag, true);
    } while(0);
    delete watcher;
}

void QFcitxPlatformInputContext::updateCapacity(FcitxQtICData* data)
{
    if (!data->proxy || !data->proxy->isValid())
        return;

    QDBusPendingReply< void > result = data->proxy->SetCapacity((uint) data->capacity);
}

void QFcitxPlatformInputContext::commitString(const QString& str)
{
    m_cursorPos = 0;
    m_preeditList.clear();
    m_commitPreedit.clear();
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString(str);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxPlatformInputContext::updateFormattedPreedit(const FcitxQtFormattedPreeditList& preeditList, int cursorPos)
{
    QObject *input = qApp->focusObject();
    if (!input)
        return;
    if (cursorPos == m_cursorPos && preeditList == m_preeditList)
        return;
    m_preeditList = preeditList;
    m_cursorPos = cursorPos;
    QString str, commitStr;
    int pos = 0;
    QList<QInputMethodEvent::Attribute> attrList;
    Q_FOREACH(const FcitxQtFormattedPreedit& preedit, preeditList)
    {
        str += preedit.string();
        if (!(preedit.format() & MSG_DONOT_COMMIT_WHEN_UNFOCUS))
            commitStr += preedit.string();
        QTextCharFormat format;
        if ((preedit.format() & MSG_NOUNDERLINE) == 0) {
            format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        }
        if (preedit.format() & MSG_HIGHLIGHT) {
            QBrush brush;
            QPalette palette;
            palette = QGuiApplication::palette();
            format.setBackground(QBrush(QColor(palette.color(QPalette::Active, QPalette::Highlight))));
            format.setForeground(QBrush(QColor(palette.color(QPalette::Active, QPalette::HighlightedText))));
        }
        attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, pos, preedit.string().length(), format));
        pos += preedit.string().length();
    }

    QByteArray array = str.toUtf8();
    array.truncate(cursorPos);
    cursorPos = QString::fromUtf8(array).length();

    attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, cursorPos, 1, 0));
    m_preedit = str;
    m_commitPreedit = commitStr;
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
    update(Qt::ImCursorRectangle);
}

void QFcitxPlatformInputContext::deleteSurroundingText(int offset, uint nchar)
{
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString("", offset, nchar);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxPlatformInputContext::forwardKey(uint keyval, uint state, int type)
{
    QObject *input = qApp->focusObject();
    if (input != NULL) {
        key_filtered = true;
        QKeyEvent *keyevent = createKeyEvent(keyval, state, type);
        QCoreApplication::sendEvent(input, keyevent);
        delete keyevent;
        key_filtered = false;
    }
}

void QFcitxPlatformInputContext::createICData(QWindow* w)
{
    FcitxQtICData* data = m_icMap.value(w->winId());
    if (!data) {
        data = new FcitxQtICData;
        m_icMap[w->winId()] = data;
        m_windowToWidMap[w] = w->winId();
        connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed(QObject*)));
    }
    createInputContext(w->winId());
}

QKeyEvent* QFcitxPlatformInputContext::createKeyEvent(uint keyval, uint state, int type)
{
    Qt::KeyboardModifiers qstate = Qt::NoModifier;

    int count = 1;
    if (state & FcitxKeyState_Alt) {
        qstate |= Qt::AltModifier;
        count ++;
    }

    if (state & FcitxKeyState_Shift) {
        qstate |= Qt::ShiftModifier;
        count ++;
    }

    if (state & FcitxKeyState_Ctrl) {
        qstate |= Qt::ControlModifier;
        count ++;
    }

    int key;
    symToKeyQt(keyval, key);

    QKeyEvent* keyevent = new QKeyEvent(
        (type == FCITX_PRESS_KEY) ? (QEvent::KeyPress) : (QEvent::KeyRelease),
        key,
        qstate,
        QString(),
        false,
        count
    );

    return keyevent;
}

bool QFcitxPlatformInputContext::x11FilterEvent(uint keyval, uint keycode, uint state, bool press)
{
    if (key_filtered)
        return false;

    if (!inputMethodAccepted())
        return false;

    QObject *input = qApp->focusObject();

    if (!input)
        return false;

    FcitxQtInputContextProxy* proxy = validICByWindow(qApp->focusWindow());

    if (!proxy) {
        return x11FilterEventFallback(keyval, keycode, state, press);
    }

    proxy->FocusIn();

    QDBusPendingReply< int > result = proxy->ProcessKeyEvent(
                                          keyval,
                                          keycode,
                                          state,
                                          (press) ? FCITX_PRESS_KEY : FCITX_RELEASE_KEY,
                                          QDateTime::currentDateTime().toTime_t()
                                      );
    result.waitForFinished();

    if (!m_connection->isConnected() || !result.isFinished() || result.isError() || result.value() <= 0) {
        return x11FilterEventFallback(keyval, keycode, state, press);
    } else {
        update(Qt::ImCursorRectangle);
        return true;
    }
    return false;
}

bool QFcitxPlatformInputContext::x11FilterEventFallback(uint keyval, uint keycode, uint state, bool press)
{
    Q_UNUSED(keycode);
    if (processCompose(keyval, state, (press) ? FCITX_PRESS_KEY : FCITX_RELEASE_KEY)) {
        return true;
    }
    return false;
}

FcitxQtInputContextProxy* QFcitxPlatformInputContext::validIC()
{
    QWindow* window = qApp->focusWindow();
    return validICByWindow(window);
}

FcitxQtInputContextProxy* QFcitxPlatformInputContext::validICByWId(WId wid)
{
    FcitxQtICData* icData = m_icMap.value(wid);
    if (!icData)
        return 0;
    if (icData->proxy.isNull()) {
        return 0;
    } else if (icData->proxy->isValid()) {
        return icData->proxy.data();
    }
    return 0;
}

FcitxQtInputContextProxy* QFcitxPlatformInputContext::validICByWindow(QWindow* w)
{
    if (!w)
        return 0;

    return validICByWId(w->winId());
}


bool QFcitxPlatformInputContext::processCompose(uint keyval, uint state, FcitxKeyEventType event)
{
    Q_UNUSED(state);
    int i;

    if (event == FCITX_RELEASE_KEY)
        return false;

    for (i = 0; fcitx_compose_ignore[i] != XK_VoidSymbol; i++) {
        if (keyval == fcitx_compose_ignore[i])
            return false;
    }

    m_compose_buffer[m_n_compose ++] = keyval;
    m_compose_buffer[m_n_compose] = 0;

    if (checkCompactTable(&fcitx_compose_table_compact)) {
        // qDebug () << "checkCompactTable ->true";
        return true;
    }

    if (checkAlgorithmically()) {
        // qDebug () << "checkAlgorithmically ->true";
        return true;
    }

    if (m_n_compose > 1) {
        m_compose_buffer[0] = 0;
        m_n_compose = 0;
        return true;
    } else {
        m_compose_buffer[0] = 0;
        m_n_compose = 0;
        return false;
    }
}

#define IS_DEAD_KEY(k) \
    ((k) >= XK_dead_grave && (k) <= (XK_dead_dasia+1))

bool QFcitxPlatformInputContext::checkAlgorithmically()
{
    int i;
    quint32 combination_buffer[MAX_COMPOSE_LEN];

    if (m_n_compose >= MAX_COMPOSE_LEN)
        return false;

    for (i = 0; i < m_n_compose && IS_DEAD_KEY(m_compose_buffer[i]); i++);
    if (i == m_n_compose)
        return true;

    if (i > 0 && i == m_n_compose - 1) {
        combination_buffer[0] = FcitxKeySymToUnicode(m_compose_buffer[i]);
        combination_buffer[m_n_compose] = 0;
        i--;
        while (i >= 0) {
            switch (m_compose_buffer[i]) {
#define CASE(keysym, unicode) \
case XK_dead_##keysym: combination_buffer[i + 1] = unicode; break
                CASE(grave, 0x0300);
                CASE(acute, 0x0301);
                CASE(circumflex, 0x0302);
                CASE(tilde, 0x0303);  /* Also used with perispomeni, 0x342. */
                CASE(macron, 0x0304);
                CASE(breve, 0x0306);
                CASE(abovedot, 0x0307);
                CASE(diaeresis, 0x0308);
                CASE(hook, 0x0309);
                CASE(abovering, 0x030A);
                CASE(doubleacute, 0x030B);
                CASE(caron, 0x030C);
                CASE(abovecomma, 0x0313);  /* Equivalent to psili */
                CASE(abovereversedcomma, 0x0314);  /* Equivalent to dasia */
                CASE(horn, 0x031B);  /* Legacy use for psili, 0x313 (or 0x343). */
                CASE(belowdot, 0x0323);
                CASE(cedilla, 0x0327);
                CASE(ogonek, 0x0328);  /* Legacy use for dasia, 0x314.*/
                CASE(iota, 0x0345);
                CASE(voiced_sound, 0x3099);  /* Per Markus Kuhn keysyms.txt file. */
                CASE(semivoiced_sound, 0x309A);  /* Per Markus Kuhn keysyms.txt file. */
                /* The following cases are to be removed once xkeyboard-config,
                * xorg are fully updated.
                **/
                /* Workaround for typo in 1.4.x xserver-xorg */
            case 0xfe66:
                combination_buffer[i + 1] = 0x314;
                break;
                /* CASE (dasia, 0x314); */
                /* CASE (perispomeni, 0x342); */
                /* CASE (psili, 0x343); */
#undef CASE
            default:
                combination_buffer[i + 1] = FcitxKeySymToUnicode(m_compose_buffer[i]);
            }
            i--;
        }

        /* If the buffer normalizes to a single character,
        * then modify the order of combination_buffer accordingly, if necessary,
        * and return TRUE.
        **/
#if 0
        if (check_normalize_nfc(combination_buffer, m_n_compose)) {
            gunichar value;
            combination_utf8 = g_ucs4_to_utf8(combination_buffer, -1, NULL, NULL, NULL);
            nfc = g_utf8_normalize(combination_utf8, -1, G_NORMALIZE_NFC);

            value = g_utf8_get_char(nfc);
            gtk_im_context_simple_commit_char(GTK_IM_CONTEXT(context_simple), value);
            context_simple->compose_buffer[0] = 0;

            g_free(combination_utf8);
            g_free(nfc);

            return TRUE;
        }
#endif
        QString s(QString::fromUcs4(combination_buffer, m_n_compose));
        s = s.normalized(QString::NormalizationForm_C);

        // qDebug () << "combination_buffer = " << QString::fromUcs4(combination_buffer, m_n_compose) << "m_n_compose" << m_n_compose;
        // qDebug () << "result = " << s << "i = " << s.length();

        if (s.length() == 1) {
            commitString(QString(s[0]));
            m_compose_buffer[0] = 0;
            m_n_compose = 0;
            return true;
        }
    }
    return false;
}


bool
QFcitxPlatformInputContext::checkCompactTable(const FcitxComposeTableCompact *table)
{
    int row_stride;
    const quint32 *seq_index;
    const quint32 *seq;
    int i;

    /* Will never match, if the sequence in the compose buffer is longer
    * than the sequences in the table. Further, compare_seq (key, val)
    * will overrun val if key is longer than val. */
    if (m_n_compose > table->max_seq_len)
        return false;

    seq_index = (const quint32 *)bsearch(m_compose_buffer,
                                         table->data, table->n_index_size,
                                         sizeof(quint32) * table->n_index_stride,
                                         compare_seq_index);

    if (!seq_index) {
        return false;
    }

    if (seq_index && m_n_compose == 1) {
        return true;
    }

    seq = NULL;
    for (i = m_n_compose - 1; i < table->max_seq_len; i++) {
        row_stride = i + 1;

        if (seq_index[i + 1] - seq_index[i] > 0) {
            seq = (const quint32 *) bsearch(m_compose_buffer + 1,
                                            table->data + seq_index[i], (seq_index[i + 1] - seq_index[i]) / row_stride,
                                            sizeof(quint32) * row_stride,
                                            compare_seq);
            if (seq) {
                if (i == m_n_compose - 1)
                    break;
                else {
                    return true;
                }
            }
        }
    }

    if (!seq) {
        return false;
    } else {
        uint value;
        value = seq[row_stride - 1];
        commitString(QString(QChar(value)));
        m_compose_buffer[0] = 0;
        m_n_compose = 0;
        return true;
    }
    return false;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0;
