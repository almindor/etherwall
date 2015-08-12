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
/** @file types.cpp
 * @author Ales Katona <almindor@gmail.com>
 * @date 2015
 *
 * Types implementation
 */

#include "types.h"

namespace Etherwall {

    IPCError::IPCError(const QString& error, int code) :
        fError(error), fCode(code) {
    }

    const QString& IPCError::getError() const {
        return fError;
    }

    int IPCError::getCode() const {
        return fCode;
    }

    AccountInfo::AccountInfo(const QString& hash, const QString& balance, quint64 transCount):
        fHash(hash), fBalance(balance), fTransCount(transCount) {
    }

    const QVariant AccountInfo::value(const int role) const {
        switch ( role ) {
            case HashRole: return QVariant(fHash);
            case BalanceRole: return QVariant(fBalance);
            case TransCountRole: return QVariant(fTransCount);
        }

        return QVariant();
    }

}
