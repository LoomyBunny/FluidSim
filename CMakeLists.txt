cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK initialization script
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(my_project)

# Initialize the Pico SDK
pico_sdk_init()

# Add your executable and source files
add_executable(my_project
    fluid-sim.cpp
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(my_project 1)
pico_enable_stdio_uart(my_project 0)

# Link the Pico SDK to your project
target_link_libraries(my_project pico_stdlib hardware_i2c pico_multicore)

# Add the root directory to the include path
target_include_directories(my_project PRIVATE ${CMAKE_SOURCE_DIR})

# Enable all compiler warnings
target_compile_options(my_project PRIVATE -Wall)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(my_project)

