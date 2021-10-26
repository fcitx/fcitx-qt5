find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Widgets Concurrent)
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED Private)

set(FcitxQt5_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/FcitxQt5)
add_definitions(-DQT_NO_KEYWORDS)

if (ENABLE_LIBRARY)
add_subdirectory(dbusaddons)
add_subdirectory(widgetsaddons)
add_subdirectory(guiwrapper)
add_subdirectory(quickphrase-editor)
endif()

add_subdirectory(platforminputcontext)
