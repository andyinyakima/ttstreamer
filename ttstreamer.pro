#-------------------------------------------------
#
# Project created by QtCreator 2019-04-04T12:10:40
#
#-------------------------------------------------

QT       += core


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets gui multimedia multimediawidgets


TARGET = ttstreamer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        tts.cpp

HEADERS  += tts.h

FORMS    += tts.ui
CONFIG += c++11


# Clang Code Model was a nuisance after Qt Creator 4.11.0 Qt 5.12.8
#Go into Help->About Plugins and uncheck the Clang code model plugin.
#You have to restart qt creator after that to make it take effect.
#I had the same problem when debugging a unit test. I was debugging code
#not in the project itself and it was dying repeatedly on me sending tons of kernel messages.
