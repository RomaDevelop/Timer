QT       += core gui widgets multimedia

CONFIG += c++17

SOURCES += \
    ../include/MyQWidgetLib.cpp \
    ../include/MyQWindows.cpp \
    TimerWindow.cpp \
    main.cpp

HEADERS += \
    ../include/MyQWidgetLib.h \
    ../include/MyQWindows.h \
    TimerWindow.h

DEPENDPATH += ../include

INCLUDEPATH += ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
