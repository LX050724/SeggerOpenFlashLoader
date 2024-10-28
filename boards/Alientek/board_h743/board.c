#include "board.h"
#include "quadspi.h"
#include "stm32h7xx_ll_bus.h"
#include "system_stm32h7xx.h"
#include <FlashAbstractionLayer.h>
#include <stdint.h>
#include <string.h>

FAL_SPI_Driver fal_driver;

int board_init(uint32_t addr, uint32_t freq, uint32_t func)
{
    SystemInit();
    memset(&hqspi, 0, sizeof(hqspi));
    memset(&fal_driver, 0, sizeof(fal_driver));
    MX_QUADSPI_Init();

    fal_driver.userdata = &hqspi;
    fal_driver.capacity.flash_size = FLASH_SIZE_MBIT(256);
    fal_driver.capacity.support_mmap = true;
    fal_driver.capacity.spi_type = SPI_TYPE_QUAD;
    if (FAL_Init(&fal_driver) != FAL_OK)
        return -1;
    return 0;
}

int board_deinit(uint32_t func)
{
    HAL_QSPI_DeInit(&hqspi);

    LL_AHB3_GRP1_ForceReset(LL_AHB3_GRP1_PERIPH_QSPI);
    LL_AHB3_GRP1_ReleaseReset(LL_AHB3_GRP1_PERIPH_QSPI);
    memset(&hqspi, 0, sizeof(hqspi));
    return 0;
}

int borad_exFlash_EnableMMAP(uint8_t en)
{
    return FAL_MemoryMappedMode(&fal_driver, en);
}

int board_exFlash_EraseSector(uint32_t SectorAddr)
{
    return FAL_EraseSector(&fal_driver, SectorAddr);
}

int board_exFlash_EraseBlock32K(uint32_t SectorAddr)
{
    return FAL_EraseBlock32K(&fal_driver, SectorAddr);
}

int board_exFlash_EraseBlock64K(uint32_t SectorAddr)
{
    return FAL_EraseBlock64K(&fal_driver, SectorAddr);
}

int board_exFlash_EraseChip()
{
    return FAL_EraseChip(&fal_driver);
}

int board_exFlash_WritePage(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return FAL_WritePage(&fal_driver, addr, buf, len);
}

int board_exFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return FAL_Read(&fal_driver, addr, buf, len);
}
