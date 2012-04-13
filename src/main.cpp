#include "main.h"


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