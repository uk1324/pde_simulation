add_executable(simulation "main.cpp")

target_link_libraries(simulation PUBLIC engine)

target_compile_features(simulation PUBLIC cxx_std_20)
set_target_properties(simulation PROPERTIES CXX_EXTENSIONS OFF)