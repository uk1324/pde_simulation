add_executable(simulation "main.cpp" "MainLoop.cpp" "Demos/PoissonEquationSolver.cpp" "Demos/PoissonEquationDemo.cpp" "Textures.cpp" "Demos/HeatEquationDemo.cpp" "PlotUtils.cpp"  "Simulation.cpp" "GridUtils.cpp" "Box2d.cpp" "Editor.cpp" "GameRenderer.cpp" "Constants.cpp" "EditorActions.cpp" "EditorEntities.cpp" "StackAllocator.cpp" "Shared.cpp" "Gizmo.cpp" "SimulationSettings.cpp" "ProgramSettings.cpp" "RelativePositions.cpp" "InputButton.cpp")

target_link_libraries(simulation PUBLIC engine)

target_compile_features(simulation PUBLIC cxx_std_23)
set_target_properties(simulation PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(simulation PUBLIC "../" "../dependencies/box2c/include")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(simulation PUBLIC gfx2d)
target_link_libraries(simulation PUBLIC box2d)

target_compile_options(simulation PRIVATE /we4062)

# targetAddGeneratedDependencies(simulation "../engine/gfx2d")
targetAddGenerated(simulation ${CMAKE_CURRENT_SOURCE_DIR})