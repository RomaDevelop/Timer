QT       += core gui widgets multimedia

CONFIG += c++17

SOURCES += \
    ../include/MyQWidgetLib.cpp \
    TimerWindow.cpp \
    main.cpp

HEADERS += \
    ../include/MyQWidgetLib.h \
    TimerWindow.h

INCLUDEPATH += ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
