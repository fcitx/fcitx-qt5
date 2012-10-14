#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtCore/QStringList>

#include "qfcitxplatforminputcontext.h"

class QFcitxPlatformInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPlatformInputContextFactoryInterface" FILE "fcitx.json")
    QStringList keys() const;
    QFcitxPlatformInputContext *create(const QString& system, const QStringList& paramList);
};