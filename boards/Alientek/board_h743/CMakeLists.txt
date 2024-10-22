if (DRIVER_STM32H7XX_ENABLE)

add_executable(Alientek_H743_W25Q256
    FlashDev.c
    board.c
    quadspi.c
    W25Q256/w25q256.c
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

target_link_libraries(Alientek_H743_W25Q256 stm32h7xx_hal)

Alientek_install_elf(Alientek_H743_W25Q256)

target_link_options(Alientek_H743_W25Q256 PRIVATE "-Wl,-Map=Alientek_H743_W25Q256.map")

endif()