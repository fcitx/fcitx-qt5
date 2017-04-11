link_directories(
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
)

ecm_setup_version(${FcitxQt5_VERSION} VARIABLE_PREFIX FCITXQT5WIDGETSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/fcitxqtwidgetsaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5WidgetsAddonsConfigVersion.cmake"
                        SOVERSION 1)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/FcitxQt5WidgetsAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/FcitxQt5WidgetsAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5WidgetsAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5WidgetsAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/FcitxQt5WidgetsAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT FcitxQt5WidgetsAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE FcitxQt5WidgetsAddonsTargets.cmake NAMESPACE FcitxQt5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitxqtwidgetsaddons_version.h
        DESTINATION ${FcitxQt5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

set(widgetsaddons_SOURCES
    fcitxqtconfiguifactory.cpp
    fcitxqtconfiguiplugin.cpp
    fcitxqtconfiguiwidget.cpp
    fcitxqtkeysequencewidget.cpp
    qtkeytrans.cpp
    )

set(widgetsaddons_HEADERS
    fcitxqtconfiguifactory.h
    fcitxqtconfiguiplugin.h
    fcitxqtconfiguiwidget.h
    fcitxqtkeysequencewidget.h
)

set(fcitxqtwidgetsaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
)

add_library(FcitxQt5WidgetsAddons SHARED ${widgetsaddons_SOURCES})
generate_export_header(FcitxQt5WidgetsAddons BASE_NAME FcitxQtWidgetsAddons)
add_library(FcitxQt5::WidgetsAddons ALIAS FcitxQt5WidgetsAddons)

target_include_directories(FcitxQt5WidgetsAddons PUBLIC "$<BUILD_INTERFACE:${fcitxqtwidgetsaddons_INCLUDE_DIRS}>")
target_include_directories(FcitxQt5WidgetsAddons INTERFACE "$<INSTALL_INTERFACE:${FcitxQt5_INCLUDE_INSTALL_DIR}/FcitxQtWidgetsAddons>")

set_target_properties(FcitxQt5WidgetsAddons
                      PROPERTIES VERSION 1.0
                      AUTOMOC TRUE
                      SOVERSION 1
                      COMPILE_FLAGS "-fvisibility=hidden"
                      LINK_FLAGS "-Wl,--no-undefined"
                      EXPORT_NAME WidgetsAddons
)

target_link_libraries(FcitxQt5WidgetsAddons
    PUBLIC
        Qt5::Core
        Qt5::Widgets
    PRIVATE
        ${FCITX4_FCITX_UTILS_LIBRARIES}
        ${FCITX4_FCITX_CONFIG_LIBRARIES}
)

install(TARGETS FcitxQt5WidgetsAddons EXPORT FcitxQt5WidgetsAddonsTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(FILES ${widgetsaddons_HEADERS}
              ${CMAKE_CURRENT_BINARY_DIR}/fcitxqtwidgetsaddons_export.h
              DESTINATION "${FcitxQt5_INCLUDE_INSTALL_DIR}/FcitxQtWidgetsAddons")
