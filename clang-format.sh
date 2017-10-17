#!/bin/sh
find . \( -not \( -name 'fcitxqt*proxy.h' -o -name 'fcitxqt*proxy.cpp' \) \) -a \( -name '*.h' -o -name '*.cpp' \)  | xargs clang-format -i
