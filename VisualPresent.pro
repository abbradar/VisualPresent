#-------------------------------------------------
#
# Project created by QtCreator 2010-10-10T23:05:19
#
#-------------------------------------------------

QT       += core gui network

TARGET = VisualPresent
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
    vpserver.cpp \
    vpclient.cpp

HEADERS  += mainwindow.h \
    vpserver.h \
    vpclient.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    doc/TODO.txt \
    doc/IDEAS.txt \
    doc/BUGS.txt
