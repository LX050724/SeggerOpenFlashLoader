
#include "w25q256.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_qspi.h"
#include <stdint.h>
//
#include "quadspi.h"

#define W25Qxx_CMD_EnableReset 0x66 // 使能复位
#define W25Qxx_CMD_ResetDevice 0x99 // 复位器件
#define W25Qxx_CMD_JedecID 0x9F     // JEDEC ID
#define W25Qxx_CMD_WriteEnable 0X06 // 写使能

#define W25Qxx_CMD_SectorErase 0x20      // 扇区擦除，4K字节， 参考擦除时间 45ms
#define W25Qxx_CMD32_SectorErase 0x21    // 扇区擦除，4K字节， 参考擦除时间 45ms
#define W25Qxx_CMD_BlockErase_32K 0x52   // 块擦除，  32K字节，参考擦除时间 120ms
#define W25Qxx_CMD_BlockErase_64K 0xD8   // 块擦除，  64K字节，参考擦除时间 150ms
#define W25Qxx_CMD32_BlockErase_64K 0xDC // 块擦除，  64K字节，参考擦除时间 150ms
#define W25Qxx_CMD_ChipErase 0xC7        // 整片擦除，参考擦除时间 20S

#define W25Qxx_CMD_QuadInputPageProgram 0x32 // 1-1-4模式下(1线指令1线地址4线数据)，页编程指令，参考写入时间 0.4ms
#define W25Qxx_CMD32_QuadInputPageProgram 0x34 // 1-1-4模式下(1线指令1线地址4线数据)，页编程指令，参考写入时间 0.4ms
#define W25Qxx_CMD_FastReadQuad_IO 0xEB   // 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令
#define W25Qxx_CMD32_FastReadQuad_IO 0xEC // 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

#define W25Qxx_CMD_Entry4BAddrMode 0xB7
#define W25Qxx_CMD_Exit4BAddrMode 0xE9
#define W25Qxx_CMD_WrtieExtAddr 0xC5

#define W25Qxx_CMD_ReadStatus_REG1 0X05 // 读状态寄存器1
#define W25Qxx_CMD_ReadStatus_REG2 0X35 // 读状态寄存器2
#define W25Qxx_CMD_ReadStatus_REG3 0X15 // 读状态寄存器
#define W25Qxx_Status_REG1_BUSY 0x01
#define W25Qxx_Status_REG1_WEL 0x02
#define W25Qxx_Status_REG3_ADS 0x01

// W25Q256JV-IQ/JQ
static int QSPI_W25Qxx_WriteEnable();
static int QSPI_W25Qxx_Cmd(uint8_t cmd);
static int QSPI_W25Qxx_SetExAddr(uint8_t addr);

static int QSPI_W25Qxx_Polling(uint8_t reg_cmd, uint8_t mask, uint8_t match)
{
    QSPI_CommandTypeDef sCommand = {};
    QSPI_AutoPollingTypeDef sConfig = {};

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 1;
    sCommand.Instruction = reg_cmd;

    sConfig.Match = match;
    sConfig.Mask = mask;
    sConfig.Interval = 0x10;
    sConfig.StatusBytesSize = 1;
    sConfig.MatchMode = QSPI_MATCH_MODE_AND;            //	与运算
    sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE; // 自动停止模式

    if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    return 0;
}

static int QSPI_W25Qxx_Cmd(uint8_t cmd)
{
    QSPI_CommandTypeDef sCommand = {};

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.Instruction = cmd;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    return 0;
}

static int QSPI_W25Qxx_Enter4ByteMode()
{
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_Entry4BAddrMode) != 0)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG3, W25Qxx_Status_REG3_ADS, W25Qxx_Status_REG3_ADS) != 0)
        return -1;
    return 0;
}

static int QSPI_W25Qxx_Exit4ByteMode()
{
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_Exit4BAddrMode) != 0)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG3, W25Qxx_Status_REG3_ADS, 0) != 0)
        return -1;
    return 0;
}

static int QSPI_W25Qxx_SetExAddr(uint8_t addr)
{
    if (QSPI_W25Qxx_WriteEnable() != HAL_OK)
        return -1;

    QSPI_CommandTypeDef sCommand = {};
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.NbData = 1;
    sCommand.DummyCycles = 0;
    sCommand.Instruction = W25Qxx_CMD_WrtieExtAddr;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    if (HAL_QSPI_Transmit(&hqspi, &addr, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    return 0;
}

int QSPI_W25Qxx_Reset()
{
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_EnableReset) != 0)
        return -1;
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_ResetDevice) != 0)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG1, W25Qxx_Status_REG1_BUSY, 0) != 0)
        return -1;

    return 0;
}

