# ==============================================================================
# Simple Keypad
# Target: Raspberry Pi Pico (RP2040)
# ==============================================================================

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(simple_keypad C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


# Main executable
add_executable(simple_keypad
    main.cpp
)


# Required Pico SDK and TinyUSB libraries
target_link_libraries(simple_keypad PRIVATE
    pico_stdlib
)

# Select stdio either over USB or UART
pico_enable_stdio_usb(simple_keypad 1)
pico_enable_stdio_uart(simple_keypad 0)


# Generate UF2, HEX, and bin outputs in addition to ELF
pico_add_extra_outputs(simple_keypad)
