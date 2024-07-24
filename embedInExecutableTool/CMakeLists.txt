project(embedTool)
add_executable(embedTool "main.cpp" "../engine/engine/Utils/PreprocessIncludes.cpp" "../engine/engine/Utils/FileIo.cpp" "../engine/engine/Utils/CrashReport.cpp" "../engine/engine/Utils/Assertions.cpp" )
target_compile_features(embedTool PUBLIC cxx_std_23)

target_include_directories(embedTool PUBLIC "../engine" "../engine/engine/Utils")