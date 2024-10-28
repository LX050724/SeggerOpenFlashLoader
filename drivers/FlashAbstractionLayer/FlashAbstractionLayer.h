#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_SIZE_MBYTE(SIZE) ((SIZE) << 20)
#define FLASH_SIZE_MBIT(SIZE) ((SIZE) << 17)

typedef enum
{
    FAL_OK = 0,
    FAL_ERR,
} FAL_Status;

typedef struct
{
    enum FAL_InstructionMode
    {
        FAL_INSTRUCTION_NONE = 0,
        FAL_INSTRUCTION_1_LINE = 1,
        FAL_INSTRUCTION_2_LINE = 2,
        FAL_INSTRUCTION_4_LINE = 4,
    } InstructionMode;

    enum FAL_AddressMode
    {
        FAL_ADDRESS_NONE = 0,
        FAL_ADDRESS_1_LINE = 1,
        FAL_ADDRESS_2_LINES = 2,
        FAL_ADDRESS_4_LINES = 4,
    } AddressMode;

    enum FAL_AddressSize
    {
        FAL_ADDRESS_8_BITS = 8,
        FAL_ADDRESS_16_BITS = 16,
        FAL_ADDRESS_24_BITS = 24,
        FAL_ADDRESS_32_BITS = 32,
    } AddressSize;

    enum FAL_DataMode
    {
        FAL_DATA_NONE = 0,
        FAL_DATA_1_LINE = 1,
        FAL_DATA_2_LINES = 2,
        FAL_DATA_4_LINES = 4,
    } DataMode;

    uint32_t Instruction;
    uint32_t Address;
    uint32_t NbData;
    uint32_t DummyCycles;
} FAL_SPI_Cmd_t;

typedef struct {
    uint32_t Match;
    uint32_t Mask;
    uint32_t StatusBytesSize;
} FAL_SPI_PollingConfig_t;


typedef struct
{
    struct
    {
        enum
        {
            SPI_TYPE_STD = FAL_DATA_1_LINE,
            // SPI_TYPE_DUAL = FAL_DATA_2_LINES, // TODO 暂不支持
            SPI_TYPE_QUAD = FAL_DATA_4_LINES,
        } spi_type;

        uint32_t flash_size;
        bool support_mmap;
    } capacity;

    struct {
        enum FAL_AddressSize AddressSize;
    } status;

    void *userdata;
} FAL_SPI_Driver;

typedef struct {
    void *spi;
    void *cs_gpio_handle;
    uint32_t cs_gpio_pin;
} FAL_SPI_Userdata_t;

FAL_Status FAL_Init(FAL_SPI_Driver *driver);
FAL_Status FAL_Read(FAL_SPI_Driver *driver, uint32_t Address, uint8_t *buf, uint32_t len);
FAL_Status FAL_WritePage(FAL_SPI_Driver *driver, uint32_t Address, uint8_t *buf, uint32_t len);
FAL_Status FAL_EraseSector(FAL_SPI_Driver *driver, uint32_t Address);
FAL_Status FAL_EraseBlock32K(FAL_SPI_Driver *driver, uint32_t Address);
FAL_Status FAL_EraseBlock64K(FAL_SPI_Driver *driver, uint32_t Address);
FAL_Status FAL_EraseChip(FAL_SPI_Driver *driver);
FAL_Status FAL_MemoryMappedMode(FAL_SPI_Driver *driver, bool enable);

FAL_Status FAL_SPI_SendCmd(FAL_SPI_Cmd_t *cmd, void *userdata);
FAL_Status FAL_SPI_SendData(uint8_t *data, uint32_t len, void *userdata);
FAL_Status FAL_SPI_ReceiveData(uint8_t *data, uint32_t len, void *userdata);
FAL_Status FAL_SPI_Polling(FAL_SPI_Cmd_t *cmd, FAL_SPI_PollingConfig_t *config, void *userdata);
FAL_Status FAL_SPI_MemoryMappedMode(FAL_SPI_Cmd_t *cmd, bool enable, void *userdata);

#ifdef __cplusplus
}
#endif
