#ifndef HDPATH_H
#define HDPATH_H

#include <QString>
#include <QList>

namespace Trezor {

    class HDPath
    {
    public:
        HDPath(const QString& fullPath);
        bool getSegment(int index, quint32& value) const;
        const QString toString() const;
        bool valid() const;
    private:
        QList<quint32> fSegments;
        bool fIsValid;
        QString fPath;

        bool parseSegment(const QString& segment, quint32& result) const;
    };

}

#endif // HDPATH_H
