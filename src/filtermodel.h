#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QObject>
#include <QAbstractListModel>

namespace Etherwall {

    class FilterModel : public QAbstractListModel
    {
    public:
        FilterModel();
    };

}

#endif // FILTERMODEL_H
