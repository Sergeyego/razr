#-------------------------------------------------
#
# Project created by QtCreator 2014-09-09T11:04:09
#
#-------------------------------------------------

QT       += serialbus serialport sql printsupport
CONFIG += qwt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#include ( /usr/local/qwt-6.1.3/features/qwt.prf )

LIBS += -lqwt-qt5
INCLUDEPATH += /usr/include/qt5/qwt

TARGET = raz_05
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    press.cpp \
    tenz.cpp \
    laser.cpp \
    tearingmach.cpp \
    partiform.cpp \
    cbrelationdelegate.cpp \
    dbtablemodel.cpp \
    viewer.cpp

HEADERS  += mainwidget.h \
    press.h \
    tenz.h \
    laser.h \
    tearingmach.h \
    partiform.h \
    cbrelationdelegate.h \
    dbtablemodel.h \
    viewer.h

FORMS    += mainwidget.ui \
    partiform.ui

LIBS     += -lgsl \
            -lgslcblas

RESOURCES += \
    res.qrc
