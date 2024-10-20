# Create an executable object type
add_library(stm32h7xx_hal INTERFACE)

# Link directories setup
target_link_directories(stm32h7xx_hal INTERFACE ${CMAKE_SOURCE_DIR}/third_party/Segger)

file(
    GLOB HAL_SRCS 
    LIST_DIRECTORIES false
    ${CMAKE_SOURCE_DIR}/third_party/stm32h7xx_hal_driver/Src/stm32h7xx_*
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
    ${CMAKE_SOURCE_DIR}/third_party/CMSIS_6/CMSIS/Core/Include
    ${CMAKE_SOURCE_DIR}/third_party/CMSIS_6/CMSIS/Core/Include/m-profile
    ${CMAKE_SOURCE_DIR}/third_party/cmsis_device_h7/Include
    ${CMAKE_SOURCE_DIR}/third_party/stm32h7xx_hal_driver/Inc
    ${CMAKE_SOURCE_DIR}/third_party/stm32h7xx_hal_driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Src
)

target_compile_definitions(stm32h7xx_hal INTERFACE
    USE_HAL_DRIVER 
    $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(stm32h7xx_hal INTERFACE SEGGER_OFL_Lib_CortexM_LE)

target_link_options(stm32h7xx_hal INTERFACE -T ${CMAKE_CURRENT_LIST_DIR}/stm32h7xx_flashloader.ld)
