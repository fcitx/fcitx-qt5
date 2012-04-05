#!/bin/sh

qdbusxml2cpp -N -p qfcitxinputcontextproxy -c QFcitxInputContextProxy interfaces/org.fcitx.Fcitx.InputContext.xml -i fcitxformattedpreedit.h
qdbusxml2cpp -N -p qfcitxinputmethodproxy -c QFcitxInputMethodProxy interfaces/org.fcitx.Fcitx.InputMethod.xml
qdbusxml2cpp -N -p qfreedesktopdbusproxy -c QFreedesktopDBusProxy interfaces/org.freedesktop.DBus.xml
