#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QList>
#include <QVariant>

namespace Etherwall {

    class IPCException : public std::exception
    {
     public:
        IPCException(const QString msg);
        ~IPCException() throw();
        const char* what() const throw();

     private:
        const QString fMessage;
    };

    enum AccountRoles {
        HashRole = Qt::UserRole + 1,
        BalanceRole
    };

    class AccountInfo
    {
    public:
        AccountInfo(const QString& hash, const qulonglong balance);

        const QVariant value(const int role) const;
    private:
        QString fHash;
        double fBalance; // in ether
    };

    typedef QList<AccountInfo> AccountList;

}

#endif // TYPES_H
