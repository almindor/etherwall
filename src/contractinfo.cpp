#include "contractinfo.h"
#include <QRegExp>
#include <QDebug>
#include "helpers.h"
#include "etherlog.h"

namespace Etherwall {

    // ***************************** FilterInfo ***************************** //

    FilterInfo::FilterInfo(const QString& name, const QString& address, const QString& contract, const QJsonArray& topics, bool active) :
        fName(name), fAddress(address), fContract(contract), fTopics(topics), fActive(active)
    {
    }

    FilterInfo::FilterInfo(const QJsonObject& source) {
        fName = source.value("name").toString("invalid");
        fAddress = source.value("address").toString("invalid");
        fContract = source.value("contract").toString("invalid");
        fTopics = source.value("topics").toArray();
        fActive = source.value("active").toBool(false);
    }

    const QVariant FilterInfo::value(const int role) const {
        switch ( role ) {
            case FilterNameRole: return fName;
            case FilterAddressRole: return fAddress;
            case FilterContractRole: return fContract;
            case FilterTopicsRole: return fTopics;
            case FilterActiveRole: return fActive;
        }

        return QVariant();
    }

    void FilterInfo::setActive(bool active) {
        fActive = active;
    }

    bool FilterInfo::getActive() const
    {
        return fActive;
    }

    const QJsonObject FilterInfo::toJson() const {
        QJsonObject result;
        result["name"] = fName;
        result["address"] = fAddress;
        result["contract"] = fContract;
        result["topics"] = fTopics;
        result["active"] = fActive;

        return result;
    }

    const QString FilterInfo::toJsonString() const {
        const QJsonDocument doc(toJson());
        return doc.toJson(QJsonDocument::Compact);
    }

    const QString FilterInfo::getHandle() const {
        return fName + "_" + fContract;
    }

    // ***************************** ContractArg ***************************** //

    ContractArg::ContractArg(const QString& name, const QString &literal, bool indexed) {
        const QRegExp typeMatcher("^([a-z]+)([0-9]*)x?([0-9]*)$");
        const QRegExp arrayMatcher("^(.*)\\[([0-9]*)\\]$");

        fName = name;
        fIndexed = indexed; // only for events
        QString typeStr = literal;
        bool ok = true;

        // if it's an array get the length and typestr from it
        if ( arrayMatcher.exactMatch(literal) ) {
            typeStr = arrayMatcher.cap(1);
            fLength = arrayMatcher.cap(2).isEmpty() ? 0 : arrayMatcher.cap(2).toInt(&ok, 10);
            if ( !ok ) {
                throw QString("Invalid array length");
            }
        } else {
            fLength = -1;
        }

        if ( !typeMatcher.exactMatch(typeStr) ) {
            throw QString("Invalid type definition");
        }

        fBaseType = fType = typeMatcher.cap(1);

        fM = typeMatcher.cap(2).isEmpty() ? -1 : typeMatcher.cap(2).toInt(&ok, 10);
        if ( !ok ) {
            throw QString("Invalid size specifier (M)");
        }

        fN = typeMatcher.cap(3).isEmpty() ? -1 : typeMatcher.cap(3).toInt(&ok, 10);
        if ( !ok ) {
            throw QString("Invalid size specifier (N)");
        }

        // canonical representations
        if ( fBaseType == "int" || fBaseType == "uint" ) {
            if ( fM < 0 ) fM = 256;
            fType += QString::number(fM, 10);
        } else if ( ( fBaseType == "fixed" || fBaseType == "ufixed") && fM < 0 ) {
            if ( fM < 0 ) fM = 128;
            if ( fN < 0 ) fN = 128;
            fType += QString::number(fM, 10) + "x" + QString::number(fN, 10);
        } else if ( fBaseType == "bytes" && fM > 0 ) {
            fType += QString::number(fM, 10);
        }

        // array to canonical
        if ( fLength >= 0 ) {
            fType += "[" + (fLength > 0 ? QString::number(fLength, 10) : "") + "]";
        }
    }

    int ContractArg::length() const {
        return fLength;
    }

    int ContractArg::M() const {
        return fM;
    }

    int ContractArg::N() const {
        return fN;
    }

    const QString ContractArg::type() const {
        return fType;
    }

    const QString ContractArg::name() const {
        return fName;
    }

    bool ContractArg::indexed() const {
        return fIndexed;
    }

