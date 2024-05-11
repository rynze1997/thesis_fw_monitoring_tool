find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include_directories(include/libs)
include_directories(include)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compact_fifo.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/compact_fifo_mutex.c)