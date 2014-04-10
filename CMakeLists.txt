cmake_minimum_required(VERSION 2.8.5)

project(fcitx-qt5)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5DBus REQUIRED)
include(GNUInstallDirs)

get_target_property(REAL_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)

execute_process(COMMAND ${REAL_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
OUTPUT_VARIABLE _Qt5_PLUGIN_INSTALL_DIR ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (NOT _Qt5_PLUGIN_INSTALL_DIR)
    message(FATAL_ERROR "not found qt5 plugin dir")
endif (NOT _Qt5_PLUGIN_INSTALL_DIR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(fcitx-qt5)
add_subdirectory(src)