    const QString ContractArg::toString() const {
        return "Type: " + fType + " Length: " + QString::number(fLength) + " M: " + QString::number(fM) + " N: " + QString::number(fN);
    }

    const QVariantMap ContractArg::toVariantMap() const {
        QVariantMap result;
        result["name"] = fName;
        result["type"] = fType;
        result["indexed"] = fIndexed;
        result["length"] = fLength;
        result["placeholder"] = getPlaceholder();
        result["valrex"] = getValRex();

        return result;
    }

    bool ContractArg::dynamic() const {
        return (fType == "string" || fType == "bytes" || fLength == 0);
    }

    const QVariant ContractArg::decode(const QString& data, bool inArray) const {
        if ( !inArray && fLength >= 0 ) {
            ulong size = fLength > 0 ? fLength : decodeInt(data.left(64), false).toUlong();

            QVariantList result;
            for ( ulong i = 1; i <= size; i++ ) {
                result.append(decode(data.mid(i * 64, 64), true));
            }

            return result;
        }

        // we decode most types to string due to bigint's limits and the fact
        // that we only display them, never use them directly
        if ( fBaseType == "address" ) {
            return "0x" + data.right(40);
        }

        if ( fBaseType == "uint" || fBaseType == "int" ) {
            const BigInt::Rossi ival = decodeInt(data, fBaseType.at(0) == 'u');
            return QString(ival.toStrDec().c_str());
        }

        if ( fBaseType == "fixed" || fBaseType == "ufixed" ) {
            const BigInt::Rossi fval = decodeInt(data, fBaseType.at(0) == 'f');

            int i;
            BigInt::Rossi twoAtN(2);
            for ( i = 1; i < fN; i++ ) {
                twoAtN = twoAtN * 2;
            }

            const BigInt::Rossi fresult = fval / twoAtN;
            return QString(fresult.toStrDec().c_str());
        }

        if ( fBaseType == "string" ) {
            // get byte count
            ulong bytes = decodeInt(data.left(64), false).toUlong();
            return QString(QByteArray::fromHex(data.mid(64, bytes * 2).toUtf8()));
        }

        if ( fBaseType == "bytes" ) {
            // get byte count
            QString hexStr;
            if ( dynamic() ) {
                ulong bytes = decodeInt(data.left(64), false).toUlong();
                hexStr = data.mid(64, bytes * 2).toUtf8();
            } else { // static bytes array
                ulong bytes = fM;
                hexStr = data.left(bytes * 2).toUtf8();
            }
            const QByteArray raw = QByteArray::fromHex(hexStr.toUtf8());
            const QString utf8 = QString::fromUtf8(raw); // bytes is mostly utf-8

            if ( utf8.length() > 0 ) {
                return utf8;
            }

            return hexStr;
       }

        if ( fBaseType == "bool" ) {
            const BigInt::Rossi ival(data.toStdString(), 16);
            return ival == BigInt::Rossi(1);
        }

        throw QString(QString("DECODE => Unknown type: ") + fBaseType);
    }

    const QString ContractArg::encode(const QVariant& val, bool inArray) const {
        // if we're an array of anything consider a string, split and encode individually
        if ( fLength >= 0 && !inArray ) {
            const QString arrStr = val.toString().remove('[').remove(']'); // optional []
            const QStringList arr = arrStr.split(',');
            if ( fLength > 0 && arr.length() != fLength ) {
                throw QString(fName + ": Invalid params count for array of " + QString::number(fLength));
            }
            QString result = fLength > 0 ? "" : encodeInt(arr.size()); // encode length on dynamic arrays only

            foreach ( const QString subVal, arr ) {
                result += encode(subVal, true);
            }

            return result;
        }

        if ( fBaseType == "address" ) {
            QString hexStr = val.toString();
            const QRegExp re("0x[a-f,A-F,0-9]{40}");
            if ( !re.exactMatch(hexStr) ) {
                throw QString("Invalid address: " + hexStr);
            }
            if ( Helpers::vitalizeAddress(hexStr) != hexStr ) {
                throw QString("Address checksum mismatch: " + hexStr);
            }
            if ( hexStr.length() == 42 ) {
                hexStr.remove(0, 2); // remove 0x
            }
            BigInt::Rossi addrNum(hexStr.toStdString(), 16);

            return encodeInt(addrNum);
        }

        bool ok = false;
        if ( fBaseType == "int" || fBaseType == "uint" ) {
            int ival = val.toInt(&ok);
            if ( !ok ) throw QString(fName + ": Invalid " + fBaseType + " argument: " + val.toString());
            return encode(ival);
        }

        if ( fBaseType == "fixed" || fBaseType == "ufixed" ) {
            QString fixedVal = val.toString();
            int n = fixedVal.indexOf('.');
            int digits = n > 0 ? fixedVal.length() - n - 1 : 0;
            if ( n > 0 ) fixedVal.remove(n, 1);
            const BigInt::Rossi fixedRossi(fixedVal.toStdString(), 10);
            return encode(fixedRossi, digits);
        }

        if ( fBaseType == "string" ) {
            return encode(val.toString());
        }

        if ( fBaseType == "bytes" ) {
            QString sVal = val.toString();
            if ( sVal.startsWith("0x") && sVal.size() > 2 ) { // hex value
                sVal.remove(0, 2);
                return encode(QByteArray::fromHex(sVal.toUtf8()));
            }
            // otherwise consider binary (strings)
            return encode(val.toByteArray());
        }

        if ( fBaseType == "bool" ) {
            return encode(val.toBool());
        }

        throw QString(QString("ENCODE => Unknown type: ") + fBaseType);
    }

