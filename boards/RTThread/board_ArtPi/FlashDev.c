#include "FlashOS.h"

struct FlashDevice const FlashDevice __attribute__((section("DevDscr"))) = {
    ALGO_VERSION,      // Algo version
    "QSPI Flash",      // Flash device name
    EXTSPI,            // Flash device type
    EXFLASH_BASE_ADDR, // Flash base address
    EXFLASH_SIZE,         // Total flash device size in Bytes
    EXFLASH_PAGE_SIZE, // Page Size (number of bytes that will be passed to ProgramPage(). May be multiple of min
                       // alignment in order to reduce overhead for calling ProgramPage multiple times
    0,    // Reserved, should be 0
    0xFF, // Flash erased value
    1000, // Program page timeout in ms
    1000, // Erase sector timeout in ms
    //
    // Flash sector layout definition
    //
    {
        {0x00001000, 0x00000000},
        SECTOR_END,
    },
};
