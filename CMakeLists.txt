cmake_minimum_required(VERSION 3.1)

project(fcitx-qt5)

set(FcitxQt5_VERSION 1.1.0)
set(REQUIRED_QT_VERSION 5.1.0)
set(REQUIRED_QT6_VERSION 6.0.0)

find_package(ECM 1.4.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(ENABLE_QT5 "Enable Qt5" On)
option(ENABLE_QT6 "Enable Qt6 im module" Off)
option(ENABLE_LIBRARY "Qt library" On)

include(GNUInstallDirs)
include(FeatureSummary)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=gnu++17" SUPPORT_CXX17)

if(NOT SUPPORT_CXX11)
    message(FATAL_ERROR "need c++ 17 compatible compiler to compile")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -Wall")

find_package(XKBCommon 0.5.0 REQUIRED COMPONENTS XKBCommon)

if (ENABLE_LIBRARY)
find_package(Fcitx 4.2.8 REQUIRED)
find_package(LibIntl REQUIRED)
add_subdirectory(po)
endif ()

if(ENABLE_QT5)
    add_subdirectory(qt5)
endif()

if(ENABLE_QT6)
    add_subdirectory(qt6)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
