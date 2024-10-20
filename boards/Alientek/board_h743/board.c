#include "board.h"
#include "W25Q256/w25q256.h"
#include "quadspi.h"
#include "stm32h743xx.h"
#include "stm32h7xx_hal_qspi.h"
#include "stm32h7xx_ll_bus.h"
#include "system_stm32h7xx.h"
#include <stdint.h>
#include <string.h>

int board_init(uint32_t addr, uint32_t freq, uint32_t func)
{
    SystemInit();
    memset(&hqspi, 0, sizeof(hqspi));
    MX_QUADSPI_Init();
    if (QSPI_W25Qxx_Init() != 0)
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
    return en ? QSPI_W25Qxx_MemoryMappedMode() : QSPI_W25Qxx_ExitMemoryMappedMode();
}

int board_exFlash_EraseSector(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_EraseSector(SectorAddr);
}

int board_exFlash_EraseBlock32K(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_EraseBlock32K(SectorAddr);
}

int board_exFlash_EraseBlock64K(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_EraseBlock64K(SectorAddr);
}

int board_exFlash_EraseChip()
{
    return QSPI_W25Qxx_EraseChip();
}

int board_exFlash_WritePage(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return QSPI_W25Qxx_WritePage(addr, buf, len);
}

int board_exFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return QSPI_W25Qxx_Read(addr, buf, len);
}
