TEMPLATE = app

QT += qml quick widgets network websockets

INCLUDEPATH += src
DEPENDPATH += src

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += hidapi-libusb protobuf libudev
}

win32 {
    INCLUDEPATH += C:\MinGW\msys\1.0\local\include
    LIBS += C:\MinGW\msys\1.0\local\lib\libprotobuf.a C:\MinGW\msys\1.0\local\lib\libhidapi.a -lhid -lsetupapi -lws2_32
    RC_ICONS = icon.ico
}

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/System/Library/Frameworks/IOKit.framework/Headers
    INCLUDEPATH += /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/System/Library/Frameworks/CoreFoundation.framework/Headers
    QMAKE_LFLAGS += -F/System/Library/Frameworks/CoreFoundation.framework -F/System/Library/Frameworks/IOKit.framework
    LIBS += -framework CoreFoundation
    LIBS += -framework IOKit
    LIBS += /usr/local/lib/libhidapi.a /usr/local/lib/libprotobuf.a
    ICON=qml/images/icon.icns
}

SOURCES += src/main.cpp \
    src/accountmodel.cpp \
    src/types.cpp \
    src/etheripc.cpp \
    src/settings.cpp \
    src/bigint.cpp \
    src/transactionmodel.cpp \
    src/clipboard.cpp \
    src/etherlog.cpp \
    src/currencymodel.cpp \
    src/accountproxymodel.cpp \
    src/gethlog.cpp \
    src/helpers.cpp \
    src/contractmodel.cpp \
    src/contractinfo.cpp \
    src/eventmodel.cpp \
    src/filtermodel.cpp \
    src/trezor/trezor.cpp \
    src/trezor/proto/messages.pb.cc \
    src/trezor/proto/config.pb.cc \
    src/trezor/proto/storage.pb.cc \
    src/trezor/proto/types.pb.cc \
    src/trezor/wire.cpp \
    src/trezor/hdpath.cpp \
    src/ethereum/tx.cpp \
    src/platform/devicemanager.cpp \
    src/remoteipc.cpp

RESOURCES += qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

# Default rules for deployment.
include(deployment.pri)

TRANSLATIONS += \
    i18n/etherwall.ts

lupdate_only {
    SOURCES += \
        qml/*.qml \
        qml/components/*.qml
}

HEADERS += \
    src/accountmodel.h \
    src/types.h \
    src/etheripc.h \
    src/settings.h \
    src/bigint.h \
    src/transactionmodel.h \
    src/clipboard.h \
    src/etherlog.h \
    src/currencymodel.h \
    src/accountproxymodel.h \
    src/gethlog.h \
    src/helpers.h \
    src/contractmodel.h \
    src/contractinfo.h \
    src/eventmodel.h \
    src/filtermodel.h \
    src/trezor/trezor.h \
    src/trezor/proto/messages.pb.h \
    src/trezor/proto/config.pb.h \
    src/trezor/proto/storage.pb.h \
    src/trezor/proto/types.pb.h \
    src/trezor/wire.h \
    src/trezor/hdpath.h \
    src/ethereum/tx.h \
    src/platform/devicemanager.h \
    src/remoteipc.h

