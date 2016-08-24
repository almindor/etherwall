#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>
#include <QAbstractListModel>

namespace Etherwall {

    class EventModel : public QAbstractListModel
    {
    public:
        EventModel();
    };

}

#endif // EVENTMODEL_H
