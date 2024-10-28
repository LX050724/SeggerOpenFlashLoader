if (DRIVER_STM32H7XX_ENABLE)

add_executable(DAMIAO_MC02_W25Q64
    FlashDev.c
    board.c
    octospi.c
)

target_include_directories(DAMIAO_MC02_W25Q64 PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(DAMIAO_MC02_W25Q64 PRIVATE
    USE_HAL_DRIVER
    STM32H723xx
    EXFLASH_BASE_ADDR=0x70000000
    EXFLASH_PAGE_SIZE=256
)

target_link_libraries(DAMIAO_MC02_W25Q64
    stm32h7xx_hal
    # FlashAbstractionLayer
    FAL_STM_HAL_OSPI
)

DAMIAO_install_elf(DAMIAO_MC02_W25Q64)

endif()
