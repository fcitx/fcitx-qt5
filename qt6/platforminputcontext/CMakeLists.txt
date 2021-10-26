include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})


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

qt6_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputContext.xml inputcontextproxy)
qt6_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputContext1.xml inputcontext1proxy)
qt6_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputMethod.xml inputmethodproxy)
qt6_add_dbus_interface(plugin_SRCS org.fcitx.Fcitx.InputMethod1.xml inputmethod1proxy)

add_library(fcitxplatforminputcontextplugin-qt6 MODULE ${plugin_SRCS})
set_target_properties(fcitxplatforminputcontextplugin-qt6 PROPERTIES
                         AUTOMOC TRUE
                         COMPILE_FLAGS "-fvisibility=hidden"
                         LINK_FLAGS "-Wl,--no-undefined"
                        )
target_include_directories(fcitxplatforminputcontextplugin-qt6
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                          )
target_link_libraries(fcitxplatforminputcontextplugin-qt6
                          Qt6::Core
                          Qt6::Gui
                          Qt6::DBus
                          XKBCommon::XKBCommon
                         )

get_target_property(_QT6_QMAKE_EXECUTABLE Qt6::qmake LOCATION)
execute_process(
    COMMAND ${_QT6_QMAKE_EXECUTABLE} -query "QT_INSTALL_PLUGINS"
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE _QT6PLUGINDIR
)
if(return_code EQUAL 0)
    string(STRIP "${_QT6PLUGINDIR}" _QT6PLUGINDIR)
else()
    message(FATAL_ERROR "QMake Qt6 call failed: ${return_code}")
endif()
set(CMAKE_INSTALL_QT6PLUGINDIR ${_QT6PLUGINDIR} CACHE PATH "Qt6 plugin dir")
install(TARGETS fcitxplatforminputcontextplugin-qt6 DESTINATION ${CMAKE_INSTALL_QT6PLUGINDIR}/platforminputcontexts)
