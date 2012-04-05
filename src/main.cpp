#include <QPlatformInputContext>
#include <private/qplatforminputcontextplugin_qpa_p.h>
#include <QtCore/QStringList>

#include "qfcitxplatforminputcontext.h"

class QFcitxPlatformInputContextPlugin : public QPlatformInputContextPlugin
{
public:
    QStringList keys() const;
    QFcitxPlatformInputContext *create(const QString& system, const QStringList& paramList);
};

QStringList QFcitxPlatformInputContextPlugin::keys() const
{
    return QStringList(QStringLiteral("fcitx"));

}

QFcitxPlatformInputContext *QFcitxPlatformInputContextPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    if (system.compare(system, QStringLiteral("fcitx"), Qt::CaseInsensitive) == 0)
        return new QFcitxPlatformInputContext;
    return 0;
}

Q_EXPORT_PLUGIN2(fcitxplatforminputcontextplugin, QFcitxPlatformInputContextPlugin)