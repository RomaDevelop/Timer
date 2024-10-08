QT       += core gui widgets multimedia

CONFIG += c++17

SOURCES += \
    TimerWindow.cpp \
    main.cpp

HEADERS += \
	TimerWindow.h

INCLUDEPATH += ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
