cmake_minimum_required(VERSION 2.8)

project(fcitx-qt5)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5PlatformSupport REQUIRED)
find_package(Fcitx REQUIRED)
add_subdirectory(src)