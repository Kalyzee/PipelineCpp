#/*
#-----------------------------------------------------------------------------
#Filename:    CMakeLists.txt
#-----------------------------------------------------------------------------
#*/
cmake_minimum_required(VERSION 2.6)
 
project(PipelineCpp)
 
set(CMAKE_DEBUG_POSTFIX "_d")
 
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/bin")

#Find all the required dependencies
#find_package(Boost REQUIRED system thread)

set(DEFAULT_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(DEFAULT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/includes)
set(DEFAULT_LINK_DIR ${CMAKE_SOURCE_DIR}/lib)
set(DEFAULT_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

#Define all the include directories
include_directories(
    ${DEFAULT_INCLUDE_DIR}
)

add_definitions(-std=c++11)
#add_definitions(-D_USE_BOOST)

link_directories(${DEFAULT_LINK_DIR})
add_subdirectory(src)
add_subdirectory(samples)
