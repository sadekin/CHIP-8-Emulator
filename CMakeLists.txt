cmake_minimum_required(VERSION 3.26)
project(Chip8)

set(CMAKE_CXX_STANDARD 20)

# Manually set SDL2 paths
set(SDL2_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_LIBRARIES "/opt/homebrew/lib/libSDL2.dylib")

# Manually set SDL2_ttf paths
set(SDL2_TTF_INCLUDE_DIRS "/opt/homebrew/include/SDL2")
set(SDL2_TTF_LIBRARIES "/opt/homebrew/lib/libSDL2_ttf.dylib")

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})

add_executable(Chip8 main.cpp
        chip8.cpp
        chip8.h
        opcodes.cpp)

target_link_libraries(Chip8 ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})