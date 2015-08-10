#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

namespace Etherwall {

    class Settings: public QSettings
    {
        Q_OBJECT
    public:
        Settings(QObject *parent = 0);
        Q_INVOKABLE QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
        Q_INVOKABLE void setValue(const QString& key, const QVariant& value);
    };

}

#endif // SETTINGS_H
