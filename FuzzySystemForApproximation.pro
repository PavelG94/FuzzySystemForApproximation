#-------------------------------------------------
#
# Project created by QtCreator 2016-02-12T12:43:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = FuzzySystemForApproximation
TEMPLATE = app

CONFIG += c++11
CONFIG += warn_on   #выдавать все возможные предупреждения
LIBS += -lpthread -larmadillo

SOURCES += \
    qcustomplot.cpp \
    Main.cpp \
    UnaryFunc.cpp \
    HoughTransform.cpp \
    SugenoCntl.cpp \
    MainWindow.cpp \
    CntlBuilder.cpp

HEADERS  += \
    qcustomplot.h \
    UnaryFunc.h \
    UnaryFuncBase.h \
    HoughTransform.h \
    SugenoCntl.h \
    MainWindow.h \
    CntlBuilder.h

DISTFILES += \
    Outlines \
    README.md \
    Conclusions

FORMS +=
