cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
if (NOT SYSBUILD)
    message(WARNING
        " This is multi-image application that should be build using sysbuild.\n"
        " Add --sysbuild argument to west build command to prepare all the images.")
endif()

project(monitoring-tool)

target_sources(app PRIVATE src/main.c)

# Include your project headers
target_include_directories(app PRIVATE include)
include_directories(include/libs)
include_directories(include/driver)

add_subdirectory(src/libs)
add_subdirectory(src/driver)