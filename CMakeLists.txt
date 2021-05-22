cmake_minimum_required(VERSION 3.12)
project(poa VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} "src/poa.cpp")

add_library(alignment "src/alignment.cpp")
add_library(graph "src/graph.cpp")

if (NOT TARGET bioparser)
  add_subdirectory(bioparser EXCLUDE_FROM_ALL)
endif ()

target_link_libraries(alignment graph)

target_link_libraries(${PROJECT_NAME} alignment bioparser)

