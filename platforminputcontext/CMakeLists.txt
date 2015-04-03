include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})


set(plugin_SRCS
    qfcitxplatforminputcontext.cpp
    keyuni.cpp
    main.cpp
    utils.cpp
)

set(plugin_MOC_HDRS
    qfcitxplatforminputcontext.h
    main.h
)

add_library(fcitxplatforminputcontextplugin MODULE ${plugin_SRCS})
set_target_properties(fcitxplatforminputcontextplugin PROPERTIES
                         AUTOMOC TRUE
                         COMPILE_FLAGS "-fvisibility=hidden"
                         LINK_FLAGS "-Wl,--no-undefined"
                        )

target_link_libraries(fcitxplatforminputcontextplugin
                          Qt5::Core
                          Qt5::Gui
                          Qt5::DBus
                          FcitxQt5::DBusAddons
                          XKBCommon::XKBCommon
                         )

include(ECMQueryQmake)
query_qmake(CMAKE_INSTALL_QTPLUGINDIR QT_INSTALL_PLUGINS)
install(TARGETS fcitxplatforminputcontextplugin DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/platforminputcontexts)
