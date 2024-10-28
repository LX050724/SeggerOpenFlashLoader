#include "FlashAbstractionLayer.h"
#include "board.h"

static void FalCmd2HalQSpiCmd(FAL_SPI_Cmd_t *fal_cmd, QSPI_CommandTypeDef *hal_cmd)
{
    switch (fal_cmd->InstructionMode)
    {
    case FAL_INSTRUCTION_NONE:
        hal_cmd->InstructionMode = QSPI_INSTRUCTION_NONE;
        break;
    case FAL_INSTRUCTION_1_LINE:
        hal_cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
        break;
    case FAL_INSTRUCTION_2_LINE:
        hal_cmd->InstructionMode = QSPI_INSTRUCTION_2_LINES;
        break;
    case FAL_INSTRUCTION_4_LINE:
        hal_cmd->InstructionMode = QSPI_INSTRUCTION_4_LINES;
        break;
    }

    switch (fal_cmd->AddressMode)
    {

    case FAL_ADDRESS_NONE:
        hal_cmd->AddressMode = QSPI_ADDRESS_NONE;
        break;
    case FAL_ADDRESS_1_LINE:
        hal_cmd->AddressMode = QSPI_ADDRESS_1_LINE;
        break;
    case FAL_ADDRESS_2_LINES:
        hal_cmd->AddressMode = QSPI_ADDRESS_2_LINES;
        break;
    case FAL_ADDRESS_4_LINES:
        hal_cmd->AddressMode = QSPI_ADDRESS_4_LINES;
        break;
    }

    switch (fal_cmd->DataMode)
    {
    case FAL_DATA_NONE:
        hal_cmd->DataMode = QSPI_DATA_NONE;
        break;
    case FAL_DATA_1_LINE:
        hal_cmd->DataMode = QSPI_DATA_1_LINE;
        break;
    case FAL_DATA_2_LINES:
        hal_cmd->DataMode = QSPI_DATA_2_LINES;
        break;
    case FAL_DATA_4_LINES:
        hal_cmd->DataMode = QSPI_DATA_4_LINES;
        break;
    }

    switch (fal_cmd->AddressSize)
    {
    case FAL_ADDRESS_8_BITS:
        hal_cmd->AddressSize = QSPI_ADDRESS_8_BITS;
        break;
    case FAL_ADDRESS_16_BITS:
        hal_cmd->AddressSize = QSPI_ADDRESS_16_BITS;
        break;
    case FAL_ADDRESS_24_BITS:
        hal_cmd->AddressSize = QSPI_ADDRESS_24_BITS;
        break;
    case FAL_ADDRESS_32_BITS:
        hal_cmd->AddressSize = QSPI_ADDRESS_32_BITS;
        break;
    }

    hal_cmd->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    hal_cmd->DdrMode = QSPI_DDR_MODE_DISABLE;
    hal_cmd->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    hal_cmd->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    hal_cmd->DummyCycles = fal_cmd->DummyCycles;
    hal_cmd->Instruction = fal_cmd->Instruction;
    hal_cmd->Address = fal_cmd->Address;
    hal_cmd->NbData = fal_cmd->NbData;
}

FAL_Status FAL_SPI_SendCmd(FAL_SPI_Cmd_t *cmd, void *userdata)
{
    QSPI_CommandTypeDef sCommand = {};
    FalCmd2HalQSpiCmd(cmd, &sCommand);
    return HAL_QSPI_Command(userdata, &sCommand, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_SendData(uint8_t *data, uint32_t len, void *userdata)
{
    return HAL_QSPI_Transmit(userdata, data, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_ReceiveData(uint8_t *data, uint32_t len, void *userdata)
{
    return HAL_QSPI_Receive(userdata, data, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_Polling(FAL_SPI_Cmd_t *cmd, FAL_SPI_PollingConfig_t *config, void *userdata)
{
    QSPI_CommandTypeDef sCommand = {};
    QSPI_AutoPollingTypeDef sConfig = {};
    FalCmd2HalQSpiCmd(cmd, &sCommand);

    sConfig.Match = config->Match;
    sConfig.Mask = config->Mask;
    sConfig.Interval = 0x10;
    sConfig.StatusBytesSize = config->StatusBytesSize;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;            //	与运算
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE; // 自动停止模式

    return HAL_QSPI_AutoPolling(userdata, &sCommand, &sConfig, HAL_MAX_DELAY) == HAL_OK ? FAL_OK : FAL_ERR;
}

FAL_Status FAL_SPI_MemoryMappedMode(FAL_SPI_Cmd_t *cmd, bool enable, void *userdata)
{
    if (enable)
    {
        QSPI_CommandTypeDef sCommand = {};
        QSPI_MemoryMappedTypeDef sMemMappedCfg = {};
        FalCmd2HalQSpiCmd(cmd, &sCommand);
        return HAL_QSPI_MemoryMapped(userdata, &sCommand, &sMemMappedCfg) == HAL_OK ? FAL_OK : FAL_ERR;
    }
    else
    {
        return HAL_QSPI_Abort(userdata) == HAL_OK ? FAL_OK : FAL_ERR;
    }
}

