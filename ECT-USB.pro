#-------------------------------------------------
#
# Project created by QtCreator 2016-09-03T19:16:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ECT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    compute.cpp \
    cuicapcolu.cpp \
    cuiconline.cpp \
    cuiconrect.cpp \
    cuiconcir.cpp \
    communication.cpp \
    Qled.cpp \
    usbthread.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    compute.h \
    cuicapcolu.h \
    cuiconline.h \
    cuiconrect.h \
    cuiconcir.h \
    communication.h \
    Qled.h \
    usbthread.h \
    cyapi/CyAPI.h \
    option.h

FORMS    += mainwindow.ui \
    capcolu.ui \
    conline.ui \
    conrect.ui \
    concir.ui \
    communication.ui

RESOURCES += \
    icon.qrc

DISTFILES +=
LIBS += -L$$PWD/cyapi/ -lCyAPI
INCLUDEPATH += $$PWD/cyapi
DEPENDPATH += $$PWD/cyapi

NODEFAULTLIB:library  #因为引入外部库，所以库类型与默认不同
