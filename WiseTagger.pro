# Copyright © 2014 cat <cat@wolfgirl.org>
# This program is free software. It comes without any warranty, to the extent
# permitted by applicable law. You can redistribute it and/or modify it under
# the terms of the Do What The Fuck You Want To Public License, Version 2, as
# published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
#
#-------------------------------------------------
#
# Project created by QtCreator 2014-01-14 17:50:00
#
#-------------------------------------------------

QT += core gui network widgets

win32 {
    QT += winextras
}

TARGET = WiseTagger

QMAKE_TARGET_COMPANY = catgirl
QMAKE_TARGET_PRODUCT = WiseTagger
QMAKE_TARGET_DESCRIPTION = Simple picture tagger
QMAKE_TARGET_COPYRIGHT = cat@wolfgirl.org

VERSION = "0.4.7"
Release:DEFINES += QT_NO_DEBUG_OUTPUT

DEFINES +=                                           \
    APP_VERSION=\\\"$$VERSION\\\"                    \
    TARGET_PRODUCT=\\\"$$QMAKE_TARGET_PRODUCT\\\"    \
    TARGET_COMPANY=\\\"$$QMAKE_TARGET_COMPANY\\\"

QMAKE_CXXFLAGS += -std=c++14

PRECOMPILED_HEADER += util/precompiled.h

SOURCES +=                                           \
    $$PWD/src/*.cpp                                  \
    $$PWD/util/*.cpp

HEADERS  +=                                          \
    $$PWD/src/*.h                                    \
    $$PWD/util/*.h

RESOURCES +=                                         \
    resources.qrc
