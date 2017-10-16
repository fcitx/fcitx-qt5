include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})


set(plugin_SRCS
    fcitxinputcontextproxy.cpp
    fcitxqtdbustypes.cpp
    fcitxwatcher.cpp
    qfcitxplatforminputcontext.cpp
    main.cpp
    qtkey.cpp
)

set_source_files_properties(org.fcitx.Fcitx.InputContext.xml PROPERTIES
INCLUDE fcitxqtdbustypes.h)
set_source_files_properties(org.fcitx.Fcitx.InputContext1.xml PROPERTIES
INCLUDE fcitxqtdbustypes.h)
set_source_files_properties(org.fcitx.Fcitx.InputMethod.xml PROPERTIES
INCLUDE fcitxqtdbustypes.h)
set_source_files_properties(org.fcitx.Fcitx.InputMethod1.xml PROPERTIES
INCLUDE fcitxqtdbustypes.h)

qt5_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputContext.xml inputcontextproxy)
qt5_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputContext1.xml inputcontext1proxy)
qt5_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputMethod.xml inputmethodproxy)
qt5_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputMethod1.xml inputmethod1proxy)

add_library(fcitxplatforminputcontextplugin MODULE ${plugin_SRCS})
set_target_properties(fcitxplatforminputcontextplugin PROPERTIES
                         AUTOMOC TRUE
                         COMPILE_FLAGS "-fvisibility=hidden"
                         LINK_FLAGS "-Wl,--no-undefined"
                        )
target_include_directories(fcitxplatforminputcontextplugin
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                          )
target_link_libraries(fcitxplatforminputcontextplugin
                          Qt5::Core
                          Qt5::Gui
                          Qt5::DBus
                          XKBCommon::XKBCommon
                         )

include(ECMQueryQmake)
if (NOT CMAKE_INSTALL_QTPLUGINDIR)
query_qmake(CMAKE_INSTALL_QTPLUGINDIR QT_INSTALL_PLUGINS)
endif()
install(TARGETS fcitxplatforminputcontextplugin DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/platforminputcontexts)
