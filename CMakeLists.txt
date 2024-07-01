cmake_minimum_required(VERSION 3.15)

project(wave_equation_simulation)

set(CODEGEN_TOOL_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(EXECUTABLE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(engine)
add_subdirectory(game)
