#include "FlashAbstractionLayer.h"
#include <stdint.h>
#include <sys/cdefs.h>

#define FAL_CMD_EnableReset 0x66
#define FAL_CMD_ResetDevice 0x99
#define FAL_CMD_JedecID 0x9F
#define FAL_CMD_WriteEnable 0X06

#define FAL_CMD_SectorErase 0x20
#define FAL_CMD_BlockErase_32K 0x52
#define FAL_CMD_BlockErase_64K 0xD8
#define FAL_CMD_ChipErase 0xC7

#define FAL_CMD_PageProgram 0x02
#define FAL_CMD_PageProgramQuad 0x32
#define FAL_CMD_FastRead 0x0B
#define FAL_CMD_FastReadQuad_IO 0xEB

#define FAL_CMD_Entry4BAddrMode 0xB
#define FAL_CMD_Exit4BAddrMode 0xE
#define FAL_CMD_WrtieExtAddr 0xC

#define FAL_CMD_ReadStatus_REG1 0X05
#define FAL_CMD_ReadStatus_REG2 0X35
#define FAL_CMD_ReadStatus_REG3 0X15
#define FAL_Status_REG1_BUSY 0x01
#define FAL_Status_REG1_WEL 0x02
#define FAL_Status_REG3_ADS 0x01

static FAL_Status FAL_SignalCmd(FAL_SPI_Driver *driver, uint8_t cmd)
{
    FAL_SPI_Cmd_t sCommand = {
        .InstructionMode = FAL_INSTRUCTION_1_LINE,
        .AddressMode = FAL_ADDRESS_NONE,
        .DataMode = FAL_DATA_NONE,
        .DummyCycles = 0,
        .Instruction = cmd,
    };

    return FAL_SPI_SendCmd(&sCommand, driver->userdata);
}

static FAL_Status FAL_PollingReg(FAL_SPI_Driver *driver, uint8_t reg_cmd, uint8_t mask, uint8_t val)
{
    FAL_SPI_Cmd_t sCommand = {
        .InstructionMode = FAL_INSTRUCTION_1_LINE,
        .Instruction = reg_cmd,
        .AddressMode = FAL_ADDRESS_NONE,
        .DataMode = FAL_DATA_1_LINE,
        .NbData = 1,
    };

    FAL_SPI_PollingConfig_t sConfig = {
        .Mask = mask,
        .Match = val,
        .StatusBytesSize = 1,
    };

    return FAL_SPI_Polling(&sCommand, &sConfig, driver->userdata);
}

static FAL_Status FAL_ReadReg(FAL_SPI_Driver *driver, uint8_t reg_cmd, uint8_t *reg)
{
    FAL_SPI_Cmd_t sCommand = {
        .InstructionMode = FAL_INSTRUCTION_1_LINE,
        .Instruction = reg_cmd,
        .AddressMode = FAL_ADDRESS_NONE,
        .DataMode = FAL_DATA_1_LINE,
        .NbData = 1,
        .DummyCycles = 0,
    };

    FAL_Status ret = FAL_SPI_SendCmd(&sCommand, driver->userdata);
    if (ret != FAL_OK)
        return ret;

    return FAL_SPI_ReceiveData(reg, sCommand.NbData, driver->userdata);
}

static FAL_Status FAL_Enter4ByteMode(FAL_SPI_Driver *driver)
{
    FAL_Status ret = FAL_SignalCmd(driver, FAL_CMD_Entry4BAddrMode);
    if (ret != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG3, FAL_Status_REG3_ADS, FAL_Status_REG3_ADS);
}

static FAL_Status FAL_WriteEnable(FAL_SPI_Driver *driver)
{
    FAL_Status ret = FAL_SignalCmd(driver, FAL_CMD_WriteEnable);
    if (ret != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG1, FAL_Status_REG1_WEL, FAL_Status_REG1_WEL);
}

