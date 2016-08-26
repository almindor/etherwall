#include "contractinfo.h"
#include <QRegExp>
#include <QDebug>

namespace Etherwall {

    // ***************************** FilterInfo ***************************** //

    FilterInfo::FilterInfo(const QString& name, const QString& contract, const QStringList& topics, bool active) :
        fName(name), fContract(contract), fTopics(topics), fActive(active)
    {
    }

    FilterInfo::FilterInfo(const QJsonObject& source) {
        fName = source.value("name").toString("invalid");
        fContract = source.value("contract").toString("invalid");
        fTopics = source.value("topics").toString("invalid").split(",");
        fActive = source.value("active").toBool(false);
    }

    const QVariant FilterInfo::value(const int role) const {
        switch ( role ) {
            case FilterNameRole: return fName;
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

    ContractArg::ContractArg(const QString& name, const QString &literal) {
        const QRegExp typeMatcher("^([a-z]+)([0-9]*)x?([0-9]*)$");
        const QRegExp arrayMatcher("^(.*)\\[([0-9]*)\\]$");

        fName = name;
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

    const QString ContractArg::toString() const {
        return "Type: " + fType + " Length: " + QString::number(fLength) + " M: " + QString::number(fM) + " N: " + QString::number(fN);
    }

    const QVariantMap ContractArg::toVariantMap() const {
        QVariantMap result;
        result["name"] = fName;
        result["type"] = fType;
        result["length"] = fLength;
        result["placeholder"] = getPlaceholder();
        result["valrex"] = getValRex();

        return result;
    }

    bool ContractArg::dynamic() const {
        return (fBaseType == "string" || fBaseType == "bytes" || fLength == 0);
    }

    const QString ContractArg::encode(const QVariant& val, bool internal) const {
        // if we're an array of anything consider a string, split and encode individually
        if ( fLength >= 0 && !internal ) {
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
            return encode(val.toByteArray());
        }

        if ( fBaseType == "bool" ) {
            return encode(val.toBool());
        }

        throw QString(QString("Unknown type: ") + fBaseType);
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

        return encodeBytes(bytes);
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

    const QString ContractArg::encodeBytes(QByteArray bytes) {
        const QString size = encodeInt(bytes.size());
        int coef = bytes.size() / 64;
        int requestedSize = 32 * (coef + 1);

        while ( bytes.size() < requestedSize ) {
            bytes.append('\0');
        }

        return size + bytes.toHex();
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

    // ***************************** ContractFunction ***************************** //

    ContractFunction::ContractFunction(const QJsonObject& source)
    {
        fName = source.value("name").toString();
        fArguments = ContractArgs();
        fReturns = ContractArgs();
        fArgModel = QVariantList();

        const QJsonArray args = source.value("inputs").toArray();
        const QJsonArray rets = source.value("outputs").toArray();

        foreach ( QJsonValue arg, args ) {
            const ContractArg carg = ContractArg(getArgName(arg), getArgLiteral(arg));
            fArguments.append(carg);
            fArgModel.append(carg.toVariantMap());
        }

        foreach ( QJsonValue ret, rets ) {
            fReturns.append(ContractArg(getArgName(ret), getArgLiteral(ret)));
        }

        fSignature = buildSignature();
        fMethodID = QString(QCryptographicHash::hash(fSignature.toUtf8(), QCryptographicHash::Sha3_256).left(4).toHex());
    }

    const QString ContractFunction::getArgLiteral(const QJsonValue& arg) const {
        if ( !arg.isObject() ) {
            throw QString("Invalid argument");
        }

        QJsonObject argObj = arg.toObject();

        if ( !argObj.contains("type") ) {
            throw QString("Invalid argument");
        }

        return argObj.value("type").toString();
    }

    const QString ContractFunction::getArgName(const QJsonValue& arg) const {
        if ( !arg.isObject() ) {
            throw QString("Invalid argument");
        }

        QJsonObject argObj = arg.toObject();

        if ( !argObj.contains("name") ) {
            throw QString("Invalid argument");
        }

        return argObj.value("name").toString();
    }

    const QString ContractFunction::buildSignature() const {
        QString list;

        foreach ( const ContractArg arg, fArguments ) {
            list = list + arg.type() + ",";
        }
        list.remove(list.length() - 1, 1); // remove trailing ","

        return fName + "(" + list + ")";
    }

    const QString ContractFunction::getName() const {
        return fName;
    }

    const ContractArg ContractFunction::getArgument(int index) const {
        return fArguments.at(index);
    }

    const ContractArgs ContractFunction::getArguments() const {
        return fArguments;
    }

    const QVariantList ContractFunction::getArgModel() const {
        return fArgModel;
    }

    const QString ContractFunction::getMethodID() const {
        return fMethodID;
    }

    const QString ContractFunction::getSignature() const {
        return fSignature;
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

    // ***************************** ContractInfo ***************************** //

    ContractInfo::ContractInfo(const QString &name, const QString& address, const QJsonArray &abi) :
        fName(name), fAddress(address), fABI(abi), fFunctions()
    {
        parse();
    }

    ContractInfo::ContractInfo(const QJsonObject &source) {
        fName = source.value("name").toString();
        fAddress = source.value("address").toString();
        fABI = source.value("abi").toArray();
        fFunctions = ContractFunctionList();

        parse();
    }

    const QVariant ContractInfo::value(const int role) const {
        switch ( role ) {
            case ContractRoles::ContractNameRole: return QVariant(fName);
            case ContractRoles::AddressRole: return QVariant(fAddress);
            case ContractRoles::ABIRole: return QVariant(QString(QJsonDocument(fABI).toJson()));
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

    void ContractInfo::parse() {
        const QJsonArray source = abiJson();

        foreach ( const QJsonValue val, source ) {
            if ( !val.isObject() ) {
                throw QString("Invalid ABI argument: " + val.toString());
            }

            const QJsonObject obj = val.toObject();
            if ( obj.contains("type") && obj.value("type").toString() == "function" ) {
                fFunctions.append(ContractFunction(obj));
            }
        }
    }

}
