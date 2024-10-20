#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

int QSPI_W25Qxx_Init();
int QSPI_W25Qxx_Reset();
int QSPI_W25Qxx_Entry4BAddr();
uint32_t QSPI_W25Qxx_Read_JEDECID();
int QSPI_W25Qxx_Read(uint32_t addr, uint8_t *buf, uint32_t len);
int QSPI_W25Qxx_WritePage(uint32_t addr, uint8_t *buf, uint32_t len);
int QSPI_W25Qxx_EraseSector(uint32_t SectorAddr);
int QSPI_W25Qxx_EraseBlock32K(uint32_t SectorAddr);
int QSPI_W25Qxx_EraseBlock64K(uint32_t SectorAddr);
int QSPI_W25Qxx_EraseChip();
int QSPI_W25Qxx_MemoryMappedMode();
int QSPI_W25Qxx_ExitMemoryMappedMode();

#ifdef __cplusplus
}
#endif