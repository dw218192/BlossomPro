cmake_minimum_required(VERSION 3.8)

project(libspline)

# Add the library target
add_library(libspline STATIC aaCurve.cpp aaCurve.h spline.cpp spline.h)

# Specify include directories
target_include_directories(libspline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