FAL_Status FAL_Reset(FAL_SPI_Driver *driver)
{
    FAL_Status ret;
    if ((ret = FAL_SignalCmd(driver, FAL_CMD_EnableReset)) != FAL_OK)
        return ret;
    if ((ret = FAL_SignalCmd(driver, FAL_CMD_ResetDevice)) != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG1, FAL_Status_REG1_BUSY, 0);
}

FAL_Status FAL_Init(FAL_SPI_Driver *driver)
{
    FAL_Status ret;

    if (driver->capacity.flash_size > FLASH_SIZE_MBIT(128))
    {
        uint8_t reg3_val = 0;
        if ((ret = FAL_ReadReg(driver, FAL_CMD_ReadStatus_REG3, &reg3_val)) != FAL_OK)
            return ret;
        if ((reg3_val & FAL_Status_REG3_ADS) == 0)
        {
            if ((ret = FAL_Enter4ByteMode(driver)) != FAL_OK)
                return ret;
        }
        driver->status.AddressSize = FAL_ADDRESS_32_BITS;
        return FAL_OK;
    }

    driver->status.AddressSize = FAL_ADDRESS_24_BITS;
    return FAL_OK;
}

FAL_Status FAL_Read(FAL_SPI_Driver *driver, uint32_t Address, uint8_t *buf, uint32_t len)
{
    FAL_Status ret = FAL_OK;
    FAL_SPI_Cmd_t sCommand = {};

    sCommand.InstructionMode = FAL_INSTRUCTION_1_LINE;
    sCommand.AddressSize = driver->status.AddressSize;
    sCommand.Address = Address;
    sCommand.NbData = len;

    if (driver->capacity.spi_type == SPI_TYPE_QUAD)
    {
        sCommand.AddressMode = FAL_ADDRESS_4_LINES;
        sCommand.Instruction = FAL_CMD_FastReadQuad_IO;
        sCommand.DataMode = FAL_DATA_4_LINES;
        sCommand.DummyCycles = 6;
    }
    else
    {
        sCommand.AddressMode = FAL_ADDRESS_1_LINE;
        sCommand.Instruction = FAL_CMD_FastRead;
        sCommand.DataMode = FAL_DATA_1_LINE;
        sCommand.DummyCycles = 8;
    }

    if ((ret = FAL_SPI_SendCmd(&sCommand, driver->userdata)) != FAL_OK)
        return ret;
    return FAL_SPI_ReceiveData(buf, len, driver->userdata);
}

FAL_Status FAL_WritePage(FAL_SPI_Driver *driver, uint32_t Address, uint8_t *buf, uint32_t len)
{
    FAL_Status ret = FAL_OK;
    FAL_SPI_Cmd_t sCommand = {};

    sCommand.InstructionMode = FAL_INSTRUCTION_1_LINE;

    sCommand.AddressSize = driver->status.AddressSize;
    sCommand.Address = Address;
    sCommand.AddressMode = FAL_ADDRESS_1_LINE;
    sCommand.NbData = len;
    sCommand.DummyCycles = 0;

    if (driver->capacity.spi_type == SPI_TYPE_QUAD)
    {
        sCommand.Instruction = FAL_CMD_PageProgramQuad;
        sCommand.DataMode = FAL_DATA_4_LINES;
    }
    else
    {
        sCommand.Instruction = FAL_CMD_PageProgram;
        sCommand.DataMode = FAL_DATA_1_LINE;
    }

    if ((ret = FAL_WriteEnable(driver)) != FAL_OK)
        return ret;
    if ((ret = FAL_SPI_SendCmd(&sCommand, driver->userdata)) != FAL_OK)
        return ret;
    if ((ret = FAL_SPI_SendData(buf, len, driver->userdata)) != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG1, FAL_Status_REG1_BUSY, 0);
}

