cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if (WIN32)
    set(JLINK_DEIVCE_DIR $ENV{APPDATA}/Segger/JLinkDevices CACHE STRING "JlinkDevices目录")
endif()

set(BOARD_MANFACTURER ALL CACHE STRING "板制造商")
set(BOARD_NAME ALL CACHE STRING "板型号")
set(DRIVER_ALL_ENABLE FALSE CACHE BOOL "启用所有驱动")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel")
endif()

set(CMAKE_PROJECT_NAME DAMIAO_MC02_OFL)

include("cmake/gcc-arm-none-eabi.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(SEGGER-OpenFlashLoader C ASM)

message("Build type: " ${CMAKE_BUILD_TYPE})
message("JlinkDevices PATH: " ${JLINK_DEIVCE_DIR})

file(GLOB DRIVER_DIRS LIST_DIRECTORIES true drivers/*)
foreach(DRIVER_DIR IN LISTS DRIVER_DIRS)
    get_filename_component(DRIVER_NAME ${DRIVER_DIR} NAME)
    message("Add Driver: " ${DRIVER_NAME})
    add_subdirectory(${DRIVER_DIR})
endforeach()

if (${BOARD_MANFACTURER} STREQUAL "ALL")
    file(GLOB BOARD_MANFACTURER_DIRS LIST_DIRECTORIES true boards/*)
else()
    list(TRANSFORM BOARD_MANFACTURER PREPEND "boards/" OUTPUT_VARIABLE BOARD_MANFACTURER_DIRS)
endif()

foreach(BOARD_MANFACTURER_DIR IN LISTS BOARD_MANFACTURER_DIRS)
    get_filename_component(MANFACTURER_NAME ${BOARD_MANFACTURER_DIR} NAME)
    message("Add Manfacturer: " ${MANFACTURER_NAME})
    add_subdirectory(${BOARD_MANFACTURER_DIR})
endforeach()
