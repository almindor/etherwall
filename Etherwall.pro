TEMPLATE = app

QT += qml quick widgets

INCLUDEPATH += src
DEPENDPATH += src

SOURCES += src/main.cpp \
    src/accountmodel.cpp \
    src/types.cpp \
    src/etheripc.cpp \
    src/settings.cpp \
    src/bigint.cpp \
    src/transactionmodel.cpp

RESOURCES += qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = qml

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/accountmodel.h \
    src/types.h \
    src/etheripc.h \
    src/settings.h \
    src/bigint.h \
    src/transactionmodel.h
