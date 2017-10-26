#ifndef CONTRACTINFO_H
#define CONTRACTINFO_H

#include <QVariant>
#include <QVariantList>
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
        FilterInfo(const QString& name, const QString& address, const QString& contract, const QJsonArray& topics, bool active);
        FilterInfo(const QJsonObject& source);

        const QVariant value(const int role) const;
        const QJsonObject toJson() const;
        const QString toJsonString() const;
        const QString getHandle() const;
        void setActive(bool active);
        bool getActive() const;
        const QString getHash() const;
    private:
        QString fName;
        QString fAddress;
        QString fContract;
        QJsonArray fTopics;
        bool fActive;
        QString fHash;

        const QString calculateHash() const;
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
        int getArgumentCount() const;
        const ContractArg getReturn(int index) const;
        int getReturnsCount() const;
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

        const QVariantList getArgModel(bool indexedOnly) const;
        const QJsonArray encodeTopics(const QVariantList& params) const;
    private:
        QVariantList fArgModel;
    };


    typedef QList<ContractEvent> ContractEventList;

    class ContractFunction : public ContractCallable
    {
    public:
        ContractFunction(const QJsonObject& source);

        const QVariantList getArgModel() const;
        const QString callData(const QVariantList& params) const;
        const QVariantList parseResponse(const QString& data) const;
        bool isConstant() const;
    private:
        QVariantList fArgModel;
        bool fConstant;
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

    class ResultInfo
    {
    public:
        ResultInfo(const QString data);
        virtual ~ResultInfo();
        void fillContract(const ContractInfo& contract);
        void fillParams(const ContractInfo& contract, const ContractCallable& source);

        const QString contract() const;
        const ContractArgs getArguments() const;
        const QVariantList getParams() const;
        const QString paramToStr(const QVariant& value) const;
    protected:
        QString fName;
        QString fContract;
        QString fData;
        ContractArgs fArguments;
        QVariantList fParams;

        virtual const QString getValue(const ContractArg arg, int &topicIndex, int &index, const QString &data);
    };

    class EventInfo : public ResultInfo
    {
    public:
        EventInfo(const QJsonObject& source);
        virtual ~EventInfo();
        const QString address() const;
        const QString signature() const;
        const QString transactionHash() const;
        const QString getMethodID() const;
        const QVariant value(const int role) const;
        quint64 blockNumber() const;
    protected:
        virtual const QString getValue(const ContractArg arg, int &topicIndex, int &index, const QString &data);
    private:
        QString fAddress;
        quint64 fBlockNumber;
        QString fTransactionHash;
        QString fBlockHash;
        QString fMethodID;
        QStringList fTopics;
    };

    typedef QList<EventInfo> EventList;

    enum ContractRoles {
        ContractNameRole = Qt::UserRole + 1,
        AddressRole,
        TokenRole,
        DecimalsRole,
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
        const QStringList eventList() const;
        const ContractFunction function(const QString& name, int& index) const;
        const ContractFunction function(int index) const;
        const ContractEvent event(const QString& name, int& index) const;
        int eventIndexByMethodID(const QString& methodID) const;
        void processEvent(EventInfo& info) const;
        const QString token() const;
        bool isERC20() const;
        quint8 decimals() const;
        bool needsERC20Init() const;
        void loadSymbolData(const QString& data);
        void loadDecimalsData(const QString& data);
        void loadNameData(const QString& data);
    private:
        void parse();

        QString fName;
        QString fAddress;
        QJsonArray fABI;
        ContractFunctionList fFunctions;
        ContractEventList fEvents;
        QString fToken;
        quint8 fDecimals;
        bool fIsERC20;

        bool checkERC20Compatibility();
    };

    typedef QList<ContractInfo> ContractList;

}

#endif // CONTRACTINFO_H
