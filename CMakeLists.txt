cmake_minimum_required(VERSION 3.26)
project(Chip8)

set(CMAKE_CXX_STANDARD 20)

add_executable(Chip8 main.cpp
        chip8.cpp
        chip8.h
        opcodes.cpp
)
include_directories(/usr/local/include)
find_package(SFML 2.6 COMPONENTS system window graphics network audio REQUIRED)
include_directories(${SFML_INCLUDE_DIRS})
target_link_libraries(Chip8 sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
