cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(AutoMeasure LANGUAGES CXX)

set(proj_exe "measure")

add_subdirectory("widget")

add_subdirectory("logger")

add_subdirectory("basler")

add_subdirectory("algorithm")




