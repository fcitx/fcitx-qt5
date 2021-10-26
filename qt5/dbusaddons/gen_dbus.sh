#!/bin/sh

qdbusxml2cpp-qt5 -N -p fcitxqtinputcontextproxy -c FcitxQtInputContextProxy interfaces/org.fcitx.Fcitx.InputContext.xml -i fcitxqtformattedpreedit.h -i fcitxqtdbusaddons_export.h
qdbusxml2cpp-qt5 -N -p fcitxqtinputmethodproxy -c FcitxQtInputMethodProxy interfaces/org.fcitx.Fcitx.InputMethod.xml -i fcitxqtinputmethoditem.h -i fcitxqtdbusaddons_export.h
qdbusxml2cpp-qt5 -N -p fcitxqtkeyboardproxy -c FcitxQtKeyboardProxy interfaces/org.fcitx.Fcitx.Keyboard.xml -i fcitxqtkeyboardlayout.h -i fcitxqtdbusaddons_export.h
