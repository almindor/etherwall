#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QThread>
#include <QTimer>

#ifdef Q_OS_MACX
#include <CoreFoundation.h>
#include <usb/IOUSBLib.h>
#include <mach/mach.h>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#include <winuser.h>
#include <QAbstractNativeEventFilter>
#endif
#ifdef Q_OS_LINUX
#include <libudev.h>
#include <fcntl.h>
#endif

namespace Etherwall {

#ifdef Q_OS_WIN32
    class DeviceManager;

    class WindowsUSBFilter: public QAbstractNativeEventFilter
    {
    public:
        WindowsUSBFilter(DeviceManager& owner);
        virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    private:
        DeviceManager& fOwner;
    };
#endif

    class DeviceManager : public QThread
    {
        Q_OBJECT
    public:
        explicit DeviceManager(QApplication& app);
        virtual ~DeviceManager();
        void run();
    signals:
        void deviceInserted() const;
        void deviceRemoved() const;
    public slots:
        void startProbe();
    private:
#ifdef Q_OS_MACX
        IONotificationPortRef    fNotifyPort;
        io_iterator_t            fRawAddedIter;
        io_iterator_t            fRawRemovedIter;
#endif
#ifdef Q_OS_WIN32
        WindowsUSBFilter fFilter;
#endif
#ifdef Q_OS_LINUX
        struct udev* fUdev;
        struct udev_monitor* fUdevMonitor;
#endif
    };

}

#endif // DEVICEMANAGER_H