    const QString ContractArg::encode(const QString& text) const {
        if ( fBaseType != "string" ) {
            throw QString("Invalid argument encode value for " + fBaseType + " expected string");
        }

        return encodeBytes(text.toUtf8());
    }

    const QString ContractArg::encode(const QByteArray& bytes) const {
        if ( fBaseType != "bytes" ) {
            throw QString("Invalid argument encode value for " + fBaseType + " expected bytes");
        }

        return encodeBytes(bytes, fM);
    }

    const QString ContractArg::encode(int number) const {
        if ( fBaseType != "int" && fBaseType != "uint" ) {
            throw QString("Invalid argument encode value for " + fBaseType + " expected int or uint");
        }

        return encodeInt(number);
    }

    const QString ContractArg::encode(const BigInt::Rossi& val, int digits) const {
        if ( fBaseType != "fixed" && fBaseType != "ufixed" ) {
            throw QString("Invalid argument encode value for " + fBaseType + " expected fixed or ufixed");
        }

        int i;
        BigInt::Rossi twoAtN(2);
        for ( i = 1; i < fN; i++ ) {
            twoAtN = twoAtN * 2;
        }

        BigInt::Rossi divider(1);
        for ( i = 0; i < digits; i++ ) {
            divider = divider * 10;
        }

        return encodeInt(val * twoAtN / divider);
    }

    const QString ContractArg::encode(bool val) const {
        if ( fBaseType != "bool" ) {
            throw QString("Invalid argument encode value for " + fBaseType + " expected bool");
        }

        int number = val ? 1 : 0;
        return encodeInt(number);
    }

    const QString ContractArg::encodeInt(int number) {
        if ( number < 0 ) {
            return QString("ff%1").arg(number * -1, 62, 16, QChar('0'));
        }

        return QString("%1").arg(number, 64, 16, QChar('0'));
    }

    const QString ContractArg::encodeInt(const BigInt::Rossi& number) {
        QString strNum;
        if ( number < BigInt::Rossi(0) ) {
            strNum = QString((number * BigInt::Rossi(-1)).toStrHex().c_str());
        } else {
            strNum = QString((number).toStrHex().c_str());
        }

        while ( strNum.length() < 64 ) {
            strNum = "0" + strNum;
        }

        return strNum;
    }

    const BigInt::Rossi ContractArg::decodeInt(const QString& data, bool isSigned) {
        QString rawData = data;
        int multiplier = 1;
        if ( isSigned && data.at(0) == 'f' && data.at(1) == 'f' ) {
            rawData.replace(0, 2, '0');
            multiplier = -1;
        }

        return BigInt::Rossi(rawData.toStdString(), 16) * multiplier;
    }

    const QString ContractArg::encodeBytes(QByteArray bytes, int fixedSize) {
        if ( fixedSize > 0 && bytes.size() > fixedSize ) {
            throw QString("Byte array too large for static bytes" + QString::number(fixedSize));
        }

        const QString sizePrefix = (fixedSize <= 0 ? encodeInt(bytes.size()) : ""); // static has no size prefix
        int coef = bytes.size() / 64;
        int requestedSize = 32 * (coef + 1);

        while ( bytes.size() < requestedSize ) {
            bytes.append('\0');
        }

        return sizePrefix + bytes.toHex();
    }

