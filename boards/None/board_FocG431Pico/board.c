#include "spi.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_spi.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_utils.h"
#include <FlashAbstractionLayer.h>
#include <board.h>
#include <stdint.h>
#include <string.h>

FAL_SPI_Driver fal_driver;
FAL_SPI_Userdata_t fal_userdata;

void SystemClock_Config(void);
void RCC_Deinit();

int board_init(uint32_t addr, uint32_t freq, uint32_t func)
{
    SystemInit();
    SystemClock_Config();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    /*Configure GPIO pin : PA4 */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    MX_SPI1_Init();
    fal_userdata.cs_gpio_handle = NULL;
    fal_userdata.spi = &hspi1;
    fal_userdata.cs_gpio_handle = GPIOA;
    fal_userdata.cs_gpio_pin = GPIO_PIN_4;
    fal_driver.userdata = &fal_userdata;
    fal_driver.capacity.flash_size = FLASH_SIZE_MBIT(8);
    fal_driver.capacity.support_mmap = false;
    fal_driver.capacity.spi_type = SPI_TYPE_STD;

    if (FAL_Init(&fal_driver) != FAL_OK)
        return -1;
    return 0;
}

int board_deinit(uint32_t func)
{
    HAL_SPI_DeInit(&hspi1);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
    RCC_Deinit();
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
    LL_PWR_EnableRange1BoostMode();
    LL_RCC_HSI_Enable();
    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1)
    {
    }

    LL_RCC_HSI_SetCalibTrimming(64);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_4, 85, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();
    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1)
    {
    }

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    }

    /* Insure 1us transition state at intermediate medium speed clock*/
    for (__IO uint32_t i = (170 >> 1); i != 0; i--)
        ;

    /* Set AHB prescaler*/
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_SetSystemCoreClock(170000000);
}

void RCC_Deinit()
{
    /* Set HSION bit to the reset value */
    SET_BIT(RCC->CR, RCC_CR_HSION);

    /* Wait till HSI is ready */
    while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
    {
    }

    /* Set HSITRIM[6:0] bits to the reset value */
    SET_BIT(RCC->ICSCR, RCC_HSICALIBRATION_DEFAULT << RCC_ICSCR_HSITRIM_Pos);

    /* Reset CFGR register (HSI is selected as system clock source) */
    RCC->CFGR = 0x00000001u;

    /* Wait till HSI is ready */
    while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    {
    }

    /* Update the SystemCoreClock global variable */
    SystemCoreClock = HSI_VALUE;

    /* Clear CR register in 2 steps: first to clear HSEON in case bypass was enabled */
    RCC->CR = RCC_CR_HSION;

    /* Then again to HSEBYP in case bypass was enabled */
    RCC->CR = RCC_CR_HSION;

    /* Wait till PLL is OFF */
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
    {
    }

    /* once PLL is OFF, reset PLLCFGR register to default value */
    RCC->PLLCFGR = RCC_PLLCFGR_PLLN_4;

    /* Disable all interrupts */
    CLEAR_REG(RCC->CIER);

    /* Clear all interrupt flags */
    WRITE_REG(RCC->CICR, 0xFFFFFFFFU);

    /* Clear all reset flags */
    SET_BIT(RCC->CSR, RCC_CSR_RMVF);
}