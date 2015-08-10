#include "types.h"

namespace Etherwall {

    AccountInfo::AccountInfo(const QString& hash, const qulonglong balance):
        fHash(hash), fBalance(balance / 1000000000000000000.0) {

    }

    const QVariant AccountInfo::value(const int role) const {
        switch ( role ) {
            case HashRole: return QVariant(fHash);
            case BalanceRole: return QVariant(fBalance);
        }

        return QVariant();
    }

}
