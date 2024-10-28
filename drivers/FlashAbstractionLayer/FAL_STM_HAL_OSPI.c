#include "FlashAbstractionLayer.h"
#include "board.h"

static void FalCmd2HalOSpiCmd(FAL_SPI_Cmd_t *fal_cmd, OSPI_RegularCmdTypeDef *hal_cmd)
{
    switch (fal_cmd->InstructionMode)
    {
    case FAL_INSTRUCTION_NONE:
        hal_cmd->InstructionMode = HAL_OSPI_INSTRUCTION_NONE;
        break;
    case FAL_INSTRUCTION_1_LINE:
        hal_cmd->InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
        break;
    case FAL_INSTRUCTION_2_LINE:
        hal_cmd->InstructionMode = HAL_OSPI_INSTRUCTION_2_LINES;
        break;
    case FAL_INSTRUCTION_4_LINE:
        hal_cmd->InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
        break;
    }

    switch (fal_cmd->AddressMode)
    {

    case FAL_ADDRESS_NONE:
        hal_cmd->AddressMode = HAL_OSPI_ADDRESS_NONE;
        break;
    case FAL_ADDRESS_1_LINE:
        hal_cmd->AddressMode = HAL_OSPI_ADDRESS_1_LINE;
        break;
    case FAL_ADDRESS_2_LINES:
        hal_cmd->AddressMode = HAL_OSPI_ADDRESS_2_LINES;
        break;
    case FAL_ADDRESS_4_LINES:
        hal_cmd->AddressMode = HAL_OSPI_ADDRESS_4_LINES;
        break;
    }

    switch (fal_cmd->DataMode)
    {
    case FAL_DATA_NONE:
        hal_cmd->DataMode = HAL_OSPI_DATA_NONE;
        break;
    case FAL_DATA_1_LINE:
        hal_cmd->DataMode = HAL_OSPI_DATA_1_LINE;
        break;
    case FAL_DATA_2_LINES:
        hal_cmd->DataMode = HAL_OSPI_DATA_2_LINES;
        break;
    case FAL_DATA_4_LINES:
        hal_cmd->DataMode = HAL_OSPI_DATA_4_LINES;
        break;
    }

    switch (fal_cmd->AddressSize)
    {
    case FAL_ADDRESS_8_BITS:
        hal_cmd->AddressSize = HAL_OSPI_ADDRESS_8_BITS;
        break;
    case FAL_ADDRESS_16_BITS:
        hal_cmd->AddressSize = HAL_OSPI_ADDRESS_16_BITS;
        break;
    case FAL_ADDRESS_24_BITS:
        hal_cmd->AddressSize = HAL_OSPI_ADDRESS_24_BITS;
        break;
    case FAL_ADDRESS_32_BITS:
        hal_cmd->AddressSize = HAL_OSPI_ADDRESS_32_BITS;
        break;
    }

    hal_cmd->SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    hal_cmd->DummyCycles = fal_cmd->DummyCycles;
    hal_cmd->Instruction = fal_cmd->Instruction;
    hal_cmd->Address = fal_cmd->Address;
    hal_cmd->NbData = fal_cmd->NbData;
}

FAL_Status FAL_SPI_SendCmd(FAL_SPI_Cmd_t *cmd, void *userdata)
{
    OSPI_RegularCmdTypeDef sCommand = {};
    FalCmd2HalOSpiCmd(cmd, &sCommand);
    return HAL_OSPI_Command(userdata, &sCommand, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_SendData(uint8_t *data, uint32_t len, void *userdata)
{
    return HAL_OSPI_Transmit(userdata, data, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_ReceiveData(uint8_t *data, uint32_t len, void *userdata)
{
    return HAL_OSPI_Receive(userdata, userdata, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_Polling(FAL_SPI_Cmd_t *cmd, FAL_SPI_PollingConfig_t *config, void *userdata)
{
    OSPI_AutoPollingTypeDef sConfig = {};

    sConfig.Match = config->Match;
    sConfig.Mask = config->Mask;
    sConfig.Interval = 0x10;
    sConfig.MatchMode = HAL_OSPI_MATCH_MODE_AND;            //	与运算
    sConfig.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE; // 自动停止模式

    FAL_SPI_SendCmd(cmd, userdata);

    return HAL_OSPI_AutoPolling(userdata, &sConfig, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_MemoryMappedMode(FAL_SPI_Cmd_t *cmd, bool enable, void *userdata)
{
    if (enable)
    {
        OSPI_MemoryMappedTypeDef sMemMappedCfg;
        sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
        sMemMappedCfg.TimeOutPeriod = 0;
        FAL_SPI_SendCmd(cmd, userdata);
        return HAL_OSPI_MemoryMapped(userdata, &sMemMappedCfg) == HAL_OK ? FAL_OK : FAL_ERR;
    }
    else
    {
        return HAL_OSPI_Abort(userdata) == HAL_OK ? FAL_OK : FAL_ERR;
    }
}
