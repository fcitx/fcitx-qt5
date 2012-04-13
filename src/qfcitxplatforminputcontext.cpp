#include <QKeyEvent>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QInputPanel>
#include <QTextCharFormat>
#include <QPalette>
#include <QWindow>
#include <QWidget>

#include <unicode/unorm.h>

#include "keyserver_x11.h"
#include "fcitx-compose-data.h"

#include "qfcitxplatforminputcontext.h"
#include "qfcitxinputcontextproxy.h"
#include "qfcitxinputmethodproxy.h"
#include "qfreedesktopdbusproxy.h"

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
    FcitxKey_Shift_L,
    FcitxKey_Shift_R,
    FcitxKey_Control_L,
    FcitxKey_Control_R,
    FcitxKey_Caps_Lock,
    FcitxKey_Shift_Lock,
    FcitxKey_Meta_L,
    FcitxKey_Meta_R,
    FcitxKey_Alt_L,
    FcitxKey_Alt_R,
    FcitxKey_Super_L,
    FcitxKey_Super_R,
    FcitxKey_Hyper_L,
    FcitxKey_Hyper_R,
    FcitxKey_Mode_switch,
    FcitxKey_ISO_Level3_Shift,
    FcitxKey_VoidSymbol
};

static bool key_filtered = false;


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
    m_connection(QDBusConnection::sessionBus()),
    m_dbusproxy(0),
    m_improxy(0),
    m_icproxy(0)
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
    return true;
}

void QFcitxPlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    if (!m_icproxy || !m_icproxy->isValid())
        return;

    if (action == QInputMethod::Click)
        commit();
}

void QFcitxPlatformInputContext::reset()
{
    QPlatformInputContext::reset();

    if (!m_icproxy || !m_icproxy->isValid())
        return;

    m_icproxy->Reset();
}

void QFcitxPlatformInputContext::update(Qt::InputMethodQueries queries )
{
    QInputMethod *method = qApp->inputMethod();
    QObject *input = method->inputItem();
    if (!input)
        return;

    QInputMethodQueryEvent query(queries);
    QGuiApplication::sendEvent(input, &query);

    if (queries & Qt::ImHints) {
        Qt::InputMethodHints hints = Qt::InputMethodHints(query.value(Qt::ImHints).toUInt());

        if (hints & Qt::ImhHiddenText)
            addCapacity(CAPACITY_PASSWORD);
        else
            removeCapacity(CAPACITY_PASSWORD);
    }
}

void QFcitxPlatformInputContext::commit()
{
    QPlatformInputContext::commit();

    if (!m_icproxy || !m_icproxy->isValid())
        return;

    QObject *input = qApp->inputMethod()->inputItem();
    if (!input) {
        return;
    }

}

void QFcitxPlatformInputContext::inputItemChanged()
{
    if (!m_icproxy || !m_icproxy->isValid())
        return;

    QObject *input = qApp->inputMethod()->inputItem();
    if (input)
        m_icproxy->FocusIn();
    else
        m_icproxy->FocusOut();
}

void QFcitxPlatformInputContext::cursorRectChanged()
{
    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if(!r.isValid())
        return;

    QWindow *inputWindow = qApp->inputMethod()->inputWindow();
    if (!inputWindow)
        return;
    r.moveTopLeft(inputWindow->mapToGlobal(r.topLeft()));
    m_icproxy->SetCursorLocation(r.x(), r.y() + r.height());
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
    QObject *input = qApp->inputMethod()->inputItem();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString(str);
    QCoreApplication::sendEvent(input, &event);
}


