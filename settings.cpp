#include "settings.h"

namespace Etherwall {

    Settings::Settings(QObject *parent)
        : QSettings(parent)
    {
    }

    QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
    {
        return QSettings::value(key, defaultValue);
    }

    void Settings::setValue(const QString& key, const QVariant& value)
    {
        QSettings::setValue(key, value);
    }

}
