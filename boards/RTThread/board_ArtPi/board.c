#include "quadspi.h"
#include "spi.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_utils.h"
#include <FlashAbstractionLayer.h>
#include <board.h>
#include <stdint.h>
#include <string.h>


FAL_SPI_Driver fal_driver;

#ifdef SPI_W25Q128
#endif
FAL_SPI_Userdata_t fal_userdata;

void SystemClock_Config(void);

int board_init(uint32_t addr, uint32_t freq, uint32_t func)
{
    SystemInit();
    SCB_EnableICache();
    SystemClock_Config();
    memset(&fal_driver, 0, sizeof(fal_driver));

#ifdef QSPI_W25Q64
    memset(&hqspi, 0, sizeof(hqspi));
    MX_QUADSPI_Init();
    fal_driver.userdata = &hqspi;
    fal_driver.capacity.flash_size = FLASH_SIZE_MBIT(64);
    fal_driver.capacity.support_mmap = true;
    fal_driver.capacity.spi_type = SPI_TYPE_QUAD;
#endif

#ifdef SPI_W25Q128
    MX_SPI1_Init();
    fal_userdata.cs_gpio_handle = NULL;
    fal_userdata.spi = &hspi1;
    fal_userdata.cs_gpio_handle = GPIOA;
    fal_userdata.cs_gpio_pin = GPIO_PIN_4;
    fal_driver.userdata = &fal_userdata;
    fal_driver.capacity.flash_size = FLASH_SIZE_MBIT(128);
    fal_driver.capacity.support_mmap = false;
    fal_driver.capacity.spi_type = SPI_TYPE_STD;
#endif
    if (FAL_Init(&fal_driver) != FAL_OK)
        return -1;
    return 0;
}

int board_deinit(uint32_t func)
{
#ifdef QSPI_W25Q64
    HAL_QSPI_DeInit(&hqspi);
    LL_AHB3_GRP1_ForceReset(LL_AHB3_GRP1_PERIPH_QSPI);
    LL_AHB3_GRP1_ReleaseReset(LL_AHB3_GRP1_PERIPH_QSPI);
    memset(&hqspi, 0, sizeof(hqspi));
#endif

#ifdef SPI_W25Q128
    LL_SPI_DeInit(SPI1);
    LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
#endif

    LL_RCC_DeInit();
    SCB_DisableICache();
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

void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
    {
    }
    LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
    while (LL_PWR_IsActiveFlag_VOS() == 0)
    {
    }
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1)
    {
    }
    LL_RCC_HSI_SetCalibTrimming(64);
    LL_RCC_HSI_SetDivider(LL_RCC_HSI_DIV1);
    LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSI);
    LL_RCC_PLL1P_Enable();
    LL_RCC_PLL1Q_Enable();
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    LL_RCC_PLL1_SetM(4);
    LL_RCC_PLL1_SetN(25);
    LL_RCC_PLL1_SetP(2);
    LL_RCC_PLL1_SetQ(4);
    LL_RCC_PLL1_SetR(2);
    LL_RCC_PLL1_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL1_IsReady() != 1)
    {
    }

    /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
    {
    }
    LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
    LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);
    LL_SetSystemCoreClock(200000000);
}
