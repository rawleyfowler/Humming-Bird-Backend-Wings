cmake_minimum_required(VERSION 3.9)

# 100% compatability with Rakudo NativeCall
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Other stuff
project(wings VERSION 0.0.1 DESCRIPTION "A High-Performance backend for Humming-Bird")
add_library(wings SHARED wings.cpp http.cpp utils.cpp)
set_target_properties(wings PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(wings PROPERTIES SOVERSION 1)
target_include_directories(wings PRIVATE .)
install(
  TARGETS wings
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
