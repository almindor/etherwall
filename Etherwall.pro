TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    accountmodel.cpp \
    types.cpp \
    etheripc.cpp \
    settings.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    accountmodel.h \
    types.h \
    etheripc.h \
    settings.h
