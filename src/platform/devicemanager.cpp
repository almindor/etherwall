#include "devicemanager.h"
#include <QDebug>

namespace Etherwall {

#ifdef Q_OS_MACX

    DeviceManager::DeviceManager(QApplication& app) : QThread(0)
    {
        Q_UNUSED(app);
        // SEE https://developer.apple.com/library/content/documentation/DeviceDrivers/Conceptual/USBBook/USBDeviceInterfaces/USBDevInterfaces.html#//apple_ref/doc/uid/TP40002645-BBIDDHCI
    }

    DeviceManager::~DeviceManager()
    {
        terminate();
        wait(5000);
    }

    void RawDeviceAdded(void *refCon, io_iterator_t iterator) {
        kern_return_t   kr;
        io_service_t    object;

        while ( (object = IOIteratorNext(iterator)) )
        {
            kr = IOObjectRelease(object);
            if (kr != kIOReturnSuccess)
            {
                qDebug() << "Couldn’t release raw device object: " << kr;
                continue;
            }

            if (refCon == nullptr) {
                continue;
            }

            DeviceManager* manager = (DeviceManager*) refCon;
            emit manager->deviceInserted();
            refCon = nullptr; // finish loop but don't emit again
        }
    }

    void RawDeviceRemoved(void *refCon, io_iterator_t iterator) {
        kern_return_t   kr;
        io_service_t    object;

        while ( (object = IOIteratorNext(iterator)) )
        {
            kr = IOObjectRelease(object);
            if (kr != kIOReturnSuccess)
            {
                qDebug() << "Couldn’t release raw device object: " << kr;
                continue;
            }

            if (refCon == nullptr) {
                continue;
            }

            DeviceManager* manager = (DeviceManager*) refCon;
            emit manager->deviceRemoved();
            refCon = nullptr; // finish loop but don't emit again
        }
    }

    void DeviceManager::run()
    {
        while ( true ) {
            sleep(2);
            emit deviceInserted();
        }

        // The following works on detection but somehow breaks hidapi and hid_enumerate gets nothing afterwards

        mach_port_t             masterPort;
        CFMutableDictionaryRef  matchingDict;
        CFRunLoopSourceRef      runLoopSource;
        kern_return_t           kr;
        SInt32                  usbVendor = 0x534c;
        SInt32                  usbProduct = 0x0001;

        //Create a master port for communication with the I/O Kit
        kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
        if (kr || !masterPort)
        {
            qDebug() << "ERR: Couldn’t create a master I/O Kit port: " << kr << "\n";
            return;
        }
        //Set up matching dictionary for class IOUSBDevice and its subclasses
        matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
        if (!matchingDict)
        {
            qDebug() << "Couldn’t create a USB matching dictionary\n";
            mach_port_deallocate(mach_task_self(), masterPort);
            return;
        }

        //Add the vendor and product IDs to the matching dictionary.
        //This is the second key in the table of device-matching keys of the
        //USB Common Class Specification

        CFDictionarySetValue(matchingDict, CFSTR(kUSBVendorName),
                            CFNumberCreate(kCFAllocatorDefault,
                                         kCFNumberSInt32Type, &usbVendor));
        CFDictionarySetValue(matchingDict, CFSTR(kUSBProductName),
                            CFNumberCreate(kCFAllocatorDefault,
                                        kCFNumberSInt32Type, &usbProduct));

        //To set up asynchronous notifications, create a notification port and
        //add its run loop event source to the program’s run loop
        fNotifyPort = IONotificationPortCreate(masterPort);
        runLoopSource = IONotificationPortGetRunLoopSource(fNotifyPort);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,
                            kCFRunLoopDefaultMode);

        //Retain additional dictionary references because each call to
        //IOServiceAddMatchingNotification consumes one reference
        matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);
        matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);
        // matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);

        //Now set up two notifications: one to be called when a raw device
        //is first matched by the I/O Kit and another to be called when the
        //device is terminated
        //Notification of first match:
        kr = IOServiceAddMatchingNotification(fNotifyPort,
                        kIOFirstMatchNotification, matchingDict,
                        RawDeviceAdded, this, &fRawAddedIter);
        RawDeviceAdded(nullptr, fRawAddedIter);

        //Notification of termination:
        kr = IOServiceAddMatchingNotification(fNotifyPort,
                        kIOTerminatedNotification, matchingDict,
                        RawDeviceRemoved, this, &fRawRemovedIter);
        RawDeviceRemoved(nullptr, fRawRemovedIter);

        mach_port_deallocate(mach_task_self(), masterPort);
        masterPort = 0;

        //Start the run loop so notifications will be received
        qDebug() << "Running notification loop\n";
        CFRunLoopRun();
    }

    void DeviceManager::startProbe() {
        start();
        emit deviceInserted(); // we only get changes so check initially
    }

#endif

#ifdef Q_OS_WIN32

    WindowsUSBFilter::WindowsUSBFilter(DeviceManager &owner) :
        fOwner(owner)
    {
    }

    bool WindowsUSBFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
        Q_UNUSED(eventType);
        Q_UNUSED(result);

        MSG *msg = static_cast<MSG*>(message);
        if ( msg->message == WM_DEVICECHANGE ) {
            emit fOwner.deviceInserted(); // insert/remove same in windblows
        }

        return false;
    }

    // filter

    DeviceManager::DeviceManager(QApplication& app) : QThread(0),
        fFilter(*this)
    {
        app.installNativeEventFilter(&fFilter);
    }

    DeviceManager::~DeviceManager()
    {

    }

    void DeviceManager::run()
    {
        // nothing as we're using windows events
    }

    void DeviceManager::startProbe()
    {
        emit deviceInserted(); // initial check as we don't get a change if it's already inserted
    }
#endif

#ifdef Q_OS_LINUX
    DeviceManager::DeviceManager(QApplication& app) : QThread(nullptr)
    {
        Q_UNUSED(app)
        fUdev = udev_new();
        fUdevMonitor = udev_monitor_new_from_netlink(fUdev, "udev");
    }

    DeviceManager::~DeviceManager()
    {
        terminate();
        wait(5000);

        if ( fUdev != nullptr ) {
            if ( fUdevMonitor != nullptr ) {
                udev_monitor_unref(fUdevMonitor);
                fUdevMonitor = nullptr;
            }

            udev_unref(fUdev);
            fUdev = nullptr;
        }
    }

    void DeviceManager::run()
    {
        /* Set up a monitor to monitor hidraw devices */
        udev_monitor_filter_add_match_subsystem_devtype(fUdevMonitor, "hidraw", nullptr);
        udev_monitor_enable_receiving(fUdevMonitor);
        // make FD blocking
        int fd = udev_monitor_get_fd(fUdevMonitor);
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags & !O_NONBLOCK);

        while (1) {
            struct udev_device* dev = udev_monitor_receive_device(fUdevMonitor);
            if (dev) {
                const QString action = QString::fromLatin1(udev_device_get_action(dev));
                if ( action == "add" ) {
                    emit deviceInserted();
                } else if ( action == "remove" ) {
                    emit deviceRemoved();
                } else {
                    qDebug() << "Unknown udev device action: " << action << "\n";
                }
                udev_device_unref(dev);
            }
            else {
                udev_device_unref(dev);
                qDebug() << "No Device from receive_device(). An error occured.\n";
            }
        }
    }

    void DeviceManager::startProbe()
    {
        emit deviceInserted();
        start();
    }
#endif
}
