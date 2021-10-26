link_directories(
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
)

set(QUICKPHRASE_EDITOR_SRCS 
  main.cpp
  model.cpp
  editor.cpp
  editordialog.cpp
  batchdialog.cpp
  filelistmodel.cpp
  )
fcitx_translate_add_sources(
  ${QUICKPHRASE_EDITOR_SRCS})

include_directories(
  ${LIBINTL_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
  )
qt5_wrap_ui(QUICKPHRASE_EDITOR_SRCS editordialog.ui batchdialog.ui editor.ui)
add_library(fcitx-quickphrase-editor5
  MODULE ${QUICKPHRASE_EDITOR_SRCS})

set_target_properties(fcitx-quickphrase-editor5 PROPERTIES
                      LINK_FLAGS "-Wl,--no-undefined"
                      AUTOMOC TRUE
)
target_link_libraries(fcitx-quickphrase-editor5
  Qt5::Core
  Qt5::Gui
  Qt5::Concurrent
  FcitxQt5::WidgetsAddons
  LibIntl::LibIntl
  ${FCITX4_FCITX_UTILS_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
)

install(TARGETS fcitx-quickphrase-editor5 DESTINATION ${FCITX4_ADDON_INSTALL_DIR}/qt)
