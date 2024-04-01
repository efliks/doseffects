cmake_minimum_required(VERSION 3.13)

project(dumydemo.exe)

set(GCC_COVERAGE_COMPILE_FLAGS "-W -Wall -pedantic")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable(dumydemo.exe bumpmain.c bumpobj.c bumptri.c envmaps.c flag3d.c fontnew.c gfx.c main.c math3d.c matrix.c object3d.c pcx.c polygon.c raytrace.c scroll.c sphere.c triangle.c)
add_subdirectory("dummy")

target_link_libraries(dumydemo.exe PUBLIC m)
