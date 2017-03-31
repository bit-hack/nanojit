cmake_minimum_required(VERSION 2.8)
project(nanojit)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(nj_lib)
add_subdirectory(nj_driver)
add_subdirectory(nj_exe_vm)
# add_subdirectory(nj_exe_vm)
