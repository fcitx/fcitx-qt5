include_directories(${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5DBus_INCLUDE_DIRS}
)

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
                          fcitx-qt5
                         )

install(TARGETS fcitxplatforminputcontextplugin DESTINATION ${_Qt5_PLUGIN_INSTALL_DIR}/platforminputcontexts)
