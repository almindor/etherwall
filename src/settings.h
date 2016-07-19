/*
    This file is part of etherwall.
    etherwall is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    etherwall is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with etherwall. If not, see <http://www.gnu.org/licenses/>.
*/
/** @file settings.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Settings QML binding header
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

namespace Etherwall {

    class Settings: public QSettings
    {
        Q_OBJECT
    public:
        Settings(QObject *parent = 0);
        Q_INVOKABLE bool contains(const QString& key) const;
        Q_INVOKABLE QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
        Q_INVOKABLE bool valueBool(const QString& key, const bool defaultValue = false) const;
        Q_INVOKABLE void setValue(const QString& key, const QVariant& value);
    };

}

#endif // SETTINGS_H
