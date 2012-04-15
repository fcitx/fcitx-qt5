cmake_minimum_required(VERSION 2.8)

project(fcitx-qt5)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(ICU REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5PlatformSupport REQUIRED)
find_package(Fcitx 4.2.3 REQUIRED)

add_subdirectory(src)
