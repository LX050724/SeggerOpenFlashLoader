#pragma once

#include <stdint.h>

//
// Some flash types require a native verify function as the memory is not memory mapped available (e.g. eMMC flashes).
// If the verify function is implemented in the algorithm, it will be used by the J-Link DLL during compare / verify
// independent of what verify type is configured in the J-Link DLL.
// Please note, that SEGGER does not recommend to use this function if the flash can be memory mapped read
// as this may can slow-down the compare / verify step.
//
#define SUPPORT_NATIVE_VERIFY (0)
#define SUPPORT_NATIVE_READ_FUNCTION (0)
#define SUPPORT_BLANK_CHECK (1)
#define SUPPORT_ERASE_CHIP (1)
#define SUPPORT_SEGGER_OPEN_Program (1)
#define SUPPORT_SEGGER_OPEN_ERASE (1)
#define SUPPORT_TURBO_MODE (1)

int board_init(uint32_t addr, uint32_t freq, uint32_t func);
int board_deinit(uint32_t func);

int board_exFlash_EraseSector(uint32_t SectorAddr);
int board_exFlash_EraseBlock32K(uint32_t SectorAddr);
int board_exFlash_EraseBlock64K(uint32_t SectorAddr);

#if SUPPORT_ERASE_CHIP
int board_exFlash_EraseChip();
#endif

int board_exFlash_WritePage(uint32_t addr, uint8_t *buf, uint32_t len);

#if SUPPORT_NATIVE_READ_FUNCTION
int board_exFlash_Read(uint32_t addr, uint8_t *buf, uint32_t len);
#else
int borad_exFlash_EnableMMAP();
#endif
