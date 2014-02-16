# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Release/CLang-Linux-x86
TARGET = QTournament
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += release 
PKGCONFIG +=
QT = core gui widgets
SOURCES += main.cpp ui/MainFrame.cpp
HEADERS += ui/MainFrame.h
FORMS += ui/MainFrame.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Release/CLang-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = clang
QMAKE_CXX = clang++
DEFINES += 
INCLUDEPATH += 
LIBS += -Wl,-rpath,../DatabaseOverlayLib/dist/Release/CLang-Linux-x86 ../DatabaseOverlayLib/dist/Release/CLang-Linux-x86/libDatabaseOverlayLib.so.0.1.0  
