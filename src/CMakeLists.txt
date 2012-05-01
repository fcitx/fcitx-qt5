include_directories(${Qt5Core_INCLUDE_DIRS}
                       ${Qt5Gui_INCLUDE_DIRS}
                       ${Qt5DBus_INCLUDE_DIRS}
                       ${Qt5PlatformSupport_INCLUDE_DIRS}
)

find_path(Qt5PlatformSupport_INCLUDE_DIRECTORY
           NAMES QtPlatformSupportVersion
           HINTS ${Qt5PlatformSupport_INCLUDE_DIRS})

include_directories(${Qt5PlatformSupport_INCLUDE_DIRECTORY}/${Qt5PlatformSupport_VERSION}/QtPlatformSupport)


set(plugin_SRCS
    qfcitxplatforminputcontext.cpp
    qfcitxinputcontextproxy.cpp
    qfcitxinputmethodproxy.cpp
    qfreedesktopdbusproxy.cpp
    fcitxformattedpreedit.cpp
    keyuni.cpp
    main.cpp
)

set(plugin_MOC_HDRS
    qfcitxplatforminputcontext.h
    qfcitxinputcontextproxy.h
    qfcitxinputmethodproxy.h
    qfreedesktopdbusproxy.h
    main.h
)

qt5_wrap_cpp(plugin_MOCS ${plugin_MOC_HDRS})

add_library(fcitxplatforminputcontextplugin MODULE ${plugin_SRCS} ${plugin_MOCS})
set_target_properties(fcitxplatforminputcontextplugin PROPERTIES
                         COMPILE_FLAGS "-fvisibility=hidden"
                         LINK_FLAGS "-Wl,--no-undefined"
                        )

target_link_libraries(fcitxplatforminputcontextplugin
                          ${Qt5Core_LIBRARIES}
                          ${Qt5Gui_LIBRARIES}
                          ${Qt5DBus_LIBRARIES}
                          ${Qt5PlatformSupport_LIBRARIES}
                          ${ICU_LIBRARIES}
                         )

install(TARGETS fcitxplatforminputcontextplugin DESTINATION ${_qt5_install_prefix}/plugins/platforminputcontexts/)