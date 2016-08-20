#ifndef CONTRACTINFO_H
#define CONTRACTINFO_H

#include <QVariant>
#include <QCryptographicHash>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "bigint.h"

#include <QDebug>

namespace Etherwall {

    class ContractArg
    {
    public:
        ContractArg(const QString& name, const QString& literal);

        int length() const; // length for arrays, -1 otherwise
        int M() const; // size M for sized types, e.g. 256 for int256 or 128 for fixed128x256
        int N() const; // size N for two-sized types, e.g. 128 for fixed128x128
        const QString type() const; // the base type e.g. int, text, byte
        const QString name() const;
        const QString toString() const;
        const QString encode(const QVariant& val, bool internal = false) const;
        static const QString encodeBytes(QByteArray bytes);
        static const QString encodeInt(int number);
        static const QString encodeInt(const BigInt::Rossi& number);
        bool dynamic() const;
    private:
        const QString encode(const QString& text) const;
        const QString encode(const QByteArray& bytes) const;
        const QString encode(int number) const;
        const QString encode(const BigInt::Rossi& val, int digits) const;
        const QString encode(bool val) const;
        QString fName;
        QString fType;
        QString fBaseType;
        int fM;
        int fN;
        int fLength;
    };

    typedef QList<ContractArg> ContractArgs;

    class ContractFunction
    {
    public:
        ContractFunction(const QJsonObject& source);

        const QString getName() const;
        const ContractArg getArgument(int index) const;
        const ContractArgs getArguments() const;
        const QString getMethodID() const;
        const QString getSignature() const;
        const QString callData(const QVariantList& params) const;
    private:
        const QString getArgLiteral(const QJsonValue& arg) const;
        const QString getArgName(const QJsonValue& arg) const;
        const QString buildSignature() const;
        QString fName;
        ContractArgs fArguments;
        ContractArgs fReturns;
        QString fSignature;
        QString fMethodID;
    };

    typedef QList<ContractFunction> ContractFunctionList;

    enum ContractRoles {
        ContractNameRole = Qt::UserRole + 1,
        AddressRole,
        ABIRole
    };

    class ContractInfo
    {
    public:
        ContractInfo(const QString& name, const QString& address, const QJsonArray& abi);
        ContractInfo(const QJsonObject& source);

        const QVariant value(const int role) const;
        const QJsonObject toJson() const;
        const QString toJsonString() const;

        const QString name() const;
        const QString address() const;
        const QString abi() const;
        const QJsonArray abiJson() const;
        const QStringList functionList() const;
        const ContractFunction function(const QString& name) const;
    private:
        void parse();

        QString fName;
        QString fAddress;
        QJsonArray fABI;
        ContractFunctionList fFunctions;
    };

    typedef QList<ContractInfo> ContractList;

}

#endif // CONTRACTINFO_H
