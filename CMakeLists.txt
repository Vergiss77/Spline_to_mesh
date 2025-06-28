cmake_minimum_required(VERSION 3.20.0)
project(Spline_to_mesh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

find_package(assimp)
if(NOT ${ASSIMP_LIBRARIES} STREQUAL "")
    set(ASSIMP_FOUND TRUE)
endif()
find_package(gismo REQUIRED)
include_directories(${GISMO_INCLUDE_DIRS})
link_directories(${GISMO_LIBRARY_DIR})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(Spline_to_mesh ${SOURCES})
target_link_libraries(Spline_to_mesh PUBLIC gismo)
if(ASSIMP_FOUND)
    target_link_libraries(Spline_to_mesh PUBLIC ${ASSIMP_LIBRARIES})
    target_compile_definitions(Spline_to_mesh PUBLIC ASSIMP_USE)
endif()
