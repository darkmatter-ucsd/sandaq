cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(SandixProcessor)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/daqtest/Processor/boost_1_81_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/daqtest/Processor/boost_1_81_0/libs")

find_package(ROOT CONFIG REQUIRED)
find_package(NumCpp 0.0.0 REQUIRED)
find_package(Boost 1.68.0 REQUIRED)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(process main.cc ${sources})
add_executable(processRun processRun.cc ${sources})
target_include_directories(process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(processRun PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(process ROOT::Tree NumCpp::NumCpp Boost::boost)
target_link_libraries(processRun ROOT::Tree NumCpp::NumCpp Boost::boost)


add_custom_target(SandixProcessor DEPENDS process processRun)
target_compile_features(processRun PRIVATE cxx_std_17)