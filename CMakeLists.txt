# Copyright 2023 Lucas Klassmann
# License: Apache License 2.0
cmake_minimum_required(VERSION 3.5)
project(enemy_ai)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
set(CMAKE_CXX_FLAGS "-Wall -std=c++0x -ggdb ${CMAKE_CXX_FLAGS}")

find_package(SDL2 REQUIRED)
find_package(Lua REQUIRED)


include_directories(
        ${PROJECT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
)


set(SOURCE_FILES
        main.c
)

# LUA SCRIPTS
configure_file("enemy_ai.lua" "enemy_ai.lua")

add_executable(enemy_ai ${SOURCE_FILES})
target_link_libraries(enemy_ai
        ${SDL2_LIBRARY}
        ${SDL2_TTF_LIBRARY}
        ${SDL2_IMAGE_LIBRARY}
        ${SDL2_MIXER_LIBRARY}
        ${LUA_LIBRARIES}
)