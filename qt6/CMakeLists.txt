find_package(Qt6 ${REQUIRED_QT6_VERSION} CONFIG REQUIRED Core DBus)
find_package(Qt6Gui ${REQUIRED_QT6_VERSION} REQUIRED Private)

add_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(platforminputcontext)
