cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(SandixProcessor)

find_package(ROOT CONFIG REQUIRED)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(process main.cc ${sources})
target_include_directories(process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(process ROOT::Tree)

add_custom_target(SandixProcessor DEPENDS process)
