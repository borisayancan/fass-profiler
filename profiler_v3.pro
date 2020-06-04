#-------------------------------------------------
#
# Project created by QtCreator 2020-06-02T17:31:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = profiler_v3
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        fmain.cpp \
        fass/fass_profiler.cpp \
        maths/algoritmos.cpp \
    tipomatlab/imshow.cpp \
    tipomatlab/plot.cpp

HEADERS += \
        fmain.h \
    fass/fass.h \
    tipos.h \
    maths/math_tipos.h \
    fass/fass_profiler.h \
    maths/algoritmos.h \
    tipomatlab/tipomatlab.h \
    tipomatlab/imshow.h \
    tipomatlab/plot.h

FORMS += \
        fmain.ui \
    tipomatlab/plot.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix:LIBS += -lgsl

win32:DEFINES += _USE_MATH_DEFINES
win32:INCLUDEPATH += ../gsl/include
win32:LIBS += ../gsl/lib/gsl.lib ../gsl/lib/gslcblas.lib
