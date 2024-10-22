include(FetchContent)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")

if (DRIVER_ALL_ENABLE)
    set(DRIVER_STM32H7XX_ENABLE TRUE CACHE BOOL "启用STM32H7xx驱动" FORCE)
else()
    set(DRIVER_STM32H7XX_ENABLE FALSE CACHE BOOL "启用STM32H7xx驱动")
endif()

if (DRIVER_STM32H7XX_ENABLE)

FetchContent_Declare(stm32h7xx_hal_driver
    GIT_REPOSITORY https://github.com/STMicroelectronics/stm32h7xx_hal_driver.git
    GIT_TAG v1.11.3
    SOURCE_DIR third_party/stm32h7xx_hal_driver
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

message("Downloading stm32h7xx_hal_driver ...")
FetchContent_MakeAvailable(stm32h7xx_hal_driver)

FetchContent_Declare(cmsis6
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_6.git
    SOURCE_DIR third_party/cmsis6
    GIT_TAG v6.1.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

message("Downloading cmsis6 ...")
FetchContent_MakeAvailable(cmsis6)

FetchContent_Declare(cmsis_device_h7
    GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis_device_h7.git
    SOURCE_DIR third_party/cmsis_device_h7
    GIT_TAG v1.10.4
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

message("Downloading cmsis_device_h7 ...")
FetchContent_MakeAvailable(cmsis_device_h7)

# Create an executable object type
add_library(stm32h7xx_hal INTERFACE)

# Link directories setup
target_link_directories(stm32h7xx_hal INTERFACE ${CMAKE_SOURCE_DIR}/third_party/Segger)

file(
    GLOB HAL_SRCS 
    LIST_DIRECTORIES false
    ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/stm32h7xx_*
)

list(FILTER HAL_SRCS EXCLUDE REGEX ".*template\.c")
list(FILTER HAL_SRCS EXCLUDE REGEX "stm32h7xx_ll_delayblock.c")

target_sources(stm32h7xx_hal INTERFACE
    ${HAL_SRCS}
    ${CMAKE_SOURCE_DIR}/Src/FlashPrg.c
    system_stm32h7xx.c
    syscalls.c
)

# Add include paths
target_include_directories(stm32h7xx_hal INTERFACE 
    ${cmsis6_SOURCE_DIR}/CMSIS/Core/Include
    ${cmsis6_SOURCE_DIR}/CMSIS/Core/Include/m-profile
    ${cmsis_device_h7_SOURCE_DIR}/Include
    ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc
    ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Src
)

target_compile_definitions(stm32h7xx_hal INTERFACE
    USE_HAL_DRIVER 
    $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(stm32h7xx_hal INTERFACE SEGGER_OFL_Lib_CortexM_LE)

target_link_options(stm32h7xx_hal INTERFACE -T ${CMAKE_CURRENT_LIST_DIR}/stm32h7xx_flashloader.ld)

set(TARGET_FLAGS -mcpu=cortex-m7)
target_link_options(stm32h7xx_hal INTERFACE ${TARGET_FLAGS} -T ${CMAKE_CURRENT_LIST_DIR}/stm32h7xx_flashloader.ld)
target_compile_options(stm32h7xx_hal INTERFACE ${TARGET_FLAGS})

endif()
endif()