cmake_minimum_required(VERSION 3.15)

set(FINAL_RELEASE TRUE)

project(wave_equation_simulation)

if (FINAL_RELEASE)
	add_definitions(-DFINAL_RELEASE)
endif()


set(CODEGEN_TOOL_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(EXECUTABLE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(dependencies/box2c)
add_subdirectory(dependencies/Clipper2/CPP)
add_subdirectory(embedInExecutableTool)
add_subdirectory(buildTool)