    const QRegExp ContractArg::getValRex() const {
        QString pattern = ".*";

        if ( fBaseType == "int" ) {
            pattern = "-?[0-9]+";
        } else if ( fBaseType == "uint" ) {
            pattern = "[0-9]+";
        } else if ( fBaseType == "bool" ) {
            pattern = "true|false";
        } else if ( fBaseType == "address" ) {
            pattern = "0x[a-f,A-F,0-9]{40}";
        } else if ( fBaseType == "fixed" ) {
            pattern = "-?[0-9]+\\.[0-9]+";
        } else if ( fBaseType == "ufixed" ) {
            pattern = "[0-9]+\\.[0-9]+";
        }

        // array pattern around the base type
        if ( fLength >= 0 ) {
            pattern = "^\\[?(" + pattern + "\\,?)+\\]?$";
        }

        return QRegExp(pattern);
    }

    const QString ContractArg::getPlaceholder() const {
        QString result = "text";

        if ( fBaseType == "int" ) {
            result = "-15";
        } else if ( fBaseType == "uint" ) {
            result = "23";
        } else if ( fBaseType == "bool" ) {
            result = "true";
        } else if ( fBaseType == "address" ) {
            result = "0x0000000000000000000000000000000000000000";
        } else if ( fBaseType == "fixed" ) {
            result = "-0.5";
        } else if ( fBaseType == "ufixed" ) {
            result = "1.245";
        }

        if ( fLength >= 0 ) {
            result = "[" + result + "," + result + "]";
        }

        return result;
    }

    // ***************************** ContractCallable ***************************** //

    ContractCallable::ContractCallable(const QJsonObject& source)
    {
        fName = source.value("name").toString();
        fArguments = ContractArgs();
        fReturns = ContractArgs();

        const QJsonArray args = source.value("inputs").toArray();
        const QJsonArray rets = source.value("outputs").toArray();

        foreach ( QJsonValue arg, args ) {
            const ContractArg carg = ContractArg(getArgName(arg), getArgLiteral(arg), getArgIndexed(arg));
            fArguments.append(carg);
        }

        foreach ( QJsonValue ret, rets ) {
            fReturns.append(ContractArg(getArgName(ret), getArgLiteral(ret)));
        }

        fSignature = buildSignature();
        fMethodID = QString(Helpers::keccak256(fSignature.toUtf8()).left(4).toHex());
    }

    const QString ContractCallable::getArgLiteral(const QJsonValue& arg) const {
        if ( !arg.isObject() ) {
            throw QString("Invalid argument");
        }

        QJsonObject argObj = arg.toObject();

        if ( !argObj.contains("type") ) {
            throw QString("Invalid argument");
        }

        return argObj.value("type").toString();
    }

    const QString ContractCallable::getArgName(const QJsonValue& arg) const {
        if ( !arg.isObject() ) {
            throw QString("Invalid argument");
        }

        QJsonObject argObj = arg.toObject();

        if ( !argObj.contains("name") ) {
            throw QString("Invalid argument");
        }

        return argObj.value("name").toString();
    }

    bool ContractCallable::getArgIndexed(const QJsonValue& arg) const {
        if ( !arg.isObject() ) {
            throw QString("Invalid argument");
        }

        QJsonObject argObj = arg.toObject();

        if ( !argObj.contains("indexed") ) {
            return false;
        }

        return argObj.value("indexed").toBool(false);
    }

    const QString ContractCallable::getName() const {
        return fName;
    }

    const ContractArg ContractCallable::getArgument(int index) const {
        return fArguments.at(index);
    }

    const ContractArgs ContractCallable::getArguments() const {
        return fArguments;
    }

    int ContractCallable::getArgumentCount() const
    {
        return fArguments.size();
    }

    const ContractArg ContractCallable::getReturn(int index) const
    {
        return fReturns.at(index);
    }

    int ContractCallable::getReturnsCount() const
    {
        return fReturns.size();
    }

    const QString ContractCallable::getMethodID() const {
        return fMethodID;
    }

    const QString ContractCallable::buildSignature() const {
        QStringList list;

        foreach ( const ContractArg arg, fArguments ) {
            list.append(arg.type());
        }

        return fName + "(" + list.join(',') + ")";
    }

