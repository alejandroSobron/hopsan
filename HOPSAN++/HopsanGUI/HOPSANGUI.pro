# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
QT += svg
TARGET = HOPSANGUI
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    treewidget.cpp \
    componentguiclass.cpp \
    treewidgetitem.cpp \
    listwidget.cpp \
    listwidgetitem.cpp \
    ProjectTabWidget.cpp \
    LibraryWidget.cpp \
    GUIConnector.cpp \
    GUIPort.cpp
HEADERS += mainwindow.h \
    treewidget.h \
    componentguiclass.h \
    treewidgetitem.h \
    listwidget.h \
    listwidgetitem.h \
    ProjectTabWidget.h \
    LibraryWidget.h \
    ../HopsanCore/HopsanCore.h \
    GUIConnector.h \
    GUIPort.h
OTHER_FILES += 

# win32:DEFINES += STATICCORE
DESTDIR = ../bin/debug

# LIBS += -L../HopsanCore/bin/debug -lHopsanCore -Wl,-rpath,../../../HopsanCore/bin/debug
LIBS += -L../bin/debug \
    -lHopsanCore
INCLUDEPATH += ../HopsanCore
