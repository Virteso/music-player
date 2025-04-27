# CMake file for windows CLI build

cmake_minimum_required(VERSION 3.16)

project(MusicPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MusicPlayerCLI src/cli_main.cpp)

# Path to VLC headers
target_include_directories(MusicPlayerCLI PRIVATE
        ${CMAKE_SOURCE_DIR}/vlc/windows/include
)

# Path to VLC .dll.a libraries
target_link_directories(MusicPlayerCLI PRIVATE
        ${CMAKE_SOURCE_DIR}/vlc/windows)

# Link against VLC libraries
target_link_libraries(MusicPlayerCLI PRIVATE
        libvlc libvlccore
)

set(VLC_PLUGIN_PATH "${CMAKE_SOURCE_DIR}/vlc/windows/plugins")
set(ENV{VLC_PLUGIN_PATH} ${VLC_PLUGIN_PATH})

# Copy VLC DLLs
add_custom_command(TARGET MusicPlayerCLI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/vlc/windows/libvlc.dll"
        "${CMAKE_SOURCE_DIR}/vlc/windows/libvlccore.dll"
        $<TARGET_FILE_DIR:MusicPlayerCLI>
)

# Copy plugins directory
add_custom_command(TARGET MusicPlayerCLI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/vlc/windows/plugins"
        "$<TARGET_FILE_DIR:MusicPlayerCLI>/plugins"
)
