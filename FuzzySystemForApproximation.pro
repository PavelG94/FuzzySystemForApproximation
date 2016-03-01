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
LIBS += -lgtest -lpthread

SOURCES += \
    MLS.cpp \
    qcustomplot.cpp \
    Main.cpp \
    TestsOfMLS.cpp \
    UnaryFunc.cpp \
    HoughTransform.cpp \
    TestsOfHoughTransform.cpp \
    SugenoCntl.cpp \
    TestsOfSugenoCntl.cpp \
    MainWindow.cpp \
    CntlBuilder.cpp

HEADERS  += \
    MLS.h \
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
