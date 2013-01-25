set(fcitx_qt_SOURCES
    fcitxqtconnection.cpp
    fcitxqtformattedpreedit.cpp
    fcitxqtinputcontextproxy.cpp
    fcitxqtinputmethodproxy.cpp
    fcitxqtkeyboardlayout.cpp
    fcitxqtkeyboardproxy.cpp
    fcitxqtinputmethoditem.cpp
    )

set(fcitx_qt_HEADERS
    fcitxqtconnection.h
    fcitxqt_export.h
    fcitxqtformattedpreedit.h
    fcitxqtinputcontextproxy.h
    fcitxqtinputmethodproxy.h
    fcitxqtkeyboardlayout.h
    fcitxqtkeyboardproxy.h
    fcitxqtinputmethoditem.h
)

include_directories(${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5DBus_INCLUDE_DIRS}
)

qt5_wrap_cpp(
    fcitx_qt_MOC_SRCS
    ${fcitx_qt_HEADERS}
    fcitxqtconnection_p.h
)

add_library(fcitx-qt5 SHARED ${fcitx_qt_SOURCES} ${fcitx_qt_MOC_SRCS})
set_target_properties(fcitx-qt5
                      PROPERTIES VERSION 0.1
                      SOVERSION 0
                      COMPILE_FLAGS "-fvisibility=hidden"
                      LINK_FLAGS "-Wl,--no-undefined"
)

target_link_libraries(
    fcitx-qt5
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

install(TARGETS fcitx-qt5 LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

configure_file(fcitx-qt5.pc.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-qt5.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-qt5.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(FILES ${fcitx_qt_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fcitx-qt5")