    const QString ContractCallable::getSignature() const {
        return fSignature;
    }

    // ***************************** ContractEvent ***************************** //

    ContractEvent::ContractEvent(const QJsonObject &source) : ContractCallable(source) {
        fMethodID = QString(Helpers::keccak256(fSignature.toUtf8()).toHex());
        fArgModel = QVariantList();

        foreach ( const ContractArg carg, fArguments ) {
            fArgModel.append(carg.toVariantMap());
        }
    }

    const QVariantList ContractEvent::getArgModel(bool indexedOnly) const
    {
        if ( !indexedOnly ) {
            return fArgModel;
        }

        QVariantList result;
        foreach ( const ContractArg carg, fArguments ) {
            if ( carg.indexed() ) {
                result.append(carg.toVariantMap());
            }
        }

        return result;
    }

    const QJsonArray ContractEvent::encodeTopics(const QVariantList &params) const
    {
        QJsonArray topics;
        topics.append("0x" + fMethodID); // 0th is methodID

        if ( params.size() > fArguments.size() ) {
            EtherLog::logMsg("More params than arguments for event topic", LS_Error);
            return topics;
        }

        int i = 0;
        foreach ( const QVariant& param, params ) {
            if ( param.type() == QVariant::List ) {
                QJsonArray innerList;

                foreach ( const QVariant& inner, param.toList() ) {
                    innerList.append("0x" + fArguments.at(i).encode(inner));
                }
                topics.append(innerList);
            } else {
                if ( param.isNull() || param.toString().size() == 0 ) {
                    topics.append(QJsonValue(QJsonValue::Null));
                } else {
                    topics.append("0x" + fArguments.at(i).encode(param));
                }
            }
            i++;
        }

        // no need to specify nulls unless there's a non-null following them
        while ( !topics.empty() && topics.last().isNull() ) {
            topics.removeLast();
        }

        return topics;
    }

    // ***************************** ContractFunction ***************************** //

    ContractFunction::ContractFunction(const QJsonObject &source) : ContractCallable(source) {
        fArgModel = QVariantList();
        fConstant = source.value("constant").toBool(false);

        foreach ( const ContractArg carg, fArguments ) {
            fArgModel.append(carg.toVariantMap());
        }
    }

    const QVariantList ContractFunction::getArgModel() const {
        return fArgModel;
    }

    const QString ContractFunction::callData(const QVariantList& params) const {
        if ( fArguments.size() != params.size() ) {
            throw QString("Incorrect amount of parameters passed to function \"" + fName + "\" got " + QString::number(params.size()) + " expected " + QString::number(fArguments.size()));
        }

        QStringList encStr(fMethodID);
        QStringList dynaStr;

        int i = 0;
        int offset = fArguments.size() * 32; // dynamic offset
        foreach ( const ContractArg& arg, fArguments ) {
            const QString encoded = arg.encode(params.at(i++));
            if ( arg.dynamic() ) {
                encStr.append(ContractArg::encodeInt(offset));
                dynaStr.append(encoded);
                offset += (encoded.length() / 2);
            } else {
                encStr.append(encoded);
            }
        }

        return encStr.join("") + dynaStr.join("");
    }

    const QVariantList ContractFunction::parseResponse(const QString &data) const
    {
        QString prepared = data;
        if ( prepared.startsWith("0x") ) {
            prepared.remove(0, 2); // remove 0x
        }

        QVariantList results;
        if ( prepared.size() == 0 ) {
            EtherLog::logMsg("Invalid result received", LS_Warning);
            return results;
        }

        int offset = 0;
        foreach ( const ContractArg arg, fReturns ) {
            QString raw;
            if ( arg.dynamic() ) {
                int dynamicOffset = arg.decodeInt(prepared.mid(offset, 64), false).toUlong() * 2;
                raw = prepared.mid(dynamicOffset); // dynamic types cut off themselves
            } else {
                raw = prepared.mid(offset, 64); // static are always 32bytes and don't cut off
            }
            const QVariant result = arg.decode(raw);

            QVariantMap row;
            row["number"] = offset / 64;
            row["type"] = arg.type();
            row["value"] = result;
            results.append(row);
            offset += 64;
        }

        return results;
    }

    bool ContractFunction::isConstant() const
    {
        return fConstant;
    }

    // ***************************** EventInfo ***************************** //

