add_executable(simulation "main.cpp" "MainLoop.cpp" "Demos/PoissonEquationSolver.cpp" "Demos/PoissonEquationDemo.cpp" "Textures.cpp" "Demos/HeatEquationDemo.cpp" "PlotUtils.cpp"  "Simulation.cpp" "GridUtils.cpp" "Box2d.cpp" "Editor.cpp" "GameRenderer.cpp" "Constants.cpp" "EditorActions.cpp" "EditorEntities.cpp" "StackAllocator.cpp" "Shared.cpp" "Gizmo.cpp" "SimulationSettings.cpp" "ProgramSettings.cpp" "RelativePositions.cpp" "InputButton.cpp" "ParametricEllipse.cpp" "Demos/WaveEquationDemo.cpp" "ShapeVertices.cpp" "ParametricParabola.cpp" "SimulationDisplay3d.cpp" "Camera3d" "Serialization/Level.cpp" "FileSelectWidget.cpp")

target_link_libraries(simulation PUBLIC engine)

target_compile_features(simulation PUBLIC cxx_std_23)
set_target_properties(simulation PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(simulation PUBLIC "../" "../dependencies/box2c/include")
target_include_directories(simulation PUBLIC "../" "../engine/dependencies/")
target_include_directories(simulation PUBLIC "../" "../dependencies/Clipper2/CPP/Clipper2Lib/include")

include("../engine/codeGenTool/targetAddGenerated.cmake")

target_link_libraries(simulation PUBLIC gfx2d)
target_link_libraries(simulation PUBLIC box2d)
target_link_libraries(simulation PUBLIC Clipper2)

target_compile_options(simulation PRIVATE /we4062)

targetAddGenerated(simulation ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on then the console logs won't show
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(simulation PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()

if (FINAL_RELEASE)
	set(EMBED_TOOL "$<TARGET_FILE:embedTool>")
	add_custom_command(
		TARGET simulation PRE_BUILD
		COMMAND ${EMBED_TOOL}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		DEPENDS ${EMBED_TOOL}
	)
	set(BUILD_TOOL "$<TARGET_FILE:buildTool>")
	set(targetName "simulation")
	add_custom_command(
		TARGET ${targetName} POST_BUILD
		COMMAND ${BUILD_TOOL} $<TARGET_FILE:simulation>
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		DEPENDS ${BUILD_TOOL}
	)
endif()