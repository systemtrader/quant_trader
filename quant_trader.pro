QT += core dbus
QT -= gui

TARGET = quant_trader
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    quant_trader.cpp \
    bar_collector.cpp \
    strategy/DblMaPsar_strategy.cpp \
    indicator/abstract_indicator.cpp \
    indicator/ma.cpp \
    indicator/parabolicsar.cpp \
    indicator/mql5_indicator.cpp \
    strategy/abstract_strategy.cpp \
    bar.cpp

DBUS_INTERFACES += market_watcher.xml ctp_executer.xml

HEADERS += \
    quant_trader.h \
    bar_collector.h \
    bar.h \
    strategy/DblMaPsar_strategy.h \
    indicator/abstract_indicator.h \
    indicator/ma.h \
    indicator/parabolicsar.h \
    mql5_compatible.h \
    indicator/mql5_indicator.h \
    strategy/abstract_strategy.h

win32:INCLUDEPATH += "$$_PRO_FILE_PWD_"

unix:QMAKE_CXXFLAGS += -std=c++11
