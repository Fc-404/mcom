cmake_minimum_required(VERSION 3.19)
project(mcom LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets
            SerialPort Svg)

# 链接窗口库
add_subdirectory(party/qwindowkit-1.4.0)

qt_standard_project_setup()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/inc/*.hpp
    ${CMAKE_SOURCE_DIR}/src/widget/*.hpp
    ${CMAKE_SOURCE_DIR}/src/utli/*.hpp
)
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/ui/*.ui
)
file(GLOB_RECURSE QRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/res/*.qrc
)
include_directories(${CMAKE_SOURCE_DIR}/src/inc ${CMAKE_SOURCE_DIR}/src/widget)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/mcom.rc)
endif()
qt_add_executable(mcom
    # WIN32
    ${SRC_FILES}
    ${UI_FILES}
    ${QRC_FILES}
    # mcom.rc
)

target_link_libraries(mcom
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
        Qt::Svg
        QWindowKit::Core
        QWindowKit::Widgets
        dwmapi
)