void QFcitxPlatformInputContext::createInputContext()
{
    m_serviceName = QString("org.fcitx.Fcitx-%1").arg(displayNumber());
    m_improxy = new QFcitxInputMethodProxy(m_serviceName,
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
        m_id = qdbus_cast<int>(result.argumentAt(0));
        m_enable = qdbus_cast<bool>(result.argumentAt(1));
        m_triggerKey[0].sym = (FcitxKeySym) qdbus_cast<uint>(result.argumentAt(2));
        m_triggerKey[0].state = qdbus_cast<uint>(result.argumentAt(3));
        m_triggerKey[1].sym = (FcitxKeySym) qdbus_cast<uint>(result.argumentAt(4));
        m_triggerKey[1].state = qdbus_cast<uint>(result.argumentAt(5));
        m_path = QString("/inputcontext_%1").arg(m_id);
        m_icproxy = new QFcitxInputContextProxy(m_serviceName, m_path, m_connection, this);
        qDebug() << m_path << m_serviceName;
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
    if (service == m_serviceName) {
        /* old die */
        if (oldowner.length() > 0 || newowner.length() > 0) {
            if (m_improxy) {
                delete m_improxy;
                m_improxy = NULL;
            }

            if (m_icproxy) {
                delete m_icproxy;
                m_icproxy = NULL;
            }
            m_enable = false;
            m_triggerKey[0].sym = m_triggerKey[1].sym = (FcitxKeySym) 0;
            m_triggerKey[0].state = m_triggerKey[1].state = 0;
        }

        /* new rise */
        if (newowner.length() > 0)
            createInputContext();
    }
}

void QFcitxPlatformInputContext::updateFormattedPreedit(const FcitxFormattedPreeditList& preeditList, int cursorPos)
{
    QObject *input = qApp->inputMethod()->inputItem();
    if (!input)
        return;
    QString str;
    int pos = 0;
    QList<QInputMethodEvent::Attribute> attrList;
    Q_FOREACH(const FcitxFormattedPreedit& preedit, preeditList)
    {
        str += preedit.string();
        QTextCharFormat format;
        if ((preedit.format() & MSG_NOUNDERLINE) == 0) {
            format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        }
        if (preedit.format() & MSG_HIGHLIGHT) {
            QBrush brush;
            QPalette palette;
            QWidget* widget = qobject_cast<QWidget*>(input);
            if (widget)
                palette = widget->palette();
            else
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
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxPlatformInputContext::updatePreedit(const QString& str, int cursorPos)
{
    QByteArray array = str.toUtf8();
    array.truncate(cursorPos);
    cursorPos = QString::fromUtf8(array).length();

    QObject *input = qApp->inputMethod()->inputItem();
    if (!input)
        return;

    QList<QInputMethodEvent::Attribute> attrList;
    QTextCharFormat format;
    format.setUnderlineStyle(QTextCharFormat::DashUnderline);
    attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, cursorPos, 1, 0));
    attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, str.length(), format));
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
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
    QObject *input = qApp->inputMethod()->inputItem();
    if (input != NULL) {
        key_filtered = true;
        QKeyEvent *keyevent = createKeyEvent(keyval, state, type);
        QCoreApplication::sendEvent(input, keyevent);
        delete keyevent;
        key_filtered = false;
    }
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

    QObject *input = qApp->inputMethod()->inputItem();

    if (!input)
        return false;

    if (!m_icproxy || !m_icproxy->isValid()) {
        return x11FilterEventFallback(keyval, keycode, state, press);
    }


    if (!m_enable) {
        FcitxKeySym fcitxsym;
        uint fcitxstate;
        FcitxHotkeyGetKey((FcitxKeySym) keyval, state, &fcitxsym, &fcitxstate);
        if (!FcitxHotkeyIsHotKey(fcitxsym, fcitxstate, m_triggerKey)) {
            return x11FilterEventFallback(keyval, keycode, state, press);
        }
    }

    m_icproxy->FocusIn();

    QDBusPendingReply< int > result = this->m_icproxy->ProcessKeyEvent(
                                          keyval,
                                          keycode,
                                          state,
                                          (press) ? FCITX_PRESS_KEY : FCITX_RELEASE_KEY,
                                          QDateTime::currentDateTime().toTime_t()
                                      );
    {
        QEventLoop loop;
        QDBusPendingCallWatcher watcher(result);
        loop.connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(quit()));
        loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);
    }

    if (result.isError() || result.value() <= 0) {
        return x11FilterEventFallback(keyval, keycode, state, press);
    } else {
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

bool QFcitxPlatformInputContext::processCompose(uint keyval, uint state, FcitxKeyEventType event)
{
    Q_UNUSED(state);
    int i;

    if (event == FCITX_RELEASE_KEY)
        return false;

    for (i = 0; fcitx_compose_ignore[i] != FcitxKey_VoidSymbol; i++) {
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

bool QFcitxPlatformInputContext::checkCompactTable(const struct _FcitxComposeTableCompact* table)
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

#define IS_DEAD_KEY(k) \
    ((k) >= FcitxKey_dead_grave && (k) <= (FcitxKey_dead_dasia+1))

bool QFcitxPlatformInputContext::checkAlgorithmically()
{
    int i;
    UChar combination_buffer[MAX_COMPOSE_LEN];

    if (m_n_compose >= MAX_COMPOSE_LEN)
        return false;

    for (i = 0; i < m_n_compose && IS_DEAD_KEY(m_compose_buffer[i]); i++);
    if (i == m_n_compose)
        return true;

    if (i > 0 && i == m_n_compose - 1) {
        combination_buffer[0] = FcitxKeySymToUnicode((FcitxKeySym) m_compose_buffer[i]);
        combination_buffer[m_n_compose] = 0;
        i--;
        while (i >= 0) {
            switch (m_compose_buffer[i]) {
#define CASE(keysym, unicode) \
case FcitxKey_dead_##keysym: combination_buffer[i + 1] = unicode; break
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
                combination_buffer[i + 1] = FcitxKeySymToUnicode((FcitxKeySym) m_compose_buffer[i]);
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
        UErrorCode state = U_ZERO_ERROR;
        UChar result[MAX_COMPOSE_LEN + 1];
        i = unorm_normalize(combination_buffer, m_n_compose, UNORM_NFC, 0, result, MAX_COMPOSE_LEN + 1, &state);

        // qDebug () << "combination_buffer = " << QString::fromUtf16(combination_buffer) << "m_n_compose" << m_n_compose;
        // qDebug () << "result = " << QString::fromUtf16(result) << "i = " << i << state;

        if (i == 1) {
            commitString(QString(QChar(result[0])));
            m_compose_buffer[0] = 0;
            m_n_compose = 0;
            return true;
        }
    }
    return false;
}

