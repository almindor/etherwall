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
/** @file settings.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Settings QML binding implementation
 */

#include "settings.h"

namespace Etherwall {

    Settings::Settings(QObject *parent)
        : QSettings(parent)
    {
    }

    bool Settings::contains(const QString& key) const
    {
        return QSettings::contains(key);
    }

    QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
    {
        return QSettings::value(key, defaultValue);
    }

    bool Settings::valueBool(const QString& key, bool defaultValue) const
    {
        return QSettings::value(key, defaultValue).toBool();
    }


    void Settings::setValue(const QString& key, const QVariant& value)
    {
        QSettings::setValue(key, value);
    }

}