    EventInfo::EventInfo(const QJsonObject& source) : ResultInfo(source["data"].toString()) {
        fBlockNumber = Helpers::toQUInt64(source["blockNumber"]);
        fBlockHash = source["blockHash"].toString();
        fAddress = Helpers::vitalizeAddress(source["address"].toString());
        fTransactionHash = source["transactionHash"].toString();
        const QVariantList topics = source["topics"].toArray().toVariantList();
        fTopics = QStringList();
        foreach ( const QVariant v, topics ) {
            fTopics.append(v.toString());
        }
        fMethodID = QString("invalid");
        if ( fTopics.length() > 0 ) {
            fMethodID = fTopics.at(0);
            if ( fMethodID.length() > 1 && fMethodID.at(0) == '0' && fMethodID.at(1) == 'x') {
                fMethodID.remove(0, 2);
            }
        }
    }

    EventInfo::~EventInfo()
    {

    }

    const QString EventInfo::address() const {
        return fAddress;
    }

    const QString EventInfo::signature() const {
        QStringList vals;
        foreach ( const QVariant param, fParams ) {
            vals.append(paramToStr(param));
        }

        return fName + "(" + vals.join(",") + ")";
    }

    const QString EventInfo::transactionHash() const {
        return fTransactionHash;
    }

    const QString EventInfo::getMethodID() const {
        return fMethodID;
    }

    const QVariant EventInfo::value(const int role) const {
        switch ( role ) {
            case EventNameRole: return fName;
            case EventAddressRole: return fAddress;
            case EventDataRole: return fData;
            case EventContractRole: return fContract;
            case EventBlockHashRole: return fBlockHash;
            case EventBlockNumberRole: return fBlockNumber;
            case EventTransactionHashRole: return fTransactionHash;
            case EventTopicsRole: return fTopics.join(",");
        }

        return QVariant();
    }

    quint64 EventInfo::blockNumber() const {
        return fBlockNumber;
    }

    const QString EventInfo::getValue(const ContractArg arg, int &topicIndex, int &index, const QString &data)
    {
        QString val;
        if ( arg.indexed() ) {
            val = fTopics.at(topicIndex++).right(64);
        } else {
            return ResultInfo::getValue(arg, topicIndex, index, data);
        }

        return val;
    }

    // ***************************** ContractInfo ***************************** //

    ContractInfo::ContractInfo(const QString &name, const QString& address, const QJsonArray &abi) :
        fName(name), fAddress(Helpers::vitalizeAddress(address)), fABI(abi), fFunctions()
    {
        parse();
        fIsERC20 = checkERC20Compatibility();
    }

    ContractInfo::ContractInfo(const QJsonObject &source) {
        fName = source.value("name").toString();
        fAddress = Helpers::vitalizeAddress(source.value("address").toString());
        fABI = source.value("abi").toArray();
        fFunctions = ContractFunctionList();

        parse();
        if ( !source.contains("erc20") ) {
            fIsERC20 = checkERC20Compatibility();
        } else {
            fIsERC20 = source.value("erc20").toBool();
            if ( fIsERC20 ) {
                fToken = source.value("token").toString();
                fDecimals = source.value("decimals").toInt(0);
            }
        }
    }

    const QVariant ContractInfo::value(const int role) const {
        switch ( role ) {
            case ContractNameRole: return QVariant(fName);
            case AddressRole: return QVariant(fAddress);
            case TokenRole: return QVariant(fToken);
            case DecimalsRole: return QVariant(fDecimals);
            case ABIRole: return QVariant(QString(QJsonDocument(fABI).toJson()));
        }

        return QVariant();
    }

    const QJsonObject ContractInfo::toJson() const {
        QJsonObject result;
        result["name"] = fName;
        result["address"] = fAddress;
        result["abi"] = fABI;
        result["token"] = fToken;
        result["decimals"] = fDecimals;
        result["erc20"] = fIsERC20;

        return result;
    }

    const QString ContractInfo::toJsonString() const {
        const QJsonDocument doc(toJson());
        return doc.toJson(QJsonDocument::Compact);
    }

    const QString ContractInfo::name() const {
        return fName;
    }

    const QString ContractInfo::address() const {
        return fAddress;
    }

    const QString ContractInfo::abi() const {
        return value(ABIRole).toString();
    }

    const QJsonArray ContractInfo::abiJson() const {
        return fABI;
    }

