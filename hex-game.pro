TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += thread

QMAKE_CXXFLAGS += -pthread

SOURCES += main.cpp
