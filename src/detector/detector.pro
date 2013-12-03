#-------------------------------------------------
#
# Project created by QtCreator 2013-08-05T13:28:45
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = detector
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L/home/raca/repo/outside_projects/chilitags/build/src/lib -lchilitags -L/usr/local/lib -lopencv_core -lopencv_highgui
INCLUDEPATH += /home/raca/repo/outside_projects/chilitags/src/lib/include /usr/local/include

SOURCES += \
    detector.cpp
