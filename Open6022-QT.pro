#-------------------------------------------------
#
# Project created by QtCreator 2016-03-04T07:43:15
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Open6022-QT
TEMPLATE = app
INCLUDEPATH += /usr/local/include/libusb-1.0
LIBS += -L/usr/local/lib
LIBS +=-lusb-1.0
LIBS +=-lm

SOURCES += main.cpp\
        mainwindow.cpp \
    HT6022fw.c \
    HT6022.c \
    worker.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    HT6022fw.h \
    HT6022.h \
    worker.h \
    qcustomplot.h

FORMS    += mainwindow.ui
