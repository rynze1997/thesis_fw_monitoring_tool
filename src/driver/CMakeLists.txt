find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include_directories(include/driver)
include_directories(include)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/usb.c)