find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include_directories(include/settings)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/settings.c)