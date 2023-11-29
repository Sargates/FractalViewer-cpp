cmake_minimum_required(VERSION 3.22)

project(FractalViewer)

add_subdirectory(${CMAKE_HOME_DIRECTORY}/external/SDL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)


add_executable(FractalViewer main.cpp)

target_link_libraries(FractalViewer PUBLIC SDL2)
