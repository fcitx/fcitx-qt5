cmake_minimum_required(VERSION 3.1)

project(fcitx-qt5)

set(FcitxQt5_VERSION 1.1.0)
set(REQUIRED_QT_VERSION 5.1.0)

find_package(ECM 1.4.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(ENABLE_LIBRARY "Qt library" On)

include(GNUInstallDirs)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++11" SUPPORT_CXX11)

if(NOT SUPPORT_CXX11)
    message(FATAL_ERROR "need c++ 11 compatible compiler to compile")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Widgets Concurrent)
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED Private)
find_package(XKBCommon 0.5.0 REQUIRED COMPONENTS XKBCommon)

if (ENABLE_LIBRARY)
find_package(Fcitx 4.2.8 REQUIRED)
find_package(LibIntl REQUIRED)
endif ()

set(FcitxQt5_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR}/FcitxQt5)

if (ENABLE_LIBRARY)
add_subdirectory(dbusaddons)
add_subdirectory(widgetsaddons)
add_subdirectory(guiwrapper)
add_subdirectory(quickphrase-editor)
add_subdirectory(po)
endif()

add_subdirectory(platforminputcontext)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
