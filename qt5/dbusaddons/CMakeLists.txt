ecm_setup_version(${FcitxQt5_VERSION} VARIABLE_PREFIX FCITXQT5DBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitxqtdbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5DBusAddonsConfigVersion.cmake"
                        SOVERSION 1)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/FcitxQt5DBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/FcitxQt5DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5DBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5DBusAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5DBusAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT FcitxQt5DBusAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE FcitxQt5DBusAddonsTargets.cmake NAMESPACE FcitxQt5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitxqtdbusaddons_version.h
        DESTINATION ${FcitxQt5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

set(dbusaddons_SOURCES
    fcitxqtconnection.cpp
    fcitxqtformattedpreedit.cpp
    fcitxqtinputcontextproxy.cpp
    fcitxqtinputmethodproxy.cpp
    fcitxqtkeyboardlayout.cpp
    fcitxqtkeyboardproxy.cpp
    fcitxqtinputmethoditem.cpp
    )

set(dbusaddons_HEADERS
    fcitxqtconnection.h
    fcitxqtformattedpreedit.h
    fcitxqtinputcontextproxy.h
    fcitxqtinputmethodproxy.h
    fcitxqtkeyboardlayout.h
    fcitxqtkeyboardproxy.h
    fcitxqtinputmethoditem.h
)

set(fcitxqtdbusaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(FcitxQt5DBusAddons SHARED ${dbusaddons_SOURCES})
generate_export_header(FcitxQt5DBusAddons BASE_NAME FcitxQtDBusAddons)
add_library(FcitxQt5::DBusAddons ALIAS FcitxQt5DBusAddons)

target_include_directories(FcitxQt5DBusAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtdbusaddons_INCLUDE_DIRS}>")
target_include_directories(FcitxQt5DBusAddons INTERFACE "$<INSTALL_INTERFACE:${FcitxQt5_INCLUDE_INSTALL_DIR}/FcitxQtDBusAddons>")

set_target_properties(FcitxQt5DBusAddons
                      PROPERTIES VERSION 1.0
                      AUTOMOC TRUE
                      SOVERSION 1
                      COMPILE_FLAGS "-fvisibility=hidden"
                      LINK_FLAGS "-Wl,--no-undefined"
                      EXPORT_NAME DBusAddons
)

target_link_libraries(
    FcitxQt5DBusAddons
    PUBLIC
        Qt5::Core
        Qt5::DBus
)

install(TARGETS FcitxQt5DBusAddons EXPORT FcitxQt5DBusAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${dbusaddons_HEADERS}
              ${CMAKE_CURRENT_BINARY_DIR}/fcitxqtdbusaddons_export.h
              DESTINATION "${FcitxQt5_INCLUDE_INSTALL_DIR}/FcitxQtDBusAddons")


