cmake_minimum_required(VERSION 3.16)

project(MusicPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64/lib/cmake")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# VLC Paths
set(VLC_DIR "${CMAKE_SOURCE_DIR}/vlc/windows")
set(VLC_INCLUDE_DIR "${VLC_DIR}/include")
set(VLC_LIB_DIR "${VLC_DIR}")
set(VLC_DLL_DIR "${VLC_DIR}")
set(VLC_PLUGIN_DIR "${VLC_DIR}/plugins")

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
)

add_executable(MusicPlayer WIN32
    ${PROJECT_SOURCES}
)

# Include and link VLC
target_include_directories(MusicPlayer PRIVATE ${VLC_INCLUDE_DIR})
target_link_directories(MusicPlayer PRIVATE ${VLC_LIB_DIR})
target_link_libraries(MusicPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets libvlc libvlccore)

set_target_properties(MusicPlayer PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Set plugin path for runtime
set(VLC_PLUGIN_PATH "${VLC_PLUGIN_DIR}")
set(ENV{VLC_PLUGIN_PATH} ${VLC_PLUGIN_PATH})

# Copy VLC DLLs and plugins folder to the build folder after building
add_custom_command(TARGET MusicPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VLC_DLL_DIR}/libvlc.dll"
        "${VLC_DLL_DIR}/libvlccore.dll"
        "$<TARGET_FILE_DIR:MusicPlayer>"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VLC_PLUGIN_DIR}"
        "$<TARGET_FILE_DIR:MusicPlayer>/plugins"
)
