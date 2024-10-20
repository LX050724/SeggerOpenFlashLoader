
add_executable(DAMIAO_MC02_W25Q64
    FlashDev.c
    board.c
    octospi.c
    W25Q64/w25q64.c
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

target_link_libraries(DAMIAO_MC02_W25Q64 stm32h7xx_hal)

DAMIAO_install_elf(DAMIAO_MC02_W25Q64)

target_link_options(DAMIAO_MC02_W25Q64 PRIVATE "-Wl,-Map=DAMIAO_MC02_W25Q64.map")
