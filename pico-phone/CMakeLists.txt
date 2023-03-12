cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# include(pico_extras_import_optional.cmake)

project(pico_phone C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
#     message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
# endif()

pico_sdk_init()

file(GLOB CORE_SRC ${PROJECT_SOURCE_DIR}/core/*.c)
file(GLOB_RECURSE PLATFORM_SRC ${PROJECT_SOURCE_DIR}/platform/*.c)
add_executable(phone ${CORE_SRC} ${PLATFORM_SRC})

pico_enable_stdio_usb(phone 1)
pico_enable_stdio_uart(phone 0)

# pull in common dependencies
target_link_libraries(phone pico_stdlib)

target_include_directories(phone PRIVATE ${PROJECT_SOURCE_DIR}/core)
target_include_directories(phone PRIVATE ${PROJECT_SOURCE_DIR}/platform)

# create map/bin/hex file etc.
pico_add_extra_outputs(phone)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

