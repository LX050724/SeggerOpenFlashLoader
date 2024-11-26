if (DRIVER_STM32H7XX_ENABLE)

add_executable(None_FocG431Pico_GD25Q80
    FlashDev.c
    board.c
    spi.c
)

target_include_directories(None_FocG431Pico_GD25Q80 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(None_FocG431Pico_GD25Q80 PRIVATE
    USE_FULL_LL_DRIVER
    USE_HAL_DRIVER
    STM32G431xx
    EXFLASH_BASE_ADDR=0xA0000000
    EXFLASH_PAGE_SIZE=256
    SPI_GD25Q80
)

target_link_libraries(None_FocG431Pico_GD25Q80
    stm32g4xx_hal
    # FlashAbstractionLayer
    FAL_STM_HAL_SPI
)

None_install_elf(None_FocG431Pico_GD25Q80)

endif()
