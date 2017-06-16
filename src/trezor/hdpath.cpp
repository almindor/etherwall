#include "hdpath.h"
#include <QDebug>

namespace Trezor {

    HDPath::HDPath(const QString &fullPath)
    {
        fIsValid = false;
        if ( !fullPath.startsWith('m') || fullPath.endsWith('/') ) {
            qDebug() << "Invalid HD path: " << fullPath << "\n";
            return;
        }

        QStringList segments = fullPath.split('/');
        if ( segments.size() <= 1 ) {
            qDebug() << "Invalid HD path: " << fullPath << "\n";
            return;
        }

        if ( segments.at(0) != "m" ) {
            qDebug() << "Invalid HD path: " << fullPath << "\n";
            return;
        }

        segments.removeFirst();
        foreach ( const QString segment, segments ) {
            quint32 num;
            if ( parseSegment(segment, num) ) {
                fSegments.append(num);
            } else {
                qDebug() << "Invalid HD path: " << fullPath << "\n";
                return;
            }
        }

        fPath = fullPath;
        fIsValid = true;
    }

    bool HDPath::getSegment(int index, quint32 &value) const
    {
        if ( !fIsValid ) {
            return false;
        }

        if ( fSegments.isEmpty() || index >= fSegments.size() ) {
            return false;
        }

        value = fSegments.at(index);
        return true;
    }

    const QString HDPath::toString() const
    {
        return fPath;
    }

    bool HDPath::valid() const
    {
        return fIsValid;
    }

    bool HDPath::parseSegment(const QString &segment, quint32& result) const
    {
        result = 0;
        QString val = segment;
        if ( val.endsWith("'") || val.endsWith('H', Qt::CaseInsensitive) ) {
            result = 0x80000000;
            val = val.left(segment.length() - 1);
        }

        bool ok = false;
        result += val.toUInt(&ok, 10);
        if ( !ok ) {
            qDebug() << "invalid segment: " << segment << "\n";
            return false;
        }

        return true;
    }

}
