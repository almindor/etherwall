TEMPLATE = app

QT += qml quick widgets network websockets
# CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += src src/ew-node/src
DEPENDPATH += src src/ew-node/src

linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += hidapi-libusb libusb-1.0 protobuf libudev
}

win32 {
    INCLUDEPATH += C:\msys64\mingw64\include
    # PKGCONFIG += hidapi libusb-1.0 protobuf libudev
    LIBS += -lprotobuf -lusb-1.0 -lhidapi -lsetupapi -lws2_32
    RC_ICONS = icon.ico
}

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/IOKit.framework/Headers
    INCLUDEPATH += /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreFoundation.framework/Headers
    QMAKE_LFLAGS += -F/System/Library/Frameworks/CoreFoundation.framework -F/System/Library/Frameworks/IOKit.framework
    LIBS += -framework CoreFoundation
    LIBS += -framework IOKit
    LIBS += /usr/local/lib/libhidapi.a /usr/local/lib/libprotobuf.a /usr/local/lib/libusb-1.0.a
    ICON=qml/images/icon.icns
}

SOURCES += src/main.cpp \
    src/accountmodel.cpp \
    src/settings.cpp \
    src/transactionmodel.cpp \
    src/clipboard.cpp \
    src/currencymodel.cpp \
    src/accountproxymodel.cpp \
    src/contractmodel.cpp \
    src/contractinfo.cpp \
    src/eventmodel.cpp \
    src/filtermodel.cpp \
    src/trezor/trezor.cpp \
    src/trezor/proto/messages.pb.cc \
    src/trezor/proto/messages-common.pb.cc \
    src/trezor/proto/messages-management.pb.cc \
    src/trezor/proto/messages-ethereum.pb.cc \
    src/trezor/wire.cpp \
    src/trezor/hdpath.cpp \
    src/platform/devicemanager.cpp \
    src/initializer.cpp \
    src/tokenmodel.cpp \
    src/ew-node/src/etherlog.cpp \
    src/ew-node/src/gethlog.cpp \
    src/ew-node/src/helpers.cpp \
    src/ew-node/src/types.cpp \
    src/ew-node/src/ethereum/tx.cpp \
    src/ew-node/src/ethereum/bigint.cpp \
    src/ew-node/src/nodeipc.cpp \
    src/ew-node/src/nodews.cpp \
    src/gethlogapp.cpp \
    src/etherlogapp.cpp \
    src/ew-node/src/networkchainmanager.cpp \
    src/nodemanager.cpp

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
    src/settings.h \
    src/transactionmodel.h \
    src/clipboard.h \
    src/currencymodel.h \
    src/accountproxymodel.h \
    src/contractmodel.h \
    src/contractinfo.h \
    src/eventmodel.h \
    src/filtermodel.h \
    src/trezor/trezor.h \
    src/trezor/proto/messages.pb.h \
    src/trezor/proto/messages-common.pb.h \
    src/trezor/proto/messages-management.pb.h \
    src/trezor/proto/messages-ethereum.pb.h \
    src/trezor/wire.h \
    src/trezor/hdpath.h \
    src/platform/devicemanager.h \
    src/initializer.h \
    src/tokenmodel.h \
    src/cert.h \
    src/ew-node/src/types.h \
    src/ew-node/src/etherlog.h \
    src/ew-node/src/gethlog.h \
    src/ew-node/src/helpers.h \
    src/ew-node/src/nodeipc.h \
    src/ew-node/src/nodews.h \
    src/ew-node/src/ethereum/bigint.h \
    src/ew-node/src/ethereum/tx.h \
    src/ew-node/src/ethereum/keccak.h \
    src/gethlogapp.h \
    src/etherlogapp.h \
    src/ew-node/src/networkchainmanager.h \
    src/nodemanager.h

