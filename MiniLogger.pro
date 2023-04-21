TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt

mac: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

SOURCES += \
        main.cpp

HEADERS += \
    logger.hpp
