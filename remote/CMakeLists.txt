cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(monitoring-tool-network)

target_sources(app PRIVATE src/main.c)

add_subdirectory(src/driver)
add_subdirectory(src/libs)
add_subdirectory(src/settings)

include_directories(include/driver)
include_directories(include/libs)
include_directories(include/settings)
include_directories(include)
