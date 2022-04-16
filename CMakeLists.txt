cmake_minimum_required(VERSION 3.1)

project(clapp)
add_library(clapp
    src/clapp.cpp)
target_include_directories(clapp PUBLIC
    include)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()