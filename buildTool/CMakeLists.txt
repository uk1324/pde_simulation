project(buildTool)

add_executable(buildTool "main.cpp")
target_compile_features(buildTool PUBLIC cxx_std_23)