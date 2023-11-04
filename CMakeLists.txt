cmake_minimum_required(VERSION 3.26)
project(Chip8)

set(CMAKE_CXX_STANDARD 20)

add_executable(Chip8 main.cpp
        chip8.cpp
        chip8.h
        opcodes.cpp
        emulator.h
        emulator.cpp
)

# SFML
find_package(SFML 2.6 COMPONENTS system window graphics network audio REQUIRED)

# TGUI
find_package(TGUI 1.0 REQUIRED)

target_include_directories(Chip8 PRIVATE ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
target_link_libraries(Chip8 PRIVATE sfml-system sfml-window sfml-graphics sfml-audio sfml-network TGUI::TGUI)
