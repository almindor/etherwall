#include "contractinfo.h"
#include <QRegExp>
#include <QDebug>
#include "helpers.h"

namespace Etherwall {

    // ***************************** FilterInfo ***************************** //

    FilterInfo::FilterInfo(const QString& name, const QString& address, const QString& contract, const QStringList& topics, bool active) :
        fName(name), fAddress(address), fContract(contract), fTopics(topics), fActive(active)
    {
    }

    FilterInfo::FilterInfo(const QJsonObject& source) {
        fName = source.value("name").toString("invalid");
        fAddress = source.value("address").toString("invalid");
        fContract = source.value("contract").toString("invalid");
        fTopics = source.value("topics").toString("invalid").split(",");
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

    const QJsonObject FilterInfo::toJson() const {
        QJsonObject result;
        result["name"] = fName;
        result["address"] = fAddress;
        result["contract"] = fContract;
        result["topics"] = fTopics.join(",");
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
            bool isAscii = true;
            foreach ( uchar b, raw ) {
                if ( b < 32 || b > 126 ) {
                    isAscii = false;
                    break;
                }
            }

            if ( isAscii ) {
                return QString(raw);
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
        fMethodID = QString(QCryptographicHash::hash(fSignature.toUtf8(), QCryptographicHash::Sha3_256).left(4).toHex());
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
        fMethodID = QString(QCryptographicHash::hash(fSignature.toUtf8(), QCryptographicHash::Sha3_256).left(32).toHex());
    }

    // ***************************** ContractFunction ***************************** //

    ContractFunction::ContractFunction(const QJsonObject &source) : ContractCallable(source) {
        fArgModel = QVariantList();

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

    // ***************************** EventInfo ***************************** //

    EventInfo::EventInfo(const QJsonObject& source) {
        fBlockNumber = Helpers::toQUInt64(source["blockNumber"]);
        fBlockHash = source["blockHash"].toString();
        fData = source["data"].toString();
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

    void EventInfo::fillContract(const ContractInfo& contract) {
        fContract = contract.name();
    }

    void EventInfo::fillParams(const ContractInfo& contract, const ContractEvent& event) {
        if ( fMethodID != event.getMethodID() ) {
            throw QString("Event methodID mismatch");
        }

        fillContract(contract);
        fName = event.getName();
        fArguments = event.getArguments();

        QString data = QString(fData);
        data.remove(0, 2); // remove the 0x
        int n = 0;
        int t = 1;

        foreach ( const ContractArg arg, event.getArguments() ) {
            QString val;
            if ( arg.indexed() ) {
                val = fTopics.at(t++).right(64);
            } else {
                val = data.mid(n, 64);
                n += 64;
            }

            if ( arg.dynamic() ) { // value holds "pointer" to data in data
                ulong ptr = arg.decodeInt(val, false).toUlong() * 2;
                val = data.mid(ptr);
                fParams.append(arg.decode(val));
            } else { // value is direct
                fParams.append(arg.decode(val));
            }
        }
    }

    const QString EventInfo::address() const {
        return fAddress;
    }

    const QString EventInfo::contract() const {
        return fContract;
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

    const ContractArgs EventInfo::getArguments() const {
        return fArguments;
    }

    const QVariantList EventInfo::getParams() const {
        return fParams;
    }

    const QString EventInfo::paramToStr(const QVariant& value) const {
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

    quint64 EventInfo::blockNumber() const {
        return fBlockNumber;
    }

    // ***************************** ContractInfo ***************************** //

    ContractInfo::ContractInfo(const QString &name, const QString& address, const QJsonArray &abi) :
        fName(name), fAddress(Helpers::vitalizeAddress(address)), fABI(abi), fFunctions()
    {
        parse();
    }

    ContractInfo::ContractInfo(const QJsonObject &source) {
        fName = source.value("name").toString();
        fAddress = Helpers::vitalizeAddress(source.value("address").toString());
        fABI = source.value("abi").toArray();
        fFunctions = ContractFunctionList();

        parse();
    }

    const QVariant ContractInfo::value(const int role) const {
        switch ( role ) {
            case ContractNameRole: return QVariant(fName);
            case AddressRole: return QVariant(fAddress);
            case ABIRole: return QVariant(QString(QJsonDocument(fABI).toJson()));
        }

        return QVariant();
    }

    const QJsonObject ContractInfo::toJson() const {
        QJsonObject result;
        result["name"] = fName;
        result["address"] = fAddress;
        result["abi"] = fABI;

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

    const ContractFunction ContractInfo::function(const QString& name) const {
        foreach ( const ContractFunction& func, fFunctions ) {
            if ( func.getName() == name ) {
                return func;
            }
        }

        throw QString("Function " + name + " not found");
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

}
