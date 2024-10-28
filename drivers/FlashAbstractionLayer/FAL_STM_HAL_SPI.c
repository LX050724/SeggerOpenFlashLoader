#include "FlashAbstractionLayer.h"
#include "board.h"

static uint8_t fuck_spi_buf[512];

static void SPI_SetCS(bool cs, void *userdata)
{
    FAL_SPI_Userdata_t *handle = userdata;
    if (cs)
    {
        if (handle->cs_gpio_handle)
        {
            HAL_GPIO_WritePin(handle->cs_gpio_handle, handle->cs_gpio_pin, GPIO_PIN_RESET);
        }
    }
    else
    {
        if (handle->cs_gpio_handle)
        {
            HAL_GPIO_WritePin(handle->cs_gpio_handle, handle->cs_gpio_pin, GPIO_PIN_SET);
        }
    }
}

static FAL_Status SPI_TransmitReceive(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, void *userdata)
{
    FAL_SPI_Userdata_t *handle = userdata;
    SPI_HandleTypeDef *spi = handle->spi;

    if (rx_buf && tx_buf)
    {
        HAL_SPI_TransmitReceive(spi, tx_buf, rx_buf, len, HAL_MAX_DELAY);
    }
    else if (tx_buf)
    {
        HAL_SPI_Transmit(spi, tx_buf, len, HAL_MAX_DELAY);
    }
    else if (rx_buf)
    {
        while (len)
        {
            uint32_t op_len = (len > sizeof(fuck_spi_buf)) ? sizeof(fuck_spi_buf) : len;
            if (HAL_SPI_TransmitReceive(spi, fuck_spi_buf, rx_buf, op_len, HAL_MAX_DELAY) != HAL_OK)
            {
                while (1) {
                
                }
            }
            len -= op_len;
            rx_buf += op_len;
        }
    }
    return FAL_OK;
}

FAL_Status FAL_SPI_SendCmd(FAL_SPI_Cmd_t *cmd, void *userdata)
{
    uint8_t buf[16];
    uint8_t len = 0;

    if (cmd->InstructionMode == FAL_INSTRUCTION_1_LINE)
        buf[len++] = cmd->Instruction;

    if (cmd->AddressMode == FAL_ADDRESS_1_LINE)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
        switch (cmd->AddressSize)
        {
        case FAL_ADDRESS_32_BITS:
            buf[len++] = (cmd->Address >> 24) & 0xff;
        case FAL_ADDRESS_24_BITS:
            buf[len++] = (cmd->Address >> 16) & 0xff;
        case FAL_ADDRESS_16_BITS:
            buf[len++] = (cmd->Address >> 8) & 0xff;
        case FAL_ADDRESS_8_BITS:
            buf[len++] = cmd->Address & 0xff;
        }
#pragma GCC diagnostic pop
    }

    for (uint32_t i = 0; i < cmd->DummyCycles / 8; i++)
        buf[len++] = 0;

    SPI_SetCS(true, userdata);
    SPI_TransmitReceive(buf, NULL, len, userdata);

    if (cmd->NbData == 0)
        SPI_SetCS(false, userdata);
    return FAL_OK;
}

FAL_Status FAL_SPI_SendData(uint8_t *data, uint32_t len, void *userdata)
{
    SPI_TransmitReceive(data, NULL, len, userdata);
    SPI_SetCS(false, userdata);
    return FAL_OK;
}

FAL_Status FAL_SPI_ReceiveData(uint8_t *data, uint32_t len, void *userdata)
{
    SPI_TransmitReceive(NULL, data, len, userdata);
    SPI_SetCS(false, userdata);
    return FAL_OK;
}

FAL_Status FAL_SPI_Polling(FAL_SPI_Cmd_t *cmd, FAL_SPI_PollingConfig_t *config, void *userdata)
{
    uint32_t value = 0;
    do
    {
        FAL_SPI_SendCmd(cmd, userdata);
        FAL_SPI_ReceiveData((uint8_t *)&value, config->StatusBytesSize, userdata);
    } while ((value & config->Mask) != config->Match);
    return FAL_OK;
}

FAL_Status FAL_SPI_MemoryMappedMode(FAL_SPI_Cmd_t *cmd, bool enable, void *userdata)
{
    return FAL_ERR;
}
