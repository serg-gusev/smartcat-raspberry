#-------------------------------------------------
#
# Project created by QtCreator 2018-01-03T18:17:22
#
#-------------------------------------------------

QT       += core gui websockets widgets

TARGET = smartcat
TEMPLATE = app
LIBS += -lusb

SOURCES += main.cpp \
    widget.cpp \
    usb-client.cpp \
    application.cpp

HEADERS  += \
    widget.h \
    usb-client.h \
    application.h

FORMS    += \
    widget.ui

RESOURCES += \
    res.qrc
