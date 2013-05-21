# -------------------------------------------------
# Project created by QtCreator 2010-01-16T18:33:13
# -------------------------------------------------
QT += network
TARGET = LanPartyManager
TEMPLATE = app
SOURCES += main.cpp \
    widget.cpp \
    computer.cpp \
    CFtpServer.cpp
HEADERS += widget.h \
    computer.h \
    CFtpServer.h
LIBS += -L".\lib" -lwsock32
RESOURCES     = resource.qrc
RC_FILE = myapp.rc