    const QStringList ContractInfo::functionList() const {
        QStringList list;

        foreach ( ContractFunction func, fFunctions ) {
            list.append(func.getName());
        }

        return list;
    }

    const QStringList ContractInfo::eventList() const {
        QStringList list;

        foreach ( ContractEvent event, fEvents ) {
            list.append(event.getName());
        }

        return list;
    }

    const ContractFunction ContractInfo::function(const QString &name, int &index) const
    {
        for ( int i = 0; i < fFunctions.size(); i++ ) {
            if ( fFunctions.at(i).getName() == name ) {
                index = i;
                return fFunctions.at(i);
            }
        }

        index = -1;
        throw QString("Function " + name + " not found");
    }

    const ContractFunction ContractInfo::function(int index) const {
        if ( index < 0 || index >= fFunctions.size() ) {
            throw QString("Function index out of bounds");
        }

        return fFunctions.at(index);
    }

    const ContractEvent ContractInfo::event(const QString &name, int &index) const
    {
        for ( int i = 0; i < fEvents.size(); i++ ) {
            if ( fEvents.at(i).getName() == name ) {
                index = i;
                return fEvents.at(i);
            }
        }

        index = -1;
        throw QString("Event " + name + " not found");
    }

    int ContractInfo::eventIndexByMethodID(const QString &methodID) const
    {
        for ( int i = 0; i < fEvents.size(); i++ ) {
            if ( fEvents.at(i).getMethodID() == methodID ) {
                return i;
            }
        }

        return -1;
    }

    void ContractInfo::processEvent(EventInfo& info) const {
        foreach ( const ContractEvent event, fEvents ) {
            if ( event.getMethodID() == info.getMethodID() ) {
                info.fillParams(*this, event);
                return;
            }
        }

        // Couldn't match event, fill contract at least
        info.fillContract(*this);
    }

    const QString ContractInfo::token() const
    {
        return fToken;
    }

    bool ContractInfo::isERC20() const
    {
        return fIsERC20;
    }

    quint8 ContractInfo::decimals() const
    {
        return fDecimals;
    }

    bool ContractInfo::needsERC20Init() const
    {
        return fIsERC20 && fToken.isEmpty();
    }

    void ContractInfo::loadSymbolData(const QString &data)
    {
        int i;
        const QVariantList outputs = function("symbol", i).parseResponse(data);
        if ( outputs.size() != 1 ) {
            throw QString("Unexpected symbol result for token contract: " + fAddress);
        }

        const QVariantMap row = outputs.at(0).toMap();
        fToken = row.value("value", "invalid").toString();
    }

    void ContractInfo::loadDecimalsData(const QString &data)
    {
        int i;
        const QVariantList outputs = function("decimals", i).parseResponse(data);
        if ( outputs.size() != 1 ) {
            throw QString("Unexpected decimals result for token contract: " + fAddress);
        }

        const QVariantMap row = outputs.at(0).toMap();
        bool ok = false;
        fDecimals = row.value("value", 0).toInt(&ok);
        if ( !ok ) {
            throw QString("Invalid decimals result: " + row.value("value").toString() + " for token contract: " + fAddress);
        }
    }

    void ContractInfo::loadNameData(const QString &data)
    {
        int i;
        const QVariantList outputs = function("name", i).parseResponse(data);
        if ( outputs.size() != 1 ) {
            throw QString("Unexpected name result for token contract: " + fAddress);
        }

        const QVariantMap row = outputs.at(0).toMap();
        fName = row.value("value", "invalid").toString();
    }

    void ContractInfo::parse() {
        const QJsonArray source = abiJson();

        foreach ( const QJsonValue val, source ) {
            if ( !val.isObject() ) {
                throw QString("Invalid ABI argument: " + val.toString());
            }

            const QJsonObject obj = val.toObject();
            if ( obj.contains("type") ) {
                const QString typeStr = obj.value("type").toString();
                if ( typeStr == "function" ) {
                    fFunctions.append(ContractFunction(obj));
                } else if ( typeStr == "event" ) {
                    fEvents.append(ContractEvent(obj));
                }
            }
        }
    }

