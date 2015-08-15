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

// ***************************** TransactionInfo ***************************** //

    AccountInfo::AccountInfo(const QString& hash, const QString& balance, quint64 transCount) :
        fHash(hash), fBalance(balance), fTransCount(transCount)
    {
    }

    const QVariant AccountInfo::value(const int role) const {
        switch ( role ) {
        case HashRole: return QVariant(fHash);
        case BalanceRole: return QVariant(fBalance.toDouble());
        case TransCountRole: return QVariant(fTransCount);
        case SummaryRole: return QVariant(fHash + (fBalance.toDouble() > 0 ? " [> 0 Ether]" : " [empty]") );
        }

        return QVariant();
    }

    void AccountInfo::setBalance(const QString& balance) {
        fBalance = balance;
    }

    void AccountInfo::setTransactionCount(quint64 count) {
        fTransCount = count;
    }

// ***************************** TransactionInfo ***************************** //

    TransactionInfo::TransactionInfo(const QString& sender, const QString& receiver, const QString& value, quint64 blockNumber, const QString& blockHash) :
        fSender(sender), fReceiver(receiver), fValue(value), fBlockNumber(blockNumber), fBlockHash(blockHash)
    {
    }

    const QVariant TransactionInfo::value(const int role) const {
        switch ( role ) {
            case SenderRole: return QVariant(fSender);
            case ReceiverRole: return QVariant(fReceiver);
            case ValueRole: return QVariant(fValue);
            case BlockNumberRole: return QVariant(fBlockNumber);
            case BlockHashRole: return QVariant(fBlockHash);
        }

        return QVariant();
    }

}
