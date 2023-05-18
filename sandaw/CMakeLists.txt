cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(SandixProcessor)
find_package(ROOT CONFIG REQUIRED)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/daqtest/Processor/boost_1_81_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/daqtest/Processor/boost_1_81_0/libs")

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(process main.cc ${sources})
target_include_directories(process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(process ROOT::Tree)

add_custom_target(SandixProcessor DEPENDS process process)
target_compile_features(process PRIVATE cxx_std_17)