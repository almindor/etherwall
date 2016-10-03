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
/** @file currencymodel.h
 * @author Ales Katona <almindor@gmail.com>
 * @date 2016
 *
 * Currency model body
 */

#include "currencymodel.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace Etherwall {

    CurrencyModel::CurrencyModel() : QAbstractListModel(0), fIndex(0), fTimer()
    {
        connect(&fNetManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(loadCurrenciesDone(QNetworkReply*)));
        loadCurrencies();

        fTimer.setInterval(300 * 1000); // once per 5m, update currency prices
        connect(&fTimer, &QTimer::timeout, this, &CurrencyModel::loadCurrencies);
        fTimer.start();
    }

    QHash<int, QByteArray> CurrencyModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "name";
        roles[PriceRole] = "price";
        return roles;
    }

    int CurrencyModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const {
        return fCurrencies.size();
    }

    QVariant CurrencyModel::data(const QModelIndex & index, int role __attribute__ ((unused))) const {
        return QVariant(fCurrencies.at(index.row()).value(role));
    }

    QString CurrencyModel::getCurrencyName(int index) const {
        if ( index < 0 ) {
            index = fIndex;
        }

        if ( fCurrencies.size() > index && index >= 0 ) {
            return fCurrencies.at(index).value(NameRole).toString();
        }

        return "UNK";
    }

    QVariant CurrencyModel::recalculate(const QVariant ether) const {
        if ( fIndex == 0 ) {
            return ether; // no change
        }

        double val = fCurrencies.at(fIndex).recalculate(ether.toDouble());
        return QVariant(QString::number(val, 'f', 18));
    }

    int CurrencyModel::getCount() const {
        return fCurrencies.size();
    }

    void CurrencyModel::loadCurrencies() {
        fCurrencies.clear();
        fCurrencies.append(CurrencyInfo("ETH", 1.0));

        // get currency data from etherdata
        QNetworkRequest request(QUrl("https://data.etherwall.com/api/currencies"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject objectJson;
        QJsonArray currencies;
        currencies.append(QJsonValue(QString("BTC")));
        currencies.append(QJsonValue(QString("EUR")));
        currencies.append(QJsonValue(QString("CAD")));
        currencies.append(QJsonValue(QString("USD")));
        currencies.append(QJsonValue(QString("GBP")));
        objectJson["currencies"] = currencies;
        objectJson["version"] = 2;
        const QByteArray data = QJsonDocument(objectJson).toJson();

        EtherLog::logMsg("HTTP Post request: " + data, LS_Debug);

        fNetManager.post(request, data);
    }

    void CurrencyModel::loadCurrenciesDone(QNetworkReply *reply) {
        if ( reply == NULL ) {
            return EtherLog::logMsg("Undefined currency reply", LS_Error);
        }

        beginResetModel();

        const QByteArray data = reply->readAll();
        EtherLog::logMsg("HTTP Post reply: " + data, LS_Debug);

        QJsonParseError parseError;
        const QJsonDocument resDoc = QJsonDocument::fromJson(data, &parseError);

        if ( parseError.error != QJsonParseError::NoError ) {
            return EtherLog::logMsg("Response parse error: " + parseError.errorString(), LS_Error);
        }

        const QJsonObject resObj = resDoc.object();
        const bool success = resObj.value("success").toBool(false);

        if ( !success ) {
            return reply->close();
        }

        const QJsonObject c = resObj.value("currencies").toObject();
        const QJsonArray d = c.value("Data").toArray();

        foreach ( const QJsonValue p, d ) {
            const QString key = p.toObject().value("Symbol").toString("bogus");
            const float value = p.toObject().value("Price").toVariant().toFloat(0);
            fCurrencies.append(CurrencyInfo(key, value));
        }

        reply->close();

        endResetModel();
        emit currencyChanged();
    }

    void CurrencyModel::setCurrencyIndex(int index) {
        if ( index >= 0 && index < fCurrencies.length() ) {
            fIndex = index;
            emit currencyChanged();
        }
    }

    int CurrencyModel::getCurrencyIndex() const {
        return fIndex;
    }

    double CurrencyModel::getCurrencyPrice(int index) const {
        if ( fCurrencies.size() > index && index >= 0 ) {
            return fCurrencies.at(index).recalculate(1.0);
        }

        return 1.0;
    }

}
