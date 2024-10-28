if (DRIVER_STM32H7XX_ENABLE)

add_executable(RTThread_ArtPi_QSPI_W25Q64
    FlashDev.c
    board.c
    quadspi.c
)

target_include_directories(RTThread_ArtPi_QSPI_W25Q64 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(RTThread_ArtPi_QSPI_W25Q64 PRIVATE
    USE_FULL_LL_DRIVER 
    USE_HAL_DRIVER 
    STM32H750xx
    EXFLASH_BASE_ADDR=0x90000000
    EXFLASH_PAGE_SIZE=256
    EXFLASH_SIZE=0x800000
    QSPI_W25Q64
)

target_link_libraries(RTThread_ArtPi_QSPI_W25Q64
    stm32h7xx_hal
    FAL_STM_HAL_QSPI
)

RTThread_install_elf(RTThread_ArtPi_QSPI_W25Q64)

add_executable(RTThread_ArtPi_SPI_W25Q128
    FlashDev.c
    board.c
    spi.c
)

target_include_directories(RTThread_ArtPi_SPI_W25Q128 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(RTThread_ArtPi_SPI_W25Q128 PRIVATE
    USE_FULL_LL_DRIVER 
    USE_HAL_DRIVER 
    STM32H750xx
    EXFLASH_BASE_ADDR=0xA0000000
    EXFLASH_PAGE_SIZE=256
    EXFLASH_SIZE=0x1000000
    SPI_W25Q128
)

target_link_libraries(RTThread_ArtPi_SPI_W25Q128
    stm32h7xx_hal
    FAL_STM_HAL_SPI
)

RTThread_install_elf(RTThread_ArtPi_SPI_W25Q128)

endif()