    bool ContractInfo::checkERC20Compatibility()
    {
        /*
         *  full erc20 has quite a lot of functions but for our wallet purpose we only check:
         *  function balanceOf(address _owner) constant returns (uint balance);
         *  function transfer(address _to, uint256 _value) returns (bool success);
         *  string public constant symbol = "SYM";
         *  uint8 public constant decimals = 18
        */
        QJsonParseError parseError;
        const QByteArray rawDefinition = "{\"balanceOf\":{\"constant\":true,\"inputs\":[\"address\"],\"outputs\":[\"uint256\"]},\"transfer\": {\"inputs\":[\"address\",\"uint256\"],\"outputs\":[\"bool\"]},\"symbol\":{\"constant\":true},\"decimals\":{\"constant\":true},\"name\":{\"constant\":true}}";
        QJsonObject required = QJsonDocument::fromJson(rawDefinition, &parseError).object();
        QMap<QString, bool> contains;

        foreach ( const QString& function, required.keys() ) {
            contains[function] = false;
        }

        foreach ( const ContractFunction& func, fFunctions ) {
            const QString funcName = func.getName();
            // if we have function of given name, check the args/returns and constant modifier
            if ( required.contains(funcName) ) {
                const QJsonObject def = required.value(funcName).toObject();
                // check constant match
                if ( def.value("constant").toBool(false) != func.isConstant() ) {
                    continue;
                }

                bool argsOk = true;
                // if def has inputs check those
                if ( def.contains("inputs") ) {
                    const QJsonArray inputs = def.value("inputs").toArray();
                    if ( inputs.size() != func.getArgumentCount() ) {
                        continue;
                    }

                    for ( int i = 0; i < func.getArgumentCount(); i++ ) {
                        if ( func.getArgument(i).type() != inputs.at(i).toString("invalid") ) {
                            argsOk = false;
                            break;
                        }
                    }
                }

                // if def has outputs check those but only for constants, omisgo and other change the tx ones to voids and we don't care
                if ( func.isConstant() && def.contains("outputs") ) {
                    const QJsonArray outputs = def.value("outputs").toArray();
                    if ( outputs.size() != func.getReturnsCount() ) {
                        continue;
                    }

                    argsOk = true;
                    for ( int i = 0; i < func.getReturnsCount(); i++ ) {
                        if ( func.getReturn(i).type() != outputs.at(i).toString("invalid") ) {
                            argsOk = false;
                            break;
                        }
                    }
                }

                if ( !argsOk ) {
                    continue;
                }

                contains[funcName] = true;
            }
        }

        foreach ( const QString& function, contains.keys() ) {
            if ( !contains[function] ) {
                return false;
            }
        }

        return true;
    }

    // ***************************** ResultInfo ***************************** //

    ResultInfo::ResultInfo(const QString data) : fData(data)
    {

    }

    ResultInfo::~ResultInfo()
    {

    }

    void ResultInfo::fillContract(const ContractInfo& contract) {
        fContract = contract.name();
    }

    void ResultInfo::fillParams(const ContractInfo &contract, const ContractCallable &source)
    {
        fillContract(contract);
        fName = source.getName();
        fArguments = source.getArguments();

        QString data = QString(fData);
        data.remove(0, 2); // remove the 0x
        int n = 0;
        int t = 1;

        foreach ( const ContractArg arg, source.getArguments() ) {
            QString val = getValue(arg, t, n, data);

            if ( arg.dynamic() ) { // value holds "pointer" to data in data
                ulong ptr = arg.decodeInt(val, false).toUlong() * 2;
                val = data.mid(ptr);
                fParams.append(arg.decode(val));
            } else { // value is direct
                fParams.append(arg.decode(val));
            }
        }
    }

    const QString ResultInfo::contract() const
    {
        return fContract;
    }

    const QString ResultInfo::getValue(const ContractArg arg, int &topicIndex, int &index, const QString &data)
    {
        Q_UNUSED(arg);
        Q_UNUSED(topicIndex);

        const QString val = data.mid(index, 64);
        index += 64;

        return val;
    }


    const ContractArgs ResultInfo::getArguments() const {
        return fArguments;
    }

    const QVariantList ResultInfo::getParams() const {
        return fParams;
    }

    const QString ResultInfo::paramToStr(const QVariant& value) const {
        QString strVal;
        if ( value.type() == QVariant::StringList ) {
            strVal = "[" + value.toStringList().join(",") + "]";
        } else if ( value.type() == QVariant::List ) {
            QStringList vals;
            foreach ( const QVariant inner, value.toList() ) {
                vals.append(inner.toString());
            }
            strVal = "[" + vals.join(",") + "]";
        } else {
            strVal = value.toString();
        }

        return strVal;
    }


}
