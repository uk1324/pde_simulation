add_executable(simulation "main.cpp" "MainLoop.cpp" "PoissonEquationSolver.cpp" "PoissonEquationDemo.cpp" "Textures.cpp")

target_link_libraries(simulation PUBLIC engine)

target_compile_features(simulation PUBLIC cxx_std_23)
set_target_properties(simulation PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(simulation PUBLIC "../")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(simulation PUBLIC gfx2d)
# targetAddGeneratedDependencies(simulation "../engine/gfx2d")
targetAddGenerated(simulation ${CMAKE_CURRENT_SOURCE_DIR})