static FAL_Status FAL_EraseXX(FAL_SPI_Driver *driver, uint8_t erase_cmd, uint32_t Address)
{
    FAL_Status ret = FAL_OK;
    FAL_SPI_Cmd_t sCommand = {};

    sCommand.InstructionMode = FAL_INSTRUCTION_1_LINE;
    sCommand.Instruction = erase_cmd;

    sCommand.AddressMode = FAL_ADDRESS_1_LINE;
    sCommand.AddressSize = driver->status.AddressSize;
    sCommand.Address = Address;

    sCommand.DataMode = FAL_DATA_NONE;
    sCommand.NbData = 0;
    sCommand.DummyCycles = 0;

    if ((ret = FAL_WriteEnable(driver)) != FAL_OK)
        return ret;
    if ((ret = FAL_SPI_SendCmd(&sCommand, driver->userdata)) != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG1, FAL_Status_REG1_BUSY, 0);
}

FAL_Status FAL_EraseSector(FAL_SPI_Driver *driver, uint32_t Address)
{
    return FAL_EraseXX(driver, FAL_CMD_SectorErase, Address);
}

FAL_Status FAL_EraseBlock32K(FAL_SPI_Driver *driver, uint32_t Address)
{
    return FAL_EraseXX(driver, FAL_CMD_BlockErase_32K, Address);
}

FAL_Status FAL_EraseBlock64K(FAL_SPI_Driver *driver, uint32_t Address)
{
    return FAL_EraseXX(driver, FAL_CMD_BlockErase_64K, Address);
}

FAL_Status FAL_EraseChip(FAL_SPI_Driver *driver)
{
    FAL_Status ret = FAL_OK;
    if ((ret = FAL_WriteEnable(driver)) != FAL_OK)
        return ret;
    if ((ret = FAL_SignalCmd(driver, FAL_CMD_ChipErase)) != FAL_OK)
        return ret;
    return FAL_PollingReg(driver, FAL_CMD_ReadStatus_REG1, FAL_Status_REG1_BUSY, 0);
}

FAL_Status FAL_MemoryMappedMode(FAL_SPI_Driver *driver, bool enable)
{
    FAL_SPI_Cmd_t sCommand = {};
    sCommand.InstructionMode = FAL_INSTRUCTION_1_LINE;
    sCommand.AddressSize = driver->status.AddressSize;
    sCommand.Address = 0;
    sCommand.NbData = 0;

    if (driver->capacity.spi_type == SPI_TYPE_QUAD)
    {
        sCommand.AddressMode = FAL_ADDRESS_4_LINES;
        sCommand.Instruction = FAL_CMD_FastReadQuad_IO;
        sCommand.DataMode = FAL_DATA_4_LINES;
        sCommand.DummyCycles = 6;
    }
    else
    {
        sCommand.AddressMode = FAL_ADDRESS_1_LINE;
        sCommand.Instruction = FAL_CMD_FastRead;
        sCommand.DataMode = FAL_DATA_1_LINE;
        sCommand.DummyCycles = 8;
    }

    return FAL_SPI_MemoryMappedMode(&sCommand, enable, driver->userdata);
}

__weak_symbol FAL_Status FAL_SPI_SendCmd(FAL_SPI_Cmd_t *cmd, void *userdata)
{
    __builtin_trap();
    return FAL_ERR;
}

__weak_symbol FAL_Status FAL_SPI_SendData(uint8_t *data, uint32_t len, void *userdata)
{
    __builtin_trap();
    return FAL_ERR;
}

__weak_symbol FAL_Status FAL_SPI_ReceiveData(uint8_t *data, uint32_t len, void *userdata)
{
    __builtin_trap();
    return FAL_ERR;
}

__weak_symbol FAL_Status FAL_SPI_Polling(FAL_SPI_Cmd_t *cmd, FAL_SPI_PollingConfig_t *config, void *userdata)
{
    __builtin_trap();
    return FAL_ERR;
}

__weak_symbol FAL_Status FAL_SPI_MemoryMappedMode(FAL_SPI_Cmd_t *cmd, bool enable, void *userdata)
{
    __builtin_trap();
    return FAL_ERR;
}
