#include "W25Q64/w25q64.h"
#include "octospi.h"
#include "stm32h7xx_hal_ospi.h"
#include "stm32h7xx_ll_bus.h"
#include "system_stm32h7xx.h"
#include <board.h>
#include <stdint.h>
#include <string.h>

int board_init(uint32_t addr, uint32_t freq, uint32_t func)
{
    SystemInit();
    memset(&hospi2, 0, sizeof(hospi2));
    MX_OCTOSPI2_Init();

    if (OSPI_W25Qxx_Init() != OSPI_W25Qxx_OK)
        return -1;
    return 0;
}

int board_deinit(uint32_t func)
{
    HAL_OSPI_DeInit(&hospi2);

    LL_AHB3_GRP1_ForceReset(LL_AHB3_GRP1_PERIPH_OSPI2);
    LL_AHB3_GRP1_ReleaseReset(LL_AHB3_GRP1_PERIPH_OSPI2);
    memset(&hospi2, 0, sizeof(hospi2));
    return 0;
}

int borad_exFlash_EnableMMAP(uint8_t en)
{
    if (en)
        return OSPI_W25Qxx_MemoryMappedMode() == OSPI_W25Qxx_OK ? 0 : -1;
    else
        return HAL_OSPI_Abort(&hospi2);
}

int board_exFlash_EraseSector(uint32_t SectorAddr)
{
    return OSPI_W25Qxx_SectorErase(SectorAddr) == OSPI_W25Qxx_OK ? 0 : -1;
}

int board_exFlash_EraseBlock32K(uint32_t SectorAddr)
{
    return OSPI_W25Qxx_BlockErase_32K(SectorAddr) == OSPI_W25Qxx_OK ? 0 : -1;
}

int board_exFlash_EraseBlock64K(uint32_t SectorAddr)
{
    return OSPI_W25Qxx_BlockErase_64K(SectorAddr) == OSPI_W25Qxx_OK ? 0 : -1;
}

int board_exFlash_EraseChip()
{
    return OSPI_W25Qxx_ChipErase() == OSPI_W25Qxx_OK ? 0 : -1;
}

int board_exFlash_WritePage(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return OSPI_W25Qxx_WritePage(buf, addr, len) == OSPI_W25Qxx_OK ? (int)len : -1;
}

int board_exFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return OSPI_W25Qxx_ReadBuffer(buf, addr, len) == OSPI_W25Qxx_OK ? (int)len : -1;
}
