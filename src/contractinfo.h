#ifndef CONTRACTINFO_H
#define CONTRACTINFO_H

#include <QVariant>
#include <QCryptographicHash>
#include <QAbstractListModel>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "bigint.h"

#include <QDebug>

namespace Etherwall {

    enum FilterRoles {
        FilterNameRole = Qt::UserRole + 1,
        FilterAddressRole,
        FilterContractRole,
        FilterTopicsRole,
        FilterActiveRole
    };

    class FilterInfo
    {
    public:
        FilterInfo(const QString& name, const QString& address, const QString& contract, const QStringList& topics, bool active);
        FilterInfo(const QJsonObject& source);

        const QVariant value(const int role) const;
        const QJsonObject toJson() const;
        const QString toJsonString() const;
        const QString getHandle() const;
        void setActive(bool active);
    private:
        QString fName;
        QString fAddress;
        QString fContract;
        QStringList fTopics;
        bool fActive;
    };

    typedef QList<FilterInfo> EventFilters;

    enum ArgumentRoles {
        ArgNameRole = Qt::UserRole + 1,
        ArgTypeRole,
        ValRexRole
    };

    class ContractArg
    {
    public:
        ContractArg(const QString& name, const QString& literal, bool indexed = false);

        int length() const; // length for arrays, -1 otherwise
        int M() const; // size M for sized types, e.g. 256 for int256 or 128 for fixed128x256
        int N() const; // size N for two-sized types, e.g. 128 for fixed128x128
        const QString type() const; // the base type e.g. int, text, byte
        const QString name() const;
        bool indexed() const;
        const QString toString() const;
        const QVariantMap toVariantMap() const;
        const QString encode(const QVariant& val, bool inArray = false) const;
        static const QString encodeBytes(QByteArray bytes, int fixedSize = 0);
        static const QString encodeInt(int number);
        static const QString encodeInt(const BigInt::Rossi& number);
        bool dynamic() const;
        const QVariant decode(const QString& data, bool inArray = false) const;
        static const BigInt::Rossi decodeInt(const QString& data, bool isSigned);
    private:
        const QString encode(const QString& text) const;
        const QString encode(const QByteArray& bytes) const;
        const QString encode(int number) const;
        const QString encode(const BigInt::Rossi& val, int digits) const;
        const QString encode(bool val) const;
        const QRegExp getValRex() const;
        const QString getPlaceholder() const;
        QString fName;
        QString fType;
        QString fBaseType;
        int fM;
        int fN;
        int fLength;
        bool fIndexed;
    };

    typedef QList<ContractArg> ContractArgs;

    // includes both an event and a function of a contract
    class ContractCallable
    {
    public:
        ContractCallable(const QJsonObject& source);

        const QString getName() const;
        const ContractArg getArgument(int index) const;
        const ContractArgs getArguments() const;
        const QString getMethodID() const;
        const QString getSignature() const;
    protected:
        const QString getArgLiteral(const QJsonValue& arg) const;
        const QString getArgName(const QJsonValue& arg) const;
        bool getArgIndexed(const QJsonValue& arg) const;
        const QString buildSignature() const;
        QString fName;
        QString fSignature;
        ContractArgs fArguments;
        ContractArgs fReturns;
        QString fMethodID;
    };

    class ContractEvent : public ContractCallable
    {
    public:
        ContractEvent(const QJsonObject& source);
    };


    typedef QList<ContractEvent> ContractEventList;

    class ContractFunction : public ContractCallable
    {
    public:
        ContractFunction(const QJsonObject& source);

        const QVariantList getArgModel() const;
        const QString callData(const QVariantList& params) const;
    private:
        QVariantList fArgModel;
    };

    typedef QList<ContractFunction> ContractFunctionList;

    enum EventRoles {
        EventNameRole = Qt::UserRole + 1,
        EventAddressRole,
        EventContractRole,
        EventDataRole,
        EventBlockNumberRole,
        EventTransactionHashRole,
        EventBlockHashRole,
        EventTopicsRole
    };

    class ContractInfo;

    class EventInfo
    {
    public:
        EventInfo(const QJsonObject& source);

        void fillContract(const ContractInfo& contract);
        void fillParams(const ContractInfo& contract, const ContractEvent& event);
        const QString address() const;
        const QString contract() const;
        const QString signature() const;
        const QString transactionHash() const;
        const QString getMethodID() const;
        const QVariant value(const int role) const;
        const ContractArgs getArguments() const;
        const QVariantList getParams() const;
        const QString paramToStr(const QVariant& value) const;
        quint64 blockNumber() const;
    private:
        QString fName;
        QString fContract;
        QString fData;
        QString fAddress;
        quint64 fBlockNumber;
        QString fTransactionHash;
        QString fBlockHash;
        QString fMethodID;
        QStringList fTopics;
        ContractArgs fArguments;
        QVariantList fParams;
    };

    typedef QList<EventInfo> EventList;

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
        void processEvent(EventInfo& info) const;
    private:
        void parse();

        QString fName;
        QString fAddress;
        QJsonArray fABI;
        ContractFunctionList fFunctions;
        ContractEventList fEvents;
    };

    typedef QList<ContractInfo> ContractList;

}

#endif // CONTRACTINFO_H
