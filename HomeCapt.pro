#-------------------------------------------------
#
# Project created by QtCreator 2019-05-29T21:23:13
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = HomeCapt
TEMPLATE = app
PREFIX = /usr/
target.path = $$PREFIX/bin
CONFIG += static

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 1.0

SOURCES += \
        main.cpp \
        homecapt.cpp \
    homecaptapi.cpp \
    locationmaker.cpp \
    sensormaker.cpp \
    qcustomplot.cpp \
    qplot.cpp

HEADERS += \
        homecapt.h \
    homecaptapi.h \
    locationmaker.h \
    sensormaker.h \
    qcustomplot.h \
    qplot.h

FORMS += \
        homecapt.ui \
    locationmaker.ui \
    sensormaker.ui

RESOURCES += \
    ressources.qrc

DISTFILES += \
    logo.svg \
    homecapt.desktop

logo.files = logo.svg
logo.path = /usr/share/homecapt/images/

desktop.files= homecapt.desktop
desktop.path = /usr/share/applications/

ICON = logo.svg

INSTALLS += target

unix:INSTALLS += logo desktop

