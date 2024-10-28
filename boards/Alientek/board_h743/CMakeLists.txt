if (DRIVER_STM32H7XX_ENABLE)

add_executable(Alientek_H743_W25Q256
    FlashDev.c
    board.c
    quadspi.c
)

target_include_directories(Alientek_H743_W25Q256 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(Alientek_H743_W25Q256 PRIVATE
    USE_HAL_DRIVER
    STM32H743xx
    EXFLASH_BASE_ADDR=0x90000000
    EXFLASH_PAGE_SIZE=256
)

target_link_libraries(Alientek_H743_W25Q256
    stm32h7xx_hal
    FAL_STM_HAL_QSPI
)

Alientek_install_elf(Alientek_H743_W25Q256)

endif()