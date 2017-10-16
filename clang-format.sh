#!/bin/sh
find . \( -not \( -name '*proxy.h' -o -name '*proxy.cpp' \) \) -a \( -name '*.h' -o -name '*.cpp' \)  | xargs clang-format -i