int QSPI_W25Qxx_ReadReg(uint8_t reg_cmd)
{
    QSPI_CommandTypeDef sCommand = {};
    uint8_t reg_val;

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_1_LINE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 1;
    sCommand.Instruction = reg_cmd;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    if (HAL_QSPI_Receive(&hqspi, &reg_val, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    return reg_val;
}

int QSPI_W25Qxx_Init()
{
    if (QSPI_W25Qxx_Reset() != 0)
        return -1;
    // 检查4Byte地址使能，如未使能则临时进入4Byte模式
    int reg3 = QSPI_W25Qxx_ReadReg(W25Qxx_CMD_ReadStatus_REG3);
    if (reg3 < 0)
        return -1;
    if ((reg3 & W25Qxx_Status_REG3_ADS) == 0)
        QSPI_W25Qxx_Enter4ByteMode();
    return 0;
}

uint32_t QSPI_W25Qxx_Read_JEDECID()
{
    QSPI_CommandTypeDef sCommand = {};
    uint8_t QSPI_ReceiveBuff[3] = {}; // 存储QSPI读到的数据
    uint32_t W25Qxx_ID = 0;           // 器件的ID

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;     // 1线指令模式
    sCommand.AddressSize = QSPI_ADDRESS_32_BITS;            // 24位地址
    sCommand.AddressMode = QSPI_ADDRESS_NONE;               // 无地址模式
    sCommand.Address = 0;                                   // 地址0
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; // 无交替字节
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;               // 禁止DDR模式
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;           // 每次传输数据都发送指令
    sCommand.DataMode = QSPI_DATA_1_LINE;                   // 4线数据模式
    sCommand.DummyCycles = 0;                               // 空周期个数
    sCommand.NbData = 3;                                    // 传输数据的长度
    sCommand.Instruction = W25Qxx_CMD_JedecID;              // 执行读器件ID命令

    // 发送指令
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return -1;
    }

    // 接收数据
    if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return -1;
    }

    // 将得到的数据组合成ID
    W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8) | QSPI_ReceiveBuff[2];
    return W25Qxx_ID;
}

int QSPI_W25Qxx_Entry4BAddr()
{
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_Entry4BAddrMode) != HAL_OK)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG3, W25Qxx_Status_REG3_ADS, W25Qxx_Status_REG3_ADS) != 0)
        return -1;
    return 0;
}

static int QSPI_W25Qxx_WriteEnable()
{
    if (QSPI_W25Qxx_Cmd(W25Qxx_CMD_WriteEnable) != HAL_OK)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG1, W25Qxx_Status_REG1_WEL, W25Qxx_Status_REG1_WEL) != 0)
        return -1;
    return 0;
}

int QSPI_W25Qxx_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    QSPI_CommandTypeDef sCommand = {};
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Address = addr;
    sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
    sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_4_LINES;
    sCommand.DummyCycles = 6;
    sCommand.NbData = len;
    sCommand.Instruction = W25Qxx_CMD32_FastReadQuad_IO;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    if (HAL_QSPI_Transmit(&hqspi, buf, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    return 0;
}

int QSPI_W25Qxx_WritePage(uint32_t addr, uint8_t *buf, uint32_t len)
{
    QSPI_CommandTypeDef sCommand = {};

    if (QSPI_W25Qxx_WriteEnable() != HAL_OK)
        return -1;

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Address = addr;
    sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_4_LINES;
    sCommand.DummyCycles = 0;
    sCommand.NbData = len;
    sCommand.Instruction = W25Qxx_CMD_QuadInputPageProgram;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;

    if (HAL_QSPI_Transmit(&hqspi, buf, HAL_MAX_DELAY) != HAL_OK)
        return -1;

    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG1, W25Qxx_Status_REG1_BUSY, 0) != 0)
        return -1;
    return len;
}

static int QSPI_W25Qxx_Erase(uint8_t erase_cmd, int addr_len, uint32_t SectorAddr)
{
    QSPI_CommandTypeDef sCommand = {};
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Address = SectorAddr;
    sCommand.AddressSize = addr_len;
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.NbData = 0;
    sCommand.Instruction = erase_cmd;

    if (QSPI_W25Qxx_WriteEnable() != HAL_OK)
        return -1;
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG1, W25Qxx_Status_REG1_BUSY, 0) != 0)
        return -1;
    return 0;
}

int QSPI_W25Qxx_EraseSector(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_Erase(W25Qxx_CMD_SectorErase, QSPI_ADDRESS_32_BITS, SectorAddr);
}

int QSPI_W25Qxx_EraseBlock32K(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_Erase(W25Qxx_CMD_BlockErase_32K, QSPI_ADDRESS_32_BITS, SectorAddr);
}

int QSPI_W25Qxx_EraseBlock64K(uint32_t SectorAddr)
{
    return QSPI_W25Qxx_Erase(W25Qxx_CMD_BlockErase_64K, QSPI_ADDRESS_32_BITS, SectorAddr);
}

int QSPI_W25Qxx_EraseChip()
{
    QSPI_CommandTypeDef sCommand = {};
    if (QSPI_W25Qxx_WriteEnable() != HAL_OK)
        return -1;
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.DummyCycles = 0;
    sCommand.Instruction = W25Qxx_CMD_ChipErase;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return -1;
    if (QSPI_W25Qxx_Polling(W25Qxx_CMD_ReadStatus_REG1, W25Qxx_Status_REG1_BUSY, 0) != 0)
        return -1;
    return 0;
}

int QSPI_W25Qxx_MemoryMappedMode()
{
    QSPI_CommandTypeDef sCommand = {};
    QSPI_MemoryMappedTypeDef sMemMappedCfg = {};

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.Address = 0;
    sCommand.AddressSize = QSPI_ADDRESS_32_BITS;
    sCommand.AddressMode = QSPI_ADDRESS_4_LINES;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.DataMode = QSPI_DATA_4_LINES;
    sCommand.DummyCycles = 6;
    sCommand.NbData = 0;
    sCommand.Instruction = W25Qxx_CMD32_FastReadQuad_IO;

    if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK)
        return -1;
    return 0;
}

int QSPI_W25Qxx_ExitMemoryMappedMode()
{
    return HAL_QSPI_Abort(&hqspi);